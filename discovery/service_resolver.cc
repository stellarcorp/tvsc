#include "discovery/service_resolver.h"

#include <filesystem>
#include <string>

#include "discovery/network_address_utils.h"
#include "discovery/service_browser.h"
#include "glog/logging.h"
#include "grpcpp/grpcpp.h"
#include "src/core/lib/iomgr/exec_ctx.h"
#include "src/core/lib/resolver/resolver.h"
#include "src/core/lib/resolver/resolver_factory.h"
#include "src/core/lib/resolver/resolver_registry.h"
#include "src/core/lib/resolver/server_address.h"
#include "src/core/lib/uri/uri_parser.h"

namespace tvsc::discovery {

absl::string_view ServiceResolverFactory::scheme() const { return "mdns"; }

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
  ServiceBrowser* browser_;
  grpc_core::ResolverArgs args_;
  grpc_channel_args* channel_args_;

  void resolve() {
    LOG(INFO) << "MDnsResolver::resolve()";

    // The service is referred to via a URI. The path part of the URI contains the service type as
    // the filename part of the URI path. We use the std::filesystem::path type to parse out that
    // filename part of the URI path.
    const std::filesystem::path path{args_.uri.path()};
    const std::string service_type{path.filename()};

    browser_->watch_service_type(service_type, [this](const std::string& service_type) {
      LOG(INFO) << "Assembling servers result for service_type '" << service_type << "'";

      grpc_core::ServerAddressList discovered_servers{};
      for (const auto& server : this->browser_->resolve_service_type(service_type)) {
        // TODO(james): Hack.
        if (server.address().interface_name() == "lo") {
          grpc_resolved_address address{};
          LOG(INFO) << "server:\n" << server.DebugString();
          resolved_server_to_grpc(server, &address);
          LOG(INFO) << "grpc_resolved_address: " << to_string(address);
          discovered_servers.emplace_back(address, nullptr);
        }
      }
      // We must report the result within an executor context. Not sure what this is doing, but we
      // get a segfault if we don't.
      grpc_core::ExecCtx exec_ctx{};
      grpc_core::Resolver::Result result{};
      result.args = this->channel_args_;
      this->channel_args_ = nullptr;
      result.addresses = std::move(discovered_servers);
      this->args_.result_handler->ReportResult(std::move(result));
    });
  }

 public:
  MDnsResolver(ServiceBrowser& browser, grpc_core::ResolverArgs&& args)
      : browser_(&browser),
        args_(std::move(args)),
        channel_args_(grpc_channel_args_copy(args_.args)) {
    args_.args = nullptr;
  }
  ~MDnsResolver() { grpc_channel_args_destroy(channel_args_); }

  void StartLocked() override { resolve(); }
  void RequestReresolutionLocked() override { resolve(); }
  void ResetBackoffLocked() override { resolve(); }

 protected:
  void ShutdownLocked() override {}
};

grpc_core::OrphanablePtr<grpc_core::Resolver> ServiceResolverFactory::CreateResolver(
    grpc_core::ResolverArgs args) const {
  return grpc_core::OrphanablePtr<grpc_core::Resolver>{
      new MDnsResolver{*browser_, std::move(args)}};
}

}  // namespace tvsc::discovery
