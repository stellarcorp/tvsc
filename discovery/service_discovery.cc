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
                                        void *service_discovery) {
  // TODO(james): Implement these states to give better error handling.
  // ServiceDiscovery *discovery{static_cast<ServiceDiscovery *>(service_discovery)};
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
                                         void *service_discovery) {
  ServiceDiscovery *discovery{static_cast<ServiceDiscovery *>(service_discovery)};
  /* Called whenever a new service becomes available on the LAN or is removed from the LAN */
  switch (event) {
    case AVAHI_BROWSER_NEW: {
      LOG(INFO) << "on_browser_change() -- AVAHI_BROWSER_NEW: service '" << name << "' of type '"
                << service_type << "' in domain '" << domain << "'";
      discovery->register_resolver(service_type);
      // Note: the resolver gets freed in on_resolver_change().
      auto resolver = avahi_service_resolver_new(
          avahi_service_browser_get_client(avahi_browser), interface, protocol, name, service_type,
          domain, AVAHI_PROTO_UNSPEC, static_cast<AvahiLookupFlags>(0), on_resolver_change,
          discovery);
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
      discovery->register_resolver(service_type);
      // Note: the resolver gets freed in on_resolver_change().
      auto resolver = avahi_service_resolver_new(
          avahi_service_browser_get_client(avahi_browser), interface, protocol, name, service_type,
          domain, AVAHI_PROTO_UNSPEC, static_cast<AvahiLookupFlags>(0), on_resolver_change,
          discovery);
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
      discovery->publish_changes(service_type);
      break;
    }
    case AVAHI_BROWSER_CACHE_EXHAUSTED: {
      LOG(INFO) << "on_browser_change() -- AVAHI_BROWSER_CACHE_EXHAUSTED. service_type: "
                << service_type;
      // TODO(james): Determine how this differs from the AVAHI_BROWSER_ALL_FOR_NOW signal.
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
                                          void *service_discovery) {
  ServiceDiscovery *discovery{static_cast<ServiceDiscovery *>(service_discovery)};
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
      discovery->add_resolution(service_type, hostname, protocol, *address, interface, port);
      break;
    }
  }
  discovery->unregister_resolver(service_type);
  avahi_service_resolver_free(resolver);
}

void ServiceDiscovery::publish_changes(const std::string &service_type) {
  DLOG(INFO) << "publish_changes() -- service_type: " << service_type
             << ", changes_in_progress_.size(): " << changes_in_progress_.size();

  std::lock_guard lock{discovery_mutex_};
  if (have_done_a_service_resolution(service_type) and have_resolvers_in_flight(service_type)) {
    // For the first condition, we want to publish the empty results if we haven't had any
    // services on the network of this service type to resolve. This would happen because we
    // launched an avahi_browser, it signalled ALL_FOR_NOW, but did not signal any NEW entries that
    // we would need to resolve.
    //
    // For the second condition, we don't want to publish changes for this service_type unless all
    // of the resolvers for it have completed their work.
    return;
  }

  discovered_services_.erase(service_type);
  for (auto [iter, end] = changes_in_progress_.equal_range(service_type); iter != end;
       /* Increment in body */) {
    discovered_services_.insert(changes_in_progress_.extract(iter++));
  }
  DLOG(INFO) << "publish_changes() -- after moving nodes: service_type: " << service_type
             << ", changes_in_progress_.size(): " << changes_in_progress_.size();

  for (auto iter = service_type_watchers_.find(service_type); iter != service_type_watchers_.end();
       ++iter) {
    if (service_type == iter->first) {
      auto &ready_callback = iter->second;
      ready_callback(service_type);
    } else {
      LOG(INFO) << "ServiceDiscovery::publish_changes() -- break on service_type '" << service_type
                << "'";
      break;
    }
  }

  service_type_watchers_.erase(service_type);
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
    std::this_thread::sleep_for(10ms);
    std::lock_guard lock{this->avahi_call_mutex_};
    int result = avahi_simple_poll_iterate(this->loop_.get(), 0);
    if (result != 0) {
      LOG(INFO) << "Poll loop -- exiting with result: " << result;
      return result;
    }
  }
  LOG(INFO) << "Poll loop -- exited.";
}

void ServiceDiscovery::add_resolution(const std::string &service_type, const std::string &hostname,
                                      AvahiProtocol protocol, const AvahiAddress &address,
                                      AvahiIfIndex interface, int port) {
  ServerDetails server{};
  server.set_hostname(hostname);
  *server.mutable_address() = avahi_address_to_network_address(protocol, address, interface);
  server.set_port(port);

  {
    std::lock_guard lock{discovery_mutex_};
    changes_in_progress_.emplace(service_type, std::move(server));
  }

  DLOG(INFO) << "ServiceDiscovery::add_resolution() -- changes_in_progress_.size(): "
             << changes_in_progress_.size();
}

std::vector<std::string> ServiceDiscovery::service_types() const {
  std::vector<std::string> types{};

  std::lock_guard lock{discovery_mutex_};
  for (const auto &entry : discovered_services_) {
    types.emplace_back(entry.first);
  }
  return types;
}

void ServiceDiscovery::add_service_type(const std::string &service_type,
                                        ReadyWatcher ready_callback) {
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
    service_type_watchers_.emplace(service_type, std::move(ready_callback));
  } else {
    ready_callback(service_type);
  }
}

void ServiceDiscovery::add_service_type_and_block_until_ready(const std::string &service_type) {
  std::mutex m{};
  std::condition_variable condition{};
  add_service_type(service_type, [&m, &condition](const std::string & /*unused*/) {
    std::unique_lock lock{m};
    condition.notify_one();
  });
  std::unique_lock lock{m};
  // Block here until the ready callback (lambda) above gets called.
  condition.wait(lock);
}

}  // namespace tvsc::discovery
