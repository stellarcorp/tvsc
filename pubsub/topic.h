#pragma once

#include <string>
#include <string_view>

namespace tvsc::pubsub {

/**
 * Should a particular MessageT type get compressed when sending over a socket?
 *
 * This implementation provides a default. Add other implementations as desired to enable/disable
 * compression for each type. Note that submessages within this type are compressed according
 * to this setting when they are sent as part of that top-level type.
 */
template <typename MessageT>
constexpr bool should_compress() {
  return false;
}

/**
 * Generic publisher of a single topic.
 */
template <typename MessageT>
class Topic {
 protected:
  const std::string topic_name_;

 public:
  Topic(std::string_view topic_name) : topic_name_(topic_name) {}
  virtual ~Topic() = default;

  virtual void publish(const MessageT& msg) = 0;
};

}  // namespace tvsc::pubsub
