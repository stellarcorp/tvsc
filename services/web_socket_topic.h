#pragma once

#include <memory>
#include <string>
#include <string_view>

#include "App.h"
#include "services/pub_sub_service.h"

namespace tvsc::services {

template <typename MessageT>
class WebSocketTopic final : public Topic<MessageT> {
 private:
  uWS::SSLApp* app_;

 protected:
  void publish_compressed(std::string_view msg) override {
    // Ignore the return value of this call. As far as I can tell, it will return true if it is
    // successful and if there exists a subscriber to the message. So, we can't differentiate
    // between a failure in the process where we would normally throw an exception and a lack of
    // interest where we might perform an optimization.
    app_->publish(this->topic_name_, msg, uWS::OpCode::BINARY, true);
  }

  void publish_uncompressed(std::string_view msg) override {
    app_->publish(this->topic_name_, msg, uWS::OpCode::BINARY, false);
  }

 public:
  WebSocketTopic(uWS::SSLApp& app, std::string_view topic_name)
      : Topic<MessageT>(topic_name), app_(&app) {}
};

template <typename MessageT>
std::unique_ptr<Topic<MessageT>> create_topic(uWS::SSLApp& app, std::string_view topic_name) {
  return std::make_unique<WebSocketTopic<MessageT>>(app, topic_name);
}

}  // namespace tvsc::services
