#include "discovery/service_advertiser.h"

#include <exception>

#include "avahi-client/client.h"
#include "avahi-client/publish.h"
#include "avahi-common/error.h"
#include "avahi-common/simple-watch.h"
#include "glog/logging.h"

namespace tvsc::discovery {

void SingleServiceAdvertiser::on_group_change(AvahiEntryGroup *group, AvahiEntryGroupState state,
                                              void *single_service_advertiser) {
  SingleServiceAdvertiser *advertiser{
      static_cast<SingleServiceAdvertiser *>(single_service_advertiser)};

  /* Called whenever the entry group state changes */
  switch (state) {
    case AVAHI_ENTRY_GROUP_ESTABLISHED:
      /* The entry group has been established successfully */
      LOG(INFO) << "on_group_change() -- AVAHI_ENTRY_GROUP_ESTABLISHED";
      // Signal service advertisement requests as succeeded.
      break;
    case AVAHI_ENTRY_GROUP_COLLISION: {
      LOG(INFO) << "on_group_change() -- AVAHI_ENTRY_GROUP_COLLISION";
      // Signal service advertisement requests as failed due to name collision.
      break;
    }
    case AVAHI_ENTRY_GROUP_FAILURE: {
      const int avahi_errno{avahi_client_errno(avahi_entry_group_get_client(group))};
      LOG(INFO) << "on_group_change() -- AVAHI_ENTRY_GROUP_FAILURE: "
                << avahi_strerror(avahi_errno);
      /* Some kind of failure happened while we were registering our services */
      // Signal service advertisement requests as failed.
      break;
    }
    case AVAHI_ENTRY_GROUP_UNCOMMITED:
      LOG(INFO) << "on_group_change() -- AVAHI_ENTRY_GROUP_UNCOMMITED. Ignored.";
      break;
    case AVAHI_ENTRY_GROUP_REGISTERING:
      LOG(INFO) << "on_group_change() -- AVAHI_ENTRY_GROUP_REGISTERING. Ignored.";
      break;
  }
}

void SingleServiceAdvertiser::normalize_names() {
  if (services_.published_name().empty()) {
    services_.set_published_name(services_.canonical_name());
  }
}

SingleServiceAdvertiser::SingleServiceAdvertiser(AvahiClient &client,
                                                 AdvertisementCallback callback)
    : client_(&client), callback_(std::move(callback)) {}

void SingleServiceAdvertiser::set_services(const ServiceSet &services) {
  reset_group();
  services_ = services;
  normalize_names();
}

void SingleServiceAdvertiser::create_group() {
  group_.reset(avahi_entry_group_new(client_, SingleServiceAdvertiser::on_group_change, this));
}

void SingleServiceAdvertiser::reset_group() {
  if (group_) {
    avahi_entry_group_reset(group_.get());
  }
}

void SingleServiceAdvertiser::populate_group() {
  for (const auto &service : services_.services()) {
    // TODO(james): Implement the allowed_interface_names from the ServiceDescriptor proto.
    int result = avahi_entry_group_add_service(
        group_.get(), AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, static_cast<AvahiPublishFlags>(0),
        services_.published_name().c_str(), service.service_type().c_str(), nullptr /* domain */,
        nullptr /* host */, service.port(), NULL);
    if (result < 0) {
      LOG(INFO) << "populate_group() -- failure: " << avahi_strerror(result);
      if (result == AVAHI_ERR_COLLISION) {
        on_group_change(group_.get(), AVAHI_ENTRY_GROUP_COLLISION, this);
      } else {
        on_group_change(group_.get(), AVAHI_ENTRY_GROUP_FAILURE, this);
      }
    }
  }
}

void SingleServiceAdvertiser::publish() {
  if (!group_) {
    create_group();
  }
  populate_group();
  int result = avahi_entry_group_commit(group_.get());
  if (result < 0) {
    LOG(INFO) << "publish() -- failure: " << avahi_strerror(result);
    if (result == AVAHI_ERR_COLLISION) {
      on_group_change(group_.get(), AVAHI_ENTRY_GROUP_COLLISION, this);
    } else {
      on_group_change(group_.get(), AVAHI_ENTRY_GROUP_FAILURE, this);
    }
  }
}

void ServiceAdvertiser::on_client_change(AvahiClient *client, AvahiClientState state,
                                         void *service_advertiser) {
  ServiceAdvertiser *advertiser{static_cast<ServiceAdvertiser *>(service_advertiser)};
  switch (state) {
    case AVAHI_CLIENT_S_RUNNING:
      LOG(INFO) << "on_client_change() -- AVAHI_CLIENT_S_RUNNING.";
      // The server has startup successfully and registered its host
      // name on the network, so now we can register services.
      advertiser->have_valid_client_.store(true);
      break;
    case AVAHI_CLIENT_FAILURE:
      LOG(INFO) << "on_client_change() -- AVAHI_CLIENT_FAILURE.";
      advertiser->have_valid_client_.store(false);
      break;
    case AVAHI_CLIENT_S_COLLISION:
      LOG(INFO) << "on_client_change() -- AVAHI_CLIENT_S_COLLISION: unhandled. TODO: Handle this "
                   "case by re-registering all previous services.";
      /* Let's drop our registered services. When the server is back
       * in AVAHI_SERVER_RUNNING state we will register them
       * again with the new host name. */
      advertiser->have_valid_client_.store(false);
      break;
    case AVAHI_CLIENT_S_REGISTERING:
      LOG(INFO) << "on_client_change() -- AVAHI_CLIENT_S_REGISTERING: unhandled. TODO: Handle this "
                   "case by re-registering all previous services.";
      /* The server records are now being established. This
       * might be caused by a host name change. We need to wait
       * for our own records to register until the host name is
       * properly esatblished. */
      advertiser->have_valid_client_.store(false);
      break;
    case AVAHI_CLIENT_CONNECTING:
      LOG(INFO) << "on_client_change() -- AVAHI_CLIENT_CONNECTING: Ignored.";
      break;
  }
}

ServiceAdvertiser::ServiceAdvertiser() {
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

  watcher_task_ = std::async(std::launch::async, &avahi_simple_poll_loop, watcher_.get());
  if (!watcher_task_.valid()) {
    throw std::runtime_error("Could not start the avahi_simple_poll_loop()");
  }
}

ServiceAdvertiser::~ServiceAdvertiser() {
  if (watcher_) avahi_simple_poll_quit(watcher_.get());
  if (watcher_task_.valid()) {
    watcher_task_.wait();
  }
}

void ServiceAdvertiser::advertise_service(const ServiceSet &service,
                                          AdvertisementCallback callback) {
  if (service.canonical_name().empty()) {
    throw std::invalid_argument("service.canonical_name must be set.");
  }
  if (services_.count(service.canonical_name()) == 0) {
    services_.emplace(service.canonical_name(), SingleServiceAdvertiser(*avahi_client_, callback));
  }
  auto &service_advertiser{services_.at(service.canonical_name())};
  service_advertiser.set_services(service);
  if (have_valid_client_.load()) {
    service_advertiser.publish();
  }
}

}  // namespace tvsc::discovery
