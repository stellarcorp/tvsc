#include "discovery/service_discovery.h"

#include <chrono>
#include <exception>
#include <future>
#include <mutex>
#include <string>
#include <thread>
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

void ServiceDiscovery::on_client_change(AvahiClient *client, AvahiClientState state,
                                        void *service_browser) {
  // TODO(james): Implement these states to give better error handling.
  // ServiceDiscovery *browser{static_cast<ServiceDiscovery *>(service_browser)};
  switch (state) {
    case AVAHI_CLIENT_S_RUNNING: {
      LOG(INFO) << "on_client_change() -- AVAHI_CLIENT_S_RUNNING.";
      break;
    }
    case AVAHI_CLIENT_FAILURE: {
      LOG(INFO) << "on_client_change() -- AVAHI_CLIENT_FAILURE.";
      break;
    }
    case AVAHI_CLIENT_S_COLLISION: {
      LOG(INFO) << "on_client_change() -- AVAHI_CLIENT_S_COLLISION.";
      break;
    }
    case AVAHI_CLIENT_S_REGISTERING: {
      LOG(INFO) << "on_client_change() -- AVAHI_CLIENT_S_REGISTERING.";
      break;
    }
    case AVAHI_CLIENT_CONNECTING:
      LOG(INFO) << "on_client_change() -- AVAHI_CLIENT_CONNECTING: Ignored.";
      break;
  }
}

void ServiceDiscovery::on_browser_change(AvahiServiceBrowser *avahi_browser, AvahiIfIndex interface,
                                         AvahiProtocol protocol, AvahiBrowserEvent event,
                                         const char *name, const char *service_type,
                                         const char *domain,
                                         AVAHI_GCC_UNUSED AvahiLookupResultFlags flags,
                                         void *service_browser) {
  ServiceDiscovery *browser{static_cast<ServiceDiscovery *>(service_browser)};
  /* Called whenever a new services becomes available on the LAN or is removed from the LAN */
  switch (event) {
    case AVAHI_BROWSER_NEW: {
      LOG(INFO) << "on_browser_change() -- AVAHI_BROWSER_NEW: service '" << name << "' of type '"
                << service_type << "' in domain '" << domain << "'";
      browser->clear_service_records(name, service_type, domain);
      ++browser->service_type_resolvers_in_flight_[service_type];
      // Note: the resolver gets freed in on_resolver_change().
      auto resolver =
          avahi_service_resolver_new(avahi_service_browser_get_client(avahi_browser), interface,
                                     protocol, name, service_type, domain, AVAHI_PROTO_UNSPEC,
                                     static_cast<AvahiLookupFlags>(0), on_resolver_change, browser);
      if (!resolver) {
        LOG(ERROR) << "on_browser_change() -- Failed to create resolver: "
                   << avahi_strerror(
                          avahi_client_errno(avahi_service_browser_get_client(avahi_browser)));
      }

      break;
    }
    case AVAHI_BROWSER_REMOVE: {
      LOG(INFO) << "on_browser_change() -- AVAHI_BROWSER_REMOVE: service '" << name << "' of type '"
                << service_type << "' in domain '" << domain << "'";
      browser->clear_service_records(name, service_type, domain);
      ++browser->service_type_resolvers_in_flight_[service_type];
      // Note: the resolver gets freed in on_resolver_change().
      auto resolver =
          avahi_service_resolver_new(avahi_service_browser_get_client(avahi_browser), interface,
                                     protocol, name, service_type, domain, AVAHI_PROTO_UNSPEC,
                                     static_cast<AvahiLookupFlags>(0), on_resolver_change, browser);
      if (!resolver) {
        LOG(ERROR) << "on_browser_change() -- Failed to create resolver: "
                   << avahi_strerror(
                          avahi_client_errno(avahi_service_browser_get_client(avahi_browser)));
      }

      break;
    }
    case AVAHI_BROWSER_ALL_FOR_NOW: {
      LOG(INFO) << "on_browser_change() -- AVAHI_BROWSER_ALL_FOR_NOW. service_type: "
                << service_type;
      browser->update_watchers(service_type);
      break;
    }
    case AVAHI_BROWSER_CACHE_EXHAUSTED: {
      LOG(INFO) << "on_browser_change() -- AVAHI_BROWSER_CACHE_EXHAUSTED. service_type: "
                << service_type;
      // Ensure that an entry for the resolvers in flight exists for this service type. We use its
      // existence to determine that all necessary resolvers were created. If we never find any
      // entries for a particular service type, we can fail to notify the relevant watcher that
      // there are no entries. On the other hand, we can also have a race condition where a single
      // resolver finishes just before another resolver gets created.
      // TODO(james): Rethink the design of this part. It's ambiguous and may not adequately
      // represent how these events work.
      browser->service_type_resolvers_in_flight_[service_type];
      break;
    }
    case AVAHI_BROWSER_FAILURE: {
      LOG(INFO) << "on_browser_change() -- AVAHI_BROWSER_FAILURE. service_type: " << service_type;
      // TODO(james): Implement retry mechanism.
      break;
    }
  }
}

