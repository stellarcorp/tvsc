#include "discovery/service_browser.h"

#include <exception>
#include <future>
#include <string>
#include <unordered_set>
#include <vector>

#include "avahi-client/client.h"
#include "avahi-client/lookup.h"
#include "avahi-common/error.h"
#include "avahi-common/malloc.h"
#include "avahi-common/simple-watch.h"
#include "discovery/network_address_utils.h"
#include "discovery/service_descriptor.pb.h"
#include "glog/logging.h"

namespace tvsc::discovery {

void ServiceBrowser::on_client_change(AvahiClient *client, AvahiClientState state,
                                      void *service_browser) {
  ServiceBrowser *browser{static_cast<ServiceBrowser *>(service_browser)};
  switch (state) {
    case AVAHI_CLIENT_S_RUNNING: {
      LOG(INFO) << "on_client_change() -- AVAHI_CLIENT_S_RUNNING.";
      // The server has startup successfully and registered its host
      // name on the network, so now we can register services.
      if (!browser->have_valid_client_.load()) {
        browser->have_valid_client_.store(true);
      }
      break;
    }
    case AVAHI_CLIENT_FAILURE: {
      LOG(INFO) << "on_client_change() -- AVAHI_CLIENT_FAILURE.";
      if (browser->have_valid_client_.load()) {
        browser->have_valid_client_.store(false);
      }
      break;
    }
    case AVAHI_CLIENT_S_COLLISION: {
      LOG(INFO) << "on_client_change() -- AVAHI_CLIENT_S_COLLISION: unhandled. TODO: Handle this "
                   "case by re-registering all previous services.";
      /* Let's drop our registered services. When the server is back
       * in AVAHI_SERVER_RUNNING state we will register them
       * again with the new host name. */
      if (browser->have_valid_client_.load()) {
        browser->have_valid_client_.store(false);
      }
      break;
    }
    case AVAHI_CLIENT_S_REGISTERING: {
      LOG(INFO) << "on_client_change() -- AVAHI_CLIENT_S_REGISTERING: unhandled. TODO: Handle this "
                   "case by re-registering all previous services.";
      /* The server records are now being established. This
       * might be caused by a host name change. We need to wait
       * for our own records to register until the host name is
       * properly esatblished. */
      if (browser->have_valid_client_.load()) {
        browser->have_valid_client_.store(false);
      }
      break;
    }
    case AVAHI_CLIENT_CONNECTING:
      LOG(INFO) << "on_client_change() -- AVAHI_CLIENT_CONNECTING: Ignored.";
      break;
  }
}

void ServiceBrowser::on_browser_change(AvahiServiceBrowser *avahi_browser, AvahiIfIndex interface,
                                       AvahiProtocol protocol, AvahiBrowserEvent event,
                                       const char *name, const char *type, const char *domain,
                                       AVAHI_GCC_UNUSED AvahiLookupResultFlags flags,
                                       void *service_browser) {
  ServiceBrowser *browser{static_cast<ServiceBrowser *>(service_browser)};
  /* Called whenever a new services becomes available on the LAN or is removed from the LAN */
  switch (event) {
    case AVAHI_BROWSER_NEW: {
      LOG(INFO) << "on_browser_change() -- AVAHI_BROWSER_NEW: service '" << name << "' of type '"
                << type << "' in domain '" << domain << "'";
      browser->add_unresolved_service(name, type, domain);
      // Note: the resolver gets freed in on_resolver_change(). It seems like the resolvers are tied
      // to a single resolution request, but still must be allocated on the heap. This idiom of
      // freeing the resolver in the callback comes from Avahi example code
      // (client-browse-services.c).
      auto resolver = avahi_service_resolver_new(
          avahi_service_browser_get_client(avahi_browser), interface, protocol, name, type, domain,
          AVAHI_PROTO_UNSPEC, static_cast<AvahiLookupFlags>(0), on_resolver_change, browser);
      if (!resolver) {
        LOG(ERROR) << "on_browser_change() -- Failed to create resolver: "
                   << avahi_strerror(
                          avahi_client_errno(avahi_service_browser_get_client(avahi_browser)));
      }

      break;
    }
    case AVAHI_BROWSER_REMOVE: {
      LOG(INFO) << "on_browser_change() -- AVAHI_BROWSER_REMOVE: service '" << name << "' of type '"
                << type << "' in domain '" << domain << "'";
      browser->remove_unresolved_service(name, type, domain);
      break;
    }
    case AVAHI_BROWSER_ALL_FOR_NOW:
      LOG(INFO) << "on_browser_change() -- AVAHI_BROWSER_ALL_FOR_NOW.";
      break;
    case AVAHI_BROWSER_CACHE_EXHAUSTED:
      LOG(INFO) << "on_browser_change() -- AVAHI_BROWSER_CACHE_EXHAUSTED.";
      break;
    case AVAHI_BROWSER_FAILURE: {
      LOG(INFO) << "on_browser_change() -- AVAHI_BROWSER_FAILURE.";
      break;
    }
  }
}

void ServiceBrowser::on_resolver_change(AvahiServiceResolver *resolver,
                                        AVAHI_GCC_UNUSED AvahiIfIndex interface,
                                        AvahiProtocol protocol, AvahiResolverEvent event,
                                        const char *name, const char *type, const char *domain,
                                        const char *hostname, const AvahiAddress *address,
                                        uint16_t port, AvahiStringList *txt,
                                        AvahiLookupResultFlags flags, void *service_browser) {
  ServiceBrowser *browser{static_cast<ServiceBrowser *>(service_browser)};
  switch (event) {
    case AVAHI_RESOLVER_FAILURE:
      LOG(INFO) << "on_resolver_change() -- AVAHI_RESOLVER_FAILURE: "
                << avahi_strerror(avahi_client_errno(avahi_service_resolver_get_client(resolver)))
                << ", name: " << name << ", type: " << type << ", domain: " << domain;
      break;
    case AVAHI_RESOLVER_FOUND: {
      LOG(INFO) << "on_resolver_change() -- AVAHI_RESOLVER_FOUND";
      browser->add_resolved_service(name, type, domain, hostname,
                                    avahi_address_to_network_address(protocol, *address), port);
      break;
    }
  }
  avahi_service_resolver_free(resolver);
}

ServiceBrowser::ServiceBrowser(const std::unordered_set<std::string> &service_types) {
  watcher_.reset(avahi_simple_poll_new());
  if (!watcher_) {
    throw std::runtime_error("Could not start AvahiSimplePoll watcher");
  }

  int avahi_error{};
  avahi_client_.reset(avahi_client_new(avahi_simple_poll_get(watcher_.get()), AVAHI_CLIENT_NO_FAIL,
                                       on_client_change, this, &avahi_error));
  if (!avahi_client_) {
    throw std::runtime_error(std::string{"Could not create AvahiClient: "} +
                             avahi_strerror(avahi_error));
  }

  for (const auto &service_type : service_types) {
    add_service_type(service_type);
  }

  watcher_task_ = std::async(std::launch::async, &avahi_simple_poll_loop, watcher_.get());
  if (!watcher_task_.valid()) {
    throw std::runtime_error("Could not start the avahi_simple_poll_loop()");
  }
}

ServiceBrowser::~ServiceBrowser() {
  if (watcher_) avahi_simple_poll_quit(watcher_.get());
  if (watcher_task_.valid()) {
    watcher_task_.wait();
  }
}

void ServiceBrowser::add_resolved_service(const std::string &name, const std::string &type,
                                          const std::string &domain, const std::string &hostname,
                                          const NetworkAddress &address, int port) {
  LOG(INFO) << "add_resolved_service() -- name: " << name << ", type: " << type
            << ", domain: " << domain << ", hostname: " << hostname
            << ", address: " << network_address_to_string(address) << ", port: " << port;
  if (discovered_services_.count(name) == 0) {
    ServiceSet service_set{};
    service_set.set_canonical_name(name);
    service_set.set_published_name(name);
    discovered_services_.emplace(name, service_set);
  }
  ServiceSet &service_set = discovered_services_.at(name);
  for (auto service : *service_set.mutable_services()) {
    if (service.service_type() == type and service.domain() == domain) {
      for (const auto &server : service.servers()) {
        if (server.hostname() == hostname and is_same_address(server.address(), address) and
            server.port() == port) {
          // Already known. Do not add again.
          return;
        }
      }
      ServerDetails *server{service.add_servers()};
      server->set_hostname(hostname);
      *server->mutable_address() = address;
      server->set_port(port);
      return;
    }
  }
  ServiceDescriptor *descriptor = service_set.add_services();
  descriptor->set_service_type(type);
  descriptor->set_domain(domain);
}

void ServiceBrowser::add_unresolved_service(const std::string &name, const std::string &type,
                                            const std::string &domain) {
  if (discovered_services_.count(name) == 0) {
    ServiceSet service_set{};
    service_set.set_canonical_name(name);
    service_set.set_published_name(name);
    discovered_services_.emplace(name, service_set);
  }
  ServiceSet &service_set = discovered_services_.at(name);
  for (const auto &service : service_set.services()) {
    if (service.service_type() == type and service.domain() == domain) {
      // Already known. Do not add again.
      return;
    }
  }
  ServiceDescriptor *descriptor = service_set.add_services();
  descriptor->set_service_type(type);
  descriptor->set_domain(domain);
}

void ServiceBrowser::remove_unresolved_service(const std::string &name, const std::string &type,
                                               const std::string &domain) {
  if (discovered_services_.count(name) == 0) {
    // Nothing to remove.
    return;
  }
  ServiceSet &service_set = discovered_services_.at(name);
  auto *services = service_set.mutable_services();
  for (int i = 0; i < services->size(); ++i) {
    if (services->at(i).service_type() == type and services->at(i).domain() == domain) {
      // Found a match. Remove it.
      services->DeleteSubrange(i, 1);
      // If the service set has no more services, remove the service name entry from the discovered
      // services map.
      if (services->empty()) {
        discovered_services_.erase(name);
      }
      return;
    }
  }
}

std::unordered_set<std::string> ServiceBrowser::service_types() const {
  std::unordered_set<std::string> types{};
  for (const auto &entry : avahi_browsers_) {
    types.emplace(entry.first);
  }
  return types;
}

void ServiceBrowser::add_service_type(const std::string &service_type) {
  auto avahi_browser{avahi_service_browser_new(
      avahi_client_.get(), AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, service_type.c_str(), nullptr,
      static_cast<AvahiLookupFlags>(0), on_browser_change, this)};

  if (!avahi_browser) {
    throw std::runtime_error("Could not create AvahiServiceBrowser for service type '" +
                             service_type +
                             "': " + avahi_strerror(avahi_client_errno(avahi_client_.get())));
  }
  avahi_browsers_.emplace(service_type,
                          std::unique_ptr<AvahiServiceBrowser, int (*)(AvahiServiceBrowser *)>{
                              avahi_browser, avahi_service_browser_free});
}

void ServiceBrowser::remove_service_type(const std::string &service_type) {
  avahi_browsers_.erase(service_type);
}

std::unordered_set<std::string> ServiceBrowser::service_names() const {
  std::unordered_set<std::string> names{};
  for (const auto &entry : discovered_services_) {
    LOG(INFO) << "Service: " << entry.first << " -> \n{\n" << entry.second.DebugString() << "\n}";
    names.emplace(entry.first);
  }
  return names;
}

}  // namespace tvsc::discovery
