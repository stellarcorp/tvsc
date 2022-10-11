#include <string_view>

#include "App.h"
#include "gtest/gtest.h"

TEST(uWebSockets, CanCompile) {
  /* ws->getUserData returns one of these */
  struct PerSocketData {
    /* Fill with user data */
  };

  /* Keep in mind that uWS::SSLApp({options}) is the same as uWS::App() when compiled without SSL support.
   * You may swap to using uWS:App() if you don't need SSL */
  uWS::SSLApp({/* There are example certificates in uWebSockets.js repo */
               .key_file_name = "misc/key.pem",
               .cert_file_name = "misc/cert.pem",
               .passphrase = "1234"})
      .ws<PerSocketData>(
          "/*",
          {/* Settings */
           .compression = uWS::CompressOptions(uWS::DEDICATED_COMPRESSOR_4KB | uWS::DEDICATED_DECOMPRESSOR),
           .maxPayloadLength = 4 * 1024,
           .idleTimeout = 16,
           .maxBackpressure = 4 * 1024,
           .closeOnBackpressureLimit = true,
           .resetIdleTimeoutOnSend = false,
           .sendPingsAutomatically = true,
           /* Handlers */
           .upgrade = nullptr,
           .open =
               [](auto * /*ws*/) {
                 /* Open event here, you may access ws->getUserData() which points to a PerSocketData struct */
               },
           .message = [](auto *ws, std::string_view message, uWS::OpCode opCode) { ws->send(message, opCode, true); },
           .drain =
               [](auto * /*ws*/) {
                 /* Check ws->getBufferedAmount() here */
               },
           .ping =
               [](auto * /*ws*/, std::string_view) {
                 /* Not implemented yet */
               },
           .pong =
               [](auto * /*ws*/, std::string_view) {
                 /* Not implemented yet */
               },
           .close =
               [](auto * /*ws*/, int /*code*/, std::string_view /*message*/) {
                 /* You may access ws->getUserData() here */
               }});
}
