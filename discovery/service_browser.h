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
class ServiceBrowser final {
 public:
  using ServiceTypeWatcher = std::function<void(const std::string& service_type)>;

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
  std::map<std::string, ServiceSet> discovered_services_{};

  /**
   * Map of watchers by service type. When a new server providing a service of that service type is
   * discovered, those watchers will get notified.
   */
  std::multimap<std::string, ServiceTypeWatcher> service_type_watchers_{};
  std::map<std::string, int> service_type_resolvers_in_flight_{};

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

  void add_service(const std::string& name, const std::string& type, const std::string& domain);
  void clear_service_records(const std::string& name, const std::string& type,
                             const std::string& domain);

  bool has_server(const ServiceDescriptor& service, const std::string& hostname,
                  const NetworkAddress& address, int port) const;

  void add_server(ServiceDescriptor& service, const std::string& hostname,
                  const NetworkAddress& address, int port);

  void add_server(const std::string& name, const std::string& type, const std::string& domain,
                  const std::string& hostname, const NetworkAddress& address, int port);

  const ServiceDescriptor& lookup_service(const std::string& name, const std::string& type,
                                          const std::string& domain) const;
  ServiceDescriptor& lookup_service(const std::string& name, const std::string& type,
                                    const std::string& domain);

  /**
   * Notify the watchers for the given service type.
   */
  void update_watchers(const std::string& service_type) const;

  /**
   * Main loop to process events. Should be run asynchronously.
   */
  int polling_loop();

 public:
  // TODO(james): Pass in a ThreadPool and run the AvahiSimplePoll watcher on its threads.
  ServiceBrowser();
  ~ServiceBrowser();

  /**
   * List the set of service types being listened for.
   */
  std::unordered_set<std::string> service_types() const;

  /**
   * Watch changes on a service type.
   *
   * This method calls the callback when it has new information about the requested service type.
   */
  void watch_service_type(const std::string& service_type, ServiceTypeWatcher watcher);

  /**
   * Resolve the service type into a set of servers providing that service.
   *
   * This method returns a snapshot of the currently known discovered services. To keep this
   * information up-to-date, you should also set up a watcher on this service type.
   */
  std::vector<ServerDetails> resolve_service_type(const std::string& service_type) const;

  /**
   * Get all discovered services and the servers implementing those services.
   */
  const std::map<std::string, ServiceSet>& all_discovered_services() const {
    return discovered_services_;
  }
};

}  // namespace tvsc::discovery