void ServiceDiscovery::on_resolver_change(AvahiServiceResolver *resolver, AvahiIfIndex interface,
                                          AvahiProtocol protocol, AvahiResolverEvent event,
                                          const char *name, const char *service_type,
                                          const char *domain, const char *hostname,
                                          const AvahiAddress *address, uint16_t port,
                                          AvahiStringList *txt, AvahiLookupResultFlags flags,
                                          void *service_browser) {
  ServiceDiscovery *browser{static_cast<ServiceDiscovery *>(service_browser)};
  switch (event) {
    case AVAHI_RESOLVER_FAILURE:
      LOG(WARNING) << "on_resolver_change() -- AVAHI_RESOLVER_FAILURE: "
                   << avahi_strerror(
                          avahi_client_errno(avahi_service_resolver_get_client(resolver)))
                   << ", name: " << name << ", service_type: " << service_type
                   << ", domain: " << domain;
      break;
    case AVAHI_RESOLVER_FOUND: {
      LOG(INFO) << "on_resolver_change() -- AVAHI_RESOLVER_FOUND";
      browser->add_server(name, service_type, domain, hostname,
                          avahi_address_to_network_address(protocol, *address, interface), port);
      break;
    }
  }
  --browser->service_type_resolvers_in_flight_[service_type];
  browser->update_watchers(service_type);
  avahi_service_resolver_free(resolver);
}

void ServiceDiscovery::update_watchers(const std::string &service_type) const {
  // TODO(james): Implement observer mechanism to subscribe to service changes.
  DLOG(INFO) << "update_watchers() -- service_type: " << service_type;
  if (service_type_resolvers_in_flight_.count(service_type) > 0 and
      service_type_resolvers_in_flight_.at(service_type) == 0) {
    for (auto iter = service_type_watchers_.find(service_type);
         iter != service_type_watchers_.end(); ++iter) {
      if (iter->first == service_type) {
        auto &watcher = iter->second;
        watcher(service_type);
      } else {
        break;
      }
    }
  }
}

ServiceDiscovery::ServiceDiscovery() {
  loop_.reset(avahi_simple_poll_new());
  if (!loop_) {
    throw std::runtime_error("Could not start AvahiSimplePoll watcher");
  }

  int avahi_error{};
  avahi_client_.reset(avahi_client_new(avahi_simple_poll_get(loop_.get()), AVAHI_CLIENT_NO_FAIL,
                                       on_client_change, this, &avahi_error));
  if (!avahi_client_) {
    throw std::runtime_error(std::string{"Could not create AvahiClient: "} +
                             avahi_strerror(avahi_error));
  }

  loop_task_ = std::async(std::launch::async, &ServiceDiscovery::polling_loop, this);
  if (!loop_task_.valid()) {
    throw std::runtime_error("Could not start the avahi_simple_poll_loop()");
  }
}

ServiceDiscovery::~ServiceDiscovery() {
  if (loop_) avahi_simple_poll_quit(loop_.get());
  if (loop_task_.valid()) {
    loop_task_.wait();
  }
}

