#include "discovery/service_advertiser.h"

#include <exception>

#include "avahi-client/client.h"
#include "avahi-client/publish.h"
#include "avahi-common/error.h"
#include "avahi-common/simple-watch.h"
#include "discovery/network_address_utils.h"
#include "glog/logging.h"

namespace tvsc::discovery {

void SingleServiceAdvertiser::on_group_change(AvahiEntryGroup *group, AvahiEntryGroupState state,
                                              void *single_service_advertiser) {
  // SingleServiceAdvertiser *advertiser{
  //     static_cast<SingleServiceAdvertiser *>(single_service_advertiser)};

  switch (state) {
    case AVAHI_ENTRY_GROUP_ESTABLISHED: {
      LOG(INFO) << "on_group_change() -- AVAHI_ENTRY_GROUP_ESTABLISHED";
      // Signal service advertisement requests as succeeded.
      // advertiser->callback_(AdvertisementResult::SUCCESS);
      break;
    }
    case AVAHI_ENTRY_GROUP_COLLISION: {
      LOG(INFO) << "on_group_change() -- AVAHI_ENTRY_GROUP_COLLISION";
      // Signal service advertisement requests as failed due to name collision.
      // advertiser->callback_(AdvertisementResult::COLLISION);
      break;
    }
    case AVAHI_ENTRY_GROUP_FAILURE: {
      const int avahi_errno{avahi_client_errno(avahi_entry_group_get_client(group))};
      LOG(INFO) << "on_group_change() -- AVAHI_ENTRY_GROUP_FAILURE: "
                << avahi_strerror(avahi_errno);
      /* Some kind of failure happened while we were registering our services */
      // Signal service advertisement requests as failed.
      // advertiser->callback_(AdvertisementResult::FAILURE);
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

SingleServiceAdvertiser::SingleServiceAdvertiser(AvahiClient &client) : client_(&client) {}

void SingleServiceAdvertiser::create_group() {
  group_.reset(avahi_entry_group_new(client_, SingleServiceAdvertiser::on_group_change, this));
}

void SingleServiceAdvertiser::reset_group() {
  if (group_) {
    avahi_entry_group_reset(group_.get());
  }
}

int SingleServiceAdvertiser::populate_group(const ServiceSet &services) {
  int result{AVAHI_OK};
  std::string publish_name{services.published_name()};
  if (publish_name.empty()) {
    publish_name = services.canonical_name();
  }
  for (const auto &service : services.services()) {
    for (const auto &server : service.servers()) {
      result = avahi_entry_group_add_service(
          group_.get(), AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, static_cast<AvahiPublishFlags>(0),
          publish_name.c_str(), service.service_type().c_str(), service.domain().c_str(),
          nullptr /* host */, server.port(), nullptr);
      if (result != AVAHI_OK) {
        LOG(INFO) << "populate_group() -- failure: " << avahi_strerror(result);
        return result;
      }
    }
  }
  return result;
}

void SingleServiceAdvertiser::advertise(const ServiceSet &services) {
  if (!group_) {
    create_group();
  } else {
    reset_group();
  }

  int result = populate_group(services);
  if (result != AVAHI_OK) {
    goto failure;
  }

  result = avahi_entry_group_commit(group_.get());

  if (result != AVAHI_OK) {
  failure:
    LOG(INFO) << "advertise() -- failure: " << avahi_strerror(result);
    if (result == AVAHI_ERR_COLLISION) {
      on_group_change(group_.get(), AVAHI_ENTRY_GROUP_COLLISION, this);
    } else {
      on_group_change(group_.get(), AVAHI_ENTRY_GROUP_FAILURE, this);
    }
  } else {
    LOG(INFO) << "advertise() -- success. services:\n" << services.DebugString();
  }
}

void ServiceAdvertiser::on_client_change(AvahiClient *client, AvahiClientState state,
                                         void *service_advertiser) {
  // TODO(james): Investigate which states get triggered when waking up from a sleep, DHCP address
  // change, etc. and implement accordingly.
  // ServiceAdvertiser *advertiser{static_cast<ServiceAdvertiser *>(service_advertiser)};
  switch (state) {
    case AVAHI_CLIENT_S_RUNNING: {
      LOG(INFO) << "on_client_change() -- AVAHI_CLIENT_S_RUNNING.";
      // The server has startup successfully and registered its host
      // name on the network, so now we can register services.
      break;
    }
    case AVAHI_CLIENT_FAILURE: {
      LOG(INFO) << "on_client_change() -- AVAHI_CLIENT_FAILURE.";
      break;
    }
    case AVAHI_CLIENT_S_COLLISION: {
      LOG(INFO) << "on_client_change() -- AVAHI_CLIENT_S_COLLISION: unhandled. TODO: Handle this "
                   "case by re-registering all previous services.";
      /* Let's drop our registered services. When the server is back
       * in AVAHI_SERVER_RUNNING state we will register them
       * again with the new host name. */
      break;
    }
    case AVAHI_CLIENT_S_REGISTERING: {
      LOG(INFO) << "on_client_change() -- AVAHI_CLIENT_S_REGISTERING: unhandled. TODO: Handle this "
                   "case by re-registering all previous services.";
      /* The server records are now being established. This
       * might be caused by a host name change. We need to wait
       * for our own records to register until the host name is
       * properly esatblished. */
      break;
    }
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

void ServiceAdvertiser::advertise_service(const std::string &service_name,
                                          const std::string &service_type,
                                          const std::string &domain, int port) {
  if (service_name.empty()) {
    throw std::invalid_argument("service_name must not be empty.");
  }
  if (service_type.empty()) {
    throw std::invalid_argument("service_type must not be empty.");
  }
  if (domain.empty()) {
    throw std::invalid_argument("domain must not be empty.");
  }
  if ((port <= 0) or (port > 65535)) {
    throw std::invalid_argument("port must be a positive integer less than 65536.");
  }

  ServiceSet service_set{};
  service_set.set_canonical_name(service_name);
  ServiceDescriptor *service{service_set.add_services()};
  service->set_service_type(service_type);
  service->set_domain(domain);
  ServerDetails *server{service->add_servers()};
  server->set_hostname(get_hostname());
  server->set_port(port);

  if (services_.count(service_name) == 0) {
    services_.emplace(service_name, SingleServiceAdvertiser(*avahi_client_));
  }

  auto &service_advertiser{services_.at(service_name)};
  service_advertiser.advertise(service_set);
}

}  // namespace tvsc::discovery
