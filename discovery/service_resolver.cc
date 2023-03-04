#include "discovery/service_resolver.h"

#include <filesystem>
#include <string>

#include "discovery/network_address_utils.h"
#include "discovery/service_discovery.h"
#include "discovery/service_types.h"
#include "glog/logging.h"
#include "grpcpp/grpcpp.h"
#include "src/core/lib/config/core_configuration.h"
#include "src/core/lib/iomgr/exec_ctx.h"
#include "src/core/lib/resolver/resolver.h"
#include "src/core/lib/resolver/resolver_factory.h"
#include "src/core/lib/resolver/resolver_registry.h"
#include "src/core/lib/resolver/server_address.h"
#include "src/core/lib/uri/uri_parser.h"

namespace tvsc::discovery {

class ServiceResolverFactory final : public grpc_core::ResolverFactory {
 private:
  std::unique_ptr<ServiceDiscovery> discovery_;

 public:
  ServiceResolverFactory(std::unique_ptr<ServiceDiscovery> discovery)
      : discovery_(std::move(discovery)) {}

  absl::string_view scheme() const override;

  /// Returns a bool indicating whether the input uri is valid to create a
  /// resolver.
  bool IsValidUri(const grpc_core::URI& uri) const override;

  /// Returns a new resolver instance.
  grpc_core::OrphanablePtr<grpc_core::Resolver> CreateResolver(
      grpc_core::ResolverArgs args) const override;
};

absl::string_view ServiceResolverFactory::scheme() const { return MDNS_SCHEME; }

bool ServiceResolverFactory::IsValidUri(const grpc_core::URI& uri) const { return true; }

std::string to_string(const grpc_resolved_address& addr) {
  using std::to_string;
  std::string result{};
  result.append("address family: ");
  const struct sockaddr* sock_addr = reinterpret_cast<const struct sockaddr*>(addr.addr);
  result.append(to_string(sock_addr->sa_family));
  result.append(", ");
  result.append("address of length: ");
  result.append(to_string(addr.len));
  return result;
}

class MDnsResolver final : public grpc_core::Resolver {
 private:
  ServiceDiscovery* discovery_;
  grpc_core::ResolverArgs args_;

  void resolve() {
    LOG(INFO) << "MDnsResolver::resolve()";

    // The service is referred to via a URI. The path part of the URI contains the service type as
    // the filename part of the URI path. We use the std::filesystem::path type to parse out that
    // filename part of the URI path.
    const std::filesystem::path path{args_.uri.path()};
    const std::string service_type{path.filename()};

    discovery_->add_service_type_and_block_until_ready(service_type);

    LOG(INFO) << "Assembling servers result for service_type '" << service_type << "'";

    grpc_core::ServerAddressList discovered_servers{};
    for (const auto& server : this->discovery_->resolve(service_type)) {
      grpc_resolved_address address{};
      LOG(INFO) << "server:\n" << server.DebugString();
      resolved_server_to_grpc(server, &address);
      LOG(INFO) << "grpc_resolved_address: " << to_string(address);
      discovered_servers.emplace_back(address, args_.args);
    }

    // Hack.
    // TODO(james): Figure out where this result_handler is getting cleaned up / set to nullptr and
    // adjust the logic appropriately.
    if (this->args_.result_handler) {
      grpc_core::Resolver::Result result;
      result.addresses = discovered_servers;
      this->args_.result_handler->ReportResult(result);
    }
  }

 public:
  MDnsResolver(ServiceDiscovery& discovery, grpc_core::ResolverArgs&& args)
      : discovery_(&discovery), args_(std::move(args)) {}

  void StartLocked() override { resolve(); }
  void RequestReresolutionLocked() override { resolve(); }
  void ResetBackoffLocked() override { resolve(); }

 protected:
  void ShutdownLocked() override {}
};

grpc_core::OrphanablePtr<grpc_core::Resolver> ServiceResolverFactory::CreateResolver(
    grpc_core::ResolverArgs args) const {
  return grpc_core::OrphanablePtr<grpc_core::Resolver>{
      new MDnsResolver{*discovery_, std::move(args)}};
}

void register_mdns_grpc_resolver() {
  grpc_core::CoreConfiguration::RegisterBuilder(
      [](grpc_core::CoreConfiguration::Builder* configuration_builder) {
        std::unique_ptr<ServiceDiscovery> discovery = std::make_unique<ServiceDiscovery>();
        std::unique_ptr<ServiceResolverFactory> resolver{
            std::make_unique<ServiceResolverFactory>(std::move(discovery))};

        grpc_core::ResolverRegistry::Builder* resolver_registry{
            configuration_builder->resolver_registry()};
        resolver_registry->RegisterResolverFactory(std::move(resolver));

        LOG(INFO) << "mdns resolver factory registered.";
      });
}

}  // namespace tvsc::discovery
