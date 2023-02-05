#pragma once

#include <functional>
#include <future>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_set>
#include <vector>

#include "avahi-client/client.h"
#include "avahi-client/lookup.h"
#include "avahi-common/simple-watch.h"
#include "discovery/service_descriptor.pb.h"

namespace tvsc::discovery {

/**
 * Mechanism to discover services and watch for changes in those services.
 */
class ServiceDiscovery final {
 public:
  using ReadyWatcher = std::function<void(const std::string& service_type)>;

 private:
  std::unique_ptr<AvahiSimplePoll, void (*)(AvahiSimplePoll*)> loop_{nullptr,
                                                                     avahi_simple_poll_free};
  std::future<int> loop_task_{};

  std::unique_ptr<AvahiClient, void (*)(AvahiClient*)> avahi_client_{nullptr, avahi_client_free};

  /**
   * Map of service browsers by the service type (such as "_http._tcp") they are browsing for.
   */
  std::map<std::string, std::unique_ptr<AvahiServiceBrowser, int (*)(AvahiServiceBrowser*)>>
      avahi_browsers_{};

  /**
   * Since we run Avahi via its AvahiSimplePoll class, it does not provide any locking against
   * access from multiple threads. That's appropriate for the library, but because of our usage, we
   * need to lock our accesses to its objects. The polling loop locks this mutex before calling the
   * AvahiSimplePoll's method to gather events and run our callbacks. We must also lock it any time
   * we are adding event sources, like creating new AvahiServiceBrowsers.
   */
  std::mutex avahi_call_mutex_{};

  /**
   * Map of published name to all of its serving details.
   */
  std::multimap<std::string, ServerDetails> discovered_services_{};

  /**
   * Map of incoming resolution information by service type. This map contains partial information
   * as it is being accumulated via the Avahi browsers and resolvers. It gets published one service
   * type at a time, not as a full set of changes. The publishing is triggered by the avahi_browser
   * state AVAHI_BROWSER_ALL_FOR_NOW for each service_type.
   */
  std::multimap<std::string, ServerDetails> changes_in_progress_{};

  /**
   * Mutex guarding access to data members of this class.
   */
  mutable std::mutex discovery_mutex_{};

  /**
   * Count of resolver requests in flight for each service type.
   *
   * We wait until all of the resolvers for a given service type have given their results before we
   * publish those results in the publish_changes() method.
   */
  std::map<std::string, int> resolvers_in_flight_{};

  /**
   * Map of watchers by service type. When a new server providing a service of that service type is
   * discovered, those watchers will get notified.
   */
  std::multimap<std::string, ReadyWatcher> service_type_watchers_{};

  static void on_client_change(AvahiClient* client, AvahiClientState state, void* service_browser);

  static void on_browser_change(AvahiServiceBrowser* avahi_browser, AvahiIfIndex interface,
                                AvahiProtocol protocol, AvahiBrowserEvent event, const char* name,
                                const char* service_type, const char* domain,
                                AvahiLookupResultFlags flags, void* service_browser);

  static void on_resolver_change(AvahiServiceResolver* resolver, AvahiIfIndex interface,
                                 AvahiProtocol protocol, AvahiResolverEvent event, const char* name,
                                 const char* type, const char* domain, const char* host_name,
                                 const AvahiAddress* address, uint16_t port, AvahiStringList* txt,
                                 AvahiLookupResultFlags flags, void* service_browser);

  void add_resolution(const std::string& type, const std::string& hostname, AvahiProtocol protocol,
                      const AvahiAddress& address, AvahiIfIndex interface, int port);

  void register_resolver(const std::string& service_type) {
    std::lock_guard lock{discovery_mutex_};
    ++resolvers_in_flight_[service_type];
  }

  void unregister_resolver(const std::string& service_type) {
    {
      std::lock_guard lock{discovery_mutex_};
      --resolvers_in_flight_[service_type];
    }
    publish_changes(service_type);
  }

  bool have_done_a_service_resolution(const std::string& service_type) const {
    return resolvers_in_flight_.count(service_type) > 0;
  }

  bool have_resolvers_in_flight(const std::string& service_type) const {
    return resolvers_in_flight_.at(service_type) > 0;
  }

  /**
   * Notify the watchers for the given service type.
   */
  void publish_changes(const std::string& service_type);

  /**
   * Main loop to process events. Should be run asynchronously.
   */
  int polling_loop();

 public:
  // TODO(james): Pass in a ThreadPool and run the AvahiSimplePoll watcher on its threads.
  ServiceDiscovery();
  ~ServiceDiscovery();

  /**
   * List the set of service types being listened for.
   *
   * Note that a service type will not appear in this list until we have done an initial lookup of
   * that service type. That is, the service_type will not appear here until the ReadyWatcher has
   * been notified that this class is ready to resolve that service_type.
   */
  std::vector<std::string> service_types() const;

  /**
   * Add a service type to be resolved.
   *
   * This method calls the callback when it first has information about the requested service type.
   * Note that the callback is only called once when the service_type is first resolved, not on
   * every change to the resolution.
   */
  void add_service_type(const std::string& service_type, ReadyWatcher watcher);

  /**
   * Convenience method to add a service type to be resolved and block until that service type is
   * ready.
   */
  void add_service_type_and_block_until_ready(const std::string& service_type);

  /**
   * Resolve the service type into a set of servers providing that service as currently known. Note
   * that this list is a snapshot and is likely to change as servers go up and down, network
   * addresses change, etc.
   */
  std::vector<ServerDetails> resolve(const std::string& service_type) const {
    std::vector<ServerDetails> result{};

    std::lock_guard lock{discovery_mutex_};
    for (auto [iter, end] = discovered_services_.equal_range(service_type); iter != end; ++iter) {
      result.emplace_back(iter->second);
    }

    return result;
  }
};

}  // namespace tvsc::discovery