int ServiceDiscovery::polling_loop() {
  using namespace std::chrono_literals;
  while (true) {
    DLOG(INFO) << "Poll loop -- sleeping.";
    std::this_thread::sleep_for(10ms);
    std::lock_guard lock{this->avahi_call_mutex_};
    DLOG(INFO) << "Poll loop -- calling avahi_simple_poll_iterate()";
    int result = avahi_simple_poll_iterate(this->loop_.get(), -1);
    if (result != 0) {
      LOG(INFO) << "Poll loop -- exiting with result: " << result;
      return result;
    }
  }
}

void ServiceDiscovery::add_server(const std::string &name, const std::string &type,
                                  const std::string &domain, const std::string &hostname,
                                  const NetworkAddress &address, int port) {
  add_service(name, type, domain);
  ServiceDescriptor &service = lookup_service(name, type, domain);
  add_server(service, hostname, address, port);
}

bool ServiceDiscovery::has_server(const ServiceDescriptor &service, const std::string &hostname,
                                  const NetworkAddress &address, int port) const {
  for (const auto &server : service.servers()) {
    if (server.hostname() == hostname and is_same_address(server.address(), address) and
        server.port() == port) {
      return true;
    }
  }

  return false;
}

void ServiceDiscovery::add_server(ServiceDescriptor &service, const std::string &hostname,
                                  const NetworkAddress &address, int port) {
  if (!has_server(service, hostname, address, port)) {
    ServerDetails *server{service.add_servers()};
    server->set_hostname(hostname);
    *server->mutable_address() = address;
    server->set_port(port);
  }
}

void ServiceDiscovery::add_service(const std::string &name, const std::string &type,
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

void ServiceDiscovery::clear_service_records(const std::string &name, const std::string &type,
                                             const std::string &domain) {
  if (discovered_services_.count(name) == 0) {
    // Nothing to clear.
    return;
  }
  ServiceSet &service_set = discovered_services_.at(name);
  auto services = service_set.mutable_services();
  for (int i = 0; i < services->size(); ++i) {
    ServiceDescriptor &service{services->at(i)};
    if (service.service_type() == type and service.domain() == domain) {
      services->DeleteSubrange(i, 1);
      return;
    }
  }
}

ServiceDescriptor &ServiceDiscovery::lookup_service(const std::string &name,
                                                    const std::string &type,
                                                    const std::string &domain) {
  ServiceSet &service_set = discovered_services_.at(name);
  auto services = service_set.mutable_services();
  for (int i = 0; i < services->size(); ++i) {
    ServiceDescriptor &service{services->at(i)};
    if (service.service_type() == type and service.domain() == domain) {
      return service;
    }
  }
  throw std::domain_error("Service with name = '" + name + "', type = '" + type + "', domain = '" +
                          domain + "' not found.");
}

const ServiceDescriptor &ServiceDiscovery::lookup_service(const std::string &name,
                                                          const std::string &type,
                                                          const std::string &domain) const {
  const ServiceSet &service_set = discovered_services_.at(name);
  const auto &services = service_set.services();
  for (int i = 0; i < services.size(); ++i) {
    const ServiceDescriptor &service{services.at(i)};
    if (service.service_type() == type and service.domain() == domain) {
      return service;
    }
  }
  throw std::domain_error("Service with name = '" + name + "', type = '" + type + "', domain = '" +
                          domain + "' not found.");
}

std::unordered_set<std::string> ServiceDiscovery::service_types() const {
  std::unordered_set<std::string> types{};
  for (const auto &entry : avahi_browsers_) {
    types.emplace(entry.first);
  }
  return types;
}

void ServiceDiscovery::watch_service_type(const std::string &service_type,
                                          ServiceTypeWatcher watcher) {
  if (avahi_browsers_.count(service_type) == 0) {
    LOG(INFO) << "Watching for services announcing for service type '" << service_type << "'";
    std::lock_guard lock{avahi_call_mutex_};
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

  service_type_watchers_.emplace(service_type, std::move(watcher));
}

std::vector<ServerDetails> ServiceDiscovery::resolve_service_type(
    const std::string &service_type) const {
  std::vector<ServerDetails> result{};
  for (const auto &[name, service_set] : discovered_services_) {
    for (const auto &service : service_set.services()) {
      if (service_type == service.service_type()) {
        for (const auto &server : service.servers()) {
          result.emplace_back(server);
        }
      }
    }
  }

  return result;
}

}  // namespace tvsc::discovery