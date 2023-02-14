#pragma once

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "discovery/service_advertiser.h"
#include "discovery/service_types.h"
#include "glog/logging.h"
#include "grpcpp/grpcpp.h"

namespace tvsc::service::utility {

/**
 * Utility for running one or more services, including logging and service advertisement via mDNS.
 */
class ServiceRunner final {
 private:
  std::unique_ptr<grpc::Server> server_{nullptr};
  grpc::ServerBuilder builder_{};
  tvsc::discovery::ServiceAdvertiser advertiser_{};

  std::vector<std::function<void()>> advertisements_{};

  int port_{0};

 public:
  /**
   * Add a service to be included in this server.
   */
  template <typename ServiceT, typename ServiceImplT>
  void add_service(const std::string& service_name, ServiceImplT& service) {
    builder_.RegisterService(&service);

    const std::string service_type{tvsc::discovery::generate_service_type<ServiceT>()};
    advertisements_.emplace_back([this, service_name, service_type]() {
      LOG(INFO) << service_name << " (" << service_type << ")"
                << " server listening on port " << this->port_;
      this->advertiser_.advertise_service(service_name, service_type, "local", this->port_);
    });
  }

  /**
   * Set the port to listen on. Only effective before the services are running.
   */
  void set_port(int port) { port_ = port; }

  /**
   * Get the port the server will listen on. Will be zero until the services are running, if the
   * port was not explicitly set.
   */
  int port() const { return port_; }

  /**
   * Start the server.
   *
   * This method selects an unused port if a port was not set. Note that after
   * this method, the port() getter will return the actual port that is being used and the
   * set_port() setter will no longer affect which port is to be listened on.
   */
  void start() {
    using std::to_string;

    // Bind the server to a particular interface and port.
    const std::string bind_address{"dns:///[::]:" + to_string(port_)};
    builder_.AddListeningPort(bind_address, grpc::InsecureServerCredentials(), &port_);

    // Start the server. This returns when the server is ready to accept connections.
    DLOG(INFO) << "ServiceRunner::run_and_wait() -- starting server.";
    server_ = builder_.BuildAndStart();

    // Advertise the services.
    DLOG(INFO) << "ServiceRunner::run_and_wait() -- advertising the services via mDNS.";
    for (auto& advertise_fn : advertisements_) {
      advertise_fn();
    }
  }

  /**
   * Block until the server finishes.
   */
  void wait() {
    DLOG(INFO) << "ServiceRunner::wait() -- calling Wait().";
    server_->Wait();
    DLOG(INFO) << "ServiceRunner::wait() -- exiting.";
  }
};

template <typename ServiceT, typename ServiceImplT>
void run_single_service(const std::string& service_name) {
  ServiceRunner runner{};
  ServiceImplT service{};
  runner.add_service<ServiceT, ServiceImplT>(service_name, service);
  runner.start();
  runner.wait();
}

}  // namespace tvsc::service::utility
