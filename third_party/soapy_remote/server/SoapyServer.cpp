// Copyright (c) 2015-2018 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "SoapyServer.hpp"

#include <csignal>
#include <cstddef>
#include <cstdlib>

#include "SoapyInfoUtils.hpp"
#include "SoapyMDNSEndpoint.hpp"
#include "SoapyRPCSocket.hpp"
#include "SoapyRemoteDefs.hpp"
#include "SoapySDR/Logger.hpp"
#include "SoapySDR/Modules.hpp"
#include "SoapySDR/Registry.hpp"
#include "SoapySSDPEndpoint.hpp"
#include "SoapyURLUtils.hpp"
#include "gflags/gflags.h"
#include "glog/logging.h"
#include "soapy_server.h"

DEFINE_string(soapy_url, "0.0.0.0:55132",
              "URL to bind the SoapyRemote service to. Can be an IPv6 or IPv4 address, as supported by network.");

/***********************************************************************
 * Signal handler for Ctrl + C
 **********************************************************************/
static sig_atomic_t serverDone = false;
void sigIntHandler(const int) {
  LOG(INFO) << "Caught Ctrl+C, shutting down the server...";
  serverDone = true;
}

bool is_ipv6_supported() {
  static bool isIPv6Supported{not SoapyRPCSocket(SoapyURL("tcp", "::", "0").toString()).null()};
  return isIPv6Supported;
}

void log_to_glog(const SoapySDRLogLevel logLevel, const char* message) {
  switch (logLevel) {
    case SOAPY_SDR_FATAL:
      LOG(FATAL) << message;
      break;
    case SOAPY_SDR_CRITICAL:
    case SOAPY_SDR_ERROR:
      LOG(ERROR) << message;
      break;
    case SOAPY_SDR_WARNING:
      LOG(WARNING) << message;
      break;
    case SOAPY_SDR_NOTICE:
    case SOAPY_SDR_INFO:
      LOG(INFO) << message;
      break;
    case SOAPY_SDR_DEBUG:
    case SOAPY_SDR_TRACE:
    case SOAPY_SDR_SSI:
      LOG(INFO) << message;
      break;
  }
}

int run_soapy_server(void) {
  SoapySDR::registerLogHandler(log_to_glog);
  SoapySDR::loadModules();

  SoapySocketSession sess;

  auto url = SoapyURL(FLAGS_soapy_url);

  // this UUID identifies the server process
  const auto serverUUID = SoapyInfo::generateUUID1();
  LOG(INFO) << "Server version: " << SoapyInfo::getServerVersion();
  LOG(INFO) << "Server UUID: " << serverUUID;

  LOG(INFO) << "Launching the server... " << url.toString();
  SoapyRPCSocket s;
  if (s.bind(url.toString()) != 0) {
    LOG(FATAL) << "Server socket bind FAIL: " << s.lastErrorMsg();
    return EXIT_FAILURE;
  }
  LOG(INFO) << "Server bound to " << s.getsockname();
  s.listen(SOAPY_REMOTE_LISTEN_BACKLOG);
  auto serverListener = new SoapyServerListener(s, serverUUID);

  const int ipVerServices = is_ipv6_supported() ? SOAPY_REMOTE_IPVER_UNSPEC : SOAPY_REMOTE_IPVER_INET;

  LOG(INFO) << "Launching discovery server... ";
  auto ssdpEndpoint = new SoapySSDPEndpoint();
  ssdpEndpoint->registerService(serverUUID, url.getService(), ipVerServices);

  LOG(INFO) << "Connecting to DNS-SD daemon... ";
  auto dnssdPublish = new SoapyMDNSEndpoint();
  dnssdPublish->printInfo();
  dnssdPublish->registerService(serverUUID, url.getService(), ipVerServices);

  const auto modules{SoapySDR::listModules()};
  if (modules.empty()) {
    LOG(INFO) << "<No Soapy modules found>\n";
  } else {
    LOG(INFO) << "Loaded modules:\n";
    for (const auto& module : modules) {
      LOG(INFO) << "\t" << module << "\n";
    }
  }

  for (const auto& entry : SoapySDR::Registry::listFindFunctions()) {
    LOG(INFO) << "Find function -- entry.first: " << entry.first;
  }

  const auto find_functions{SoapySDR::Registry::listFindFunctions()};
  if (find_functions.empty()) {
    LOG(ERROR) << "<No find functions available>";
  } else {
    for (const auto& entry : find_functions) {
      LOG(INFO) << "find_function entry: " << entry.first;
    }
  }

  LOG(INFO) << "Press Ctrl+C to stop the server";
  signal(SIGINT, sigIntHandler);
  bool exitFailure = false;
  while (not serverDone and not exitFailure) {
    serverListener->handleOnce();
    if (not s.status()) {
      LOG(ERROR) << "Server socket failure: " << s.lastErrorMsg();
      exitFailure = true;
    }
    if (not dnssdPublish->status()) {
      LOG(ERROR) << "DNS-SD daemon disconnected...";
      exitFailure = true;
    }
  }
  if (exitFailure) {
    LOG(ERROR) << "Exiting prematurely...";
  }

  delete ssdpEndpoint;
  delete dnssdPublish;

  LOG(INFO) << "Shutdown client handler threads";
  delete serverListener;
  s.close();

  SoapySDR::unloadModules();

  LOG(INFO) << "Cleanup complete, exiting";
  return exitFailure ? EXIT_FAILURE : EXIT_SUCCESS;
}
