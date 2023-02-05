#pragma once

#include <atomic>
#include <functional>
#include <future>
#include <map>
#include <memory>
#include <string>

#include "avahi-client/client.h"
#include "avahi-client/publish.h"
#include "avahi-common/simple-watch.h"
#include "discovery/service_descriptor.pb.h"

namespace tvsc::discovery {

class SingleServiceAdvertiser final {
 private:
  AvahiClient* client_;

  std::unique_ptr<AvahiEntryGroup, int (*)(AvahiEntryGroup*)> group_{nullptr,
                                                                     avahi_entry_group_free};

  static void on_group_change(AvahiEntryGroup* group, AvahiEntryGroupState state,
                              void* service_set);

  void create_group();
  void reset_group();
  int populate_group(const ServiceSet& services);

  SingleServiceAdvertiser(AvahiClient& client);
  void advertise(const ServiceSet& services);

  friend class ServiceAdvertiser;
};

class ServiceAdvertiser final {
 private:
  std::unique_ptr<AvahiSimplePoll, void (*)(AvahiSimplePoll*)> watcher_{nullptr,
                                                                        avahi_simple_poll_free};
  std::future<int> watcher_task_{};

  std::unique_ptr<AvahiClient, void (*)(AvahiClient*)> avahi_client_{nullptr, avahi_client_free};

  // Map of the service canonical name to the SingleServiceAdvertiser that handles it.
  std::map<std::string, SingleServiceAdvertiser> services_{};

  static void on_client_change(AvahiClient* client, AvahiClientState state,
                               void* service_advertiser);

 public:
  // TODO(james): Pass in a ThreadPool and run the AvahiSimplePoll watcher on its threads.
  ServiceAdvertiser();
  ~ServiceAdvertiser();

  /**
   * Advertise a service running on this server.
   *
   * This method will overwrite all advertisements for this service name. It allows for only a
   * single service per service name.
   *
   * We restrict the API for this class to a single service type per service name. That is not as
   * general as possible; it is quite reasonable for a single service to provide multiple service
   * types. But, we have not encountered that requirement in our code yet. When necessary, we can
   * add methods to advertise multiple service types for a single service name.
   */
  void advertise_service(const std::string& service_name, const std::string& service_type,
                         const std::string& domain, int port);
};

}  // namespace tvsc::discovery
