#pragma once

namespace tvsc::control {

  template <typename T>
  class Parameter {
  public:
    virtual bool is_allowed(const T& value) const  = 0;
  };
  

}
