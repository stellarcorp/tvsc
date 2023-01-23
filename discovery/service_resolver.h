#pragma once

#include "discovery/service_browser.h"
#include "grpcpp/grpcpp.h"
#include "src/core/lib/resolver/resolver.h"
#include "src/core/lib/resolver/resolver_factory.h"
#include "src/core/lib/resolver/resolver_registry.h"

namespace tvsc::discovery {

class ServiceResolverFactory final : public grpc_core::ResolverFactory {
 private:
  ServiceBrowser* browser_;

 public:
  ServiceResolverFactory(ServiceBrowser& browser) : browser_(&browser) {}

  absl::string_view scheme() const override;

  /// Returns a bool indicating whether the input uri is valid to create a
  /// resolver.
  bool IsValidUri(const grpc_core::URI& uri) const override;

  /// Returns a new resolver instance.
  grpc_core::OrphanablePtr<grpc_core::Resolver> CreateResolver(
      grpc_core::ResolverArgs args) const override;
};

}  // namespace tvsc::discovery
