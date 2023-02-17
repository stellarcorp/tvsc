#pragma once

#include <chrono>
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
  struct ServiceDescriptor final {
    std::string service_name;
    std::string service_type;
    grpc::Service* service;
  };

  std::unique_ptr<grpc::ServerBuilder> builder_{nullptr};
  std::unique_ptr<grpc::Server> server_{nullptr};
  std::unique_ptr<tvsc::discovery::ServiceAdvertiser> advertiser_{nullptr};

  std::vector<ServiceDescriptor> services_{};

  int port_{0};
  bool was_port_requested_{false};

  void reset() {
    if (!was_port_requested_) {
      port_ = 0;
    }

    // Note: destroying the ServiceAdvertiser instance signals to clients that the services are no
    // longer available.
    advertiser_.reset();
    server_.reset();
    builder_.reset();
  }

 public:
  /**
   * Add a service to be included in this server when the server is started.
   *
   * Note that this operation won't add the service until the server is started. If the server is
   * already running, this service will NOT be added to it until the runner is stopped and
   * restarted.
   */
  template <typename ServiceT, typename ServiceImplT>
  void add_service(const std::string& service_name, ServiceImplT& service) {
    const std::string service_type{tvsc::discovery::generate_service_type<ServiceT>()};

    services_.push_back({service_name, service_type, &service});
  }

  /**
   * Set the port to listen on. Only effective before the services are running.
   */
  void set_port(int port) {
    port_ = port;
    if (port != 0) {
      was_port_requested_ = true;
    } else {
      was_port_requested_ = false;
    }
  }

  /**
   * Get the port the server will listen on. Will be zero when the services are not running, if the
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

    if (server_) {
      // We are already running.
      return;
    }

    // Create a new ServerBuilder.
    builder_ = std::make_unique<grpc::ServerBuilder>();

    // Register all of the services with the ServerBuilder.
    for (auto& descriptor : services_) {
      builder_->RegisterService(descriptor.service);
    }

    // Bind the server to a particular interface and port.
    const std::string bind_address{"dns:///[::]:" + to_string(port_)};
    builder_->AddListeningPort(bind_address, grpc::InsecureServerCredentials(), &port_);

    // Start the server. This call returns when the server is ready to accept connections.
    DLOG(INFO) << "ServiceRunner::start() -- starting server.";
    server_ = builder_->BuildAndStart();

    // All of the services are up. Advertise the services.
    advertiser_ = std::make_unique<tvsc::discovery::ServiceAdvertiser>();
    DLOG(INFO) << "ServiceRunner::start() -- advertising the services via mDNS.";
    for (const auto& descriptor : services_) {
      this->advertiser_->advertise_service(descriptor.service_name, descriptor.service_type,
                                           "local", this->port_);
    }

    // Build the log message so that users know what is running and where.
    std::string services_text{"{"};
    bool needs_comma{false};
    for (const auto& descriptor : services_) {
      if (needs_comma) {
        services_text.append(",");
      }
      services_text.append(descriptor.service_name);
      needs_comma = true;
    }
    services_text.append("}");

    LOG(INFO) << "Services " << services_text << " running on port " << port_;
  }

  /**
   * Block until the server finishes.
   */
  void wait() {
    DLOG(INFO) << "ServiceRunner::wait() -- calling Wait().";
    server_->Wait();
    DLOG(INFO) << "ServiceRunner::wait() -- exiting.";
  }

  /**
   * Force the server to stop NOW.
   *
   * Prefer the stop() method when possible.
   */
  void force_stop() {
    // Only do the shutdown procedure if the server is running.
    if (server_) {
      server_->Shutdown();
      reset();
    }
  }

  /**
   * Gracefully shutdown the server.
   *
   * This finishes current requests and blocks new requests from being processed.
   *
   * If the server does not shutdown in the required timeout time, it is forcefully shutdown.
   */
  template <typename Rep, typename Period, typename Clock = std::chrono::system_clock>
  void stop(std::chrono::duration<Rep, Period> timeout = std::chrono::milliseconds(100)) {
    // Only do the shutdown procedure if the server is running.
    if (server_) {
      server_->Shutdown(Clock::now() + timeout);
      reset();
    }
  }
};

/**
 * Utility to run a single service.
 *
 * Note that this method blocks indefinitely.
 *
 * The resulting server can only be shutdown by signals or stopping the process.
 */
template <typename ServiceT, typename ServiceImplT>
void run_single_service(const std::string& service_name) {
  ServiceRunner runner{};
  ServiceImplT service{};
  runner.add_service<ServiceT, ServiceImplT>(service_name, service);
  runner.start();
  runner.wait();
}

}  // namespace tvsc::service::utility
