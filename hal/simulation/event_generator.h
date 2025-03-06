#pragma once

namespace tvsc::hal::simulation {

template <typename ClockType>
class EventGenerator {
 public:
  virtual ~EventGenerator() = default;

  virtual ClockType::duration next_event_in(ClockType::time_point now) const noexcept = 0;

  virtual void generate(ClockType::time_point now) noexcept = 0;
};

}  // namespace tvsc::hal::simulation
