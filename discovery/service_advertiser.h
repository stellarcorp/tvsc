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

enum class AdvertisementResult {
  SUCCESS,
  COLLISION,
  FAILURE,
};
using AdvertisementCallback = std::function<void(AdvertisementResult result)>;
std::string to_string(AdvertisementResult result);

class SingleServiceAdvertiser final {
 private:
  AvahiClient* client_;
  AdvertisementCallback callback_;
  ServiceSet services_{};
  std::unique_ptr<AvahiEntryGroup, int (*)(AvahiEntryGroup*)> group_{nullptr,
                                                                     avahi_entry_group_free};

  static void on_group_change(AvahiEntryGroup* group, AvahiEntryGroupState state,
                              void* service_set);

  void create_group();

  void normalize_names();

  void reset_group();
  void populate_group();

  SingleServiceAdvertiser(AvahiClient& client, AdvertisementCallback callback);

  friend class ServiceAdvertiser;

 public:
  const std::string& published_name() const { return services_.published_name(); }

  void set_services(const ServiceSet& services);
  void reset() { reset_group(); }

  void publish();
};

class ServiceAdvertiser final {
 private:
  std::unique_ptr<AvahiSimplePoll, void (*)(AvahiSimplePoll*)> watcher_{nullptr,
                                                                        avahi_simple_poll_free};
  std::future<int> watcher_task_{};

  std::unique_ptr<AvahiClient, void (*)(AvahiClient*)> avahi_client_{nullptr, avahi_client_free};

  // Map of the service canonical name to the SingleServiceAdvertiser that handles it.
  std::map<std::string, SingleServiceAdvertiser> services_{};

  std::atomic<bool> have_valid_client_{false};

  static void on_client_change(AvahiClient* client, AvahiClientState state,
                               void* service_advertiser);

 public:
  // TODO(james): Pass in a ThreadPool and run the AvahiSimplePoll watcher on its threads.
  ServiceAdvertiser();
  ~ServiceAdvertiser();

  /**
   * Advertise the set of services given by the ServiceSet.
   *
   * This set of services will be identified within this advertiser by its
   * ServiceSet.canonical_name. It will be published on the network by its ServiceSet.published_name
   * if that is set, or by the canonical name if the published name is not set.
   *
   * Note that if another set of services was previously registered with the same canonical name,
   * this set will supercede the previous set.
   */
  void advertise_local_service(const std::string& service_name, const std::string& service_type,
                               const std::string& domain, int port, AdvertisementCallback callback);

  bool contains(const std::string& canonical_service_name) const {
    return services_.count(canonical_service_name) > 0;
  }

  const std::string& lookup_published_name(const std::string& canonical_service_name) const {
    return services_.at(canonical_service_name).published_name();
  }
};

}  // namespace tvsc::discovery
