#pragma once

#include <atomic>
#include <future>
#include <map>
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

#include "avahi-client/client.h"
#include "avahi-client/lookup.h"
#include "avahi-common/simple-watch.h"
#include "discovery/service_descriptor.pb.h"

namespace tvsc::discovery {

class ServiceBrowser final {
 private:
  std::unique_ptr<AvahiSimplePoll, void (*)(AvahiSimplePoll*)> watcher_{nullptr,
                                                                        avahi_simple_poll_free};
  std::future<int> watcher_task_{};

  std::unique_ptr<AvahiClient, void (*)(AvahiClient*)> avahi_client_{nullptr, avahi_client_free};

  /**
   * Map of service browsers by the service type (such as "_http._tcp") they are browsing for.
   */
  std::map<std::string, std::unique_ptr<AvahiServiceBrowser, int (*)(AvahiServiceBrowser*)>>
      avahi_browsers_{};

  std::atomic<bool> have_valid_client_{false};

  std::map<std::string, ServiceSet> discovered_services_{};

  static void on_client_change(AvahiClient* client, AvahiClientState state, void* service_browser);

  static void on_browser_change(AvahiServiceBrowser* avahi_browser, AvahiIfIndex interface,
                                AvahiProtocol protocol, AvahiBrowserEvent event, const char* name,
                                const char* type, const char* domain, AvahiLookupResultFlags flags,
                                void* service_browser);

  static void on_resolver_change(AvahiServiceResolver* resolver, AvahiIfIndex interface,
                                 AvahiProtocol protocol, AvahiResolverEvent event, const char* name,
                                 const char* type, const char* domain, const char* host_name,
                                 const AvahiAddress* address, uint16_t port, AvahiStringList* txt,
                                 AvahiLookupResultFlags flags, void* service_browser);

  void add_unresolved_service(const std::string& name, const std::string& type,
                              const std::string& domain);
  void remove_unresolved_service(const std::string& name, const std::string& type,
                                 const std::string& domain);
  void add_resolved_service(const std::string& name, const std::string& type,
                            const std::string& domain, const std::string& hostname,
                            const NetworkAddress& address, int port);

 public:
  // TODO(james): Pass in a ThreadPool and run the AvahiSimplePoll watcher on its threads.
  ServiceBrowser(const std::unordered_set<std::string>& service_types);
  ~ServiceBrowser();

  /**
   * List the set of service types being listened for.
   */
  std::unordered_set<std::string> service_types() const;
  /**
   * Add a service type to listen for.
   */
  void add_service_type(const std::string& service_type);
  /**
   * Remove a service type to listen for.
   */
  void remove_service_type(const std::string& service_type);

  /**
   * Get the set of discovered service names.
   */
  std::unordered_set<std::string> service_names() const;
};

}  // namespace tvsc::discovery
