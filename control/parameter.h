#pragma once

#include <memory>
#include <initializer_list>
#include <vector>

namespace tvsc::control {

  template <typename T>
  class Parameter {
  public:
    virtual ~Parameter() = default;
    virtual bool is_allowed(const T& value) const  = 0;
  };
  

  namespace impl {

    template <typename T>
    class CombinedParameters final : public Parameter<T> {
    private:
      std::vector<std::unique_ptr<Parameter<T>>> parameters_{};
    public:
      CombinedParameters(std::unique_ptr<Parameter<T>> first, std::unique_ptr<Parameter<T>> second, std::initializer_list<std::unique_ptr<Parameter<T>>> remaining) : parameters_(std::move(first)) {
	parameters_.emplace_back(std::move(second));
	parameters_.insert(parameters_.end(), remaining.begin(), remaining.end());
      }


      bool is_allowed(const T& value) override {
	for (const auto& p : parameters_) {
	  if (p->is_allowed(value)) {
	    return true;
	  }
	}
	return false;
      }
      
    };
    
  }

  
  template <typename T>
  std::unique_ptr<Parameter<T>> combine(std::unique_ptr<Parameter<T>> first, std::unique_ptr<Parameter<T>> second, std::initializer_list<std::unique_ptr<Parameter<T>>> remaining) {
    return new impl::CombinedParameters<T>(std::move(first), std::move(second), std::move(remaining));
  }

  template <typename T>
  std::unique_ptr<T> exclude(const Parameter<T>& parameter, const Parameter<T>& excluded);

}
