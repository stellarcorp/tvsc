#pragma once

namespace tvsc::discovery {

  // TODO(james): Consider implementing as a protobuf message in order to serialize, use from other languages, etc.
  class ServiceDescriptor final {

  public:
    std::string canonical_service_name{};
    std::string published_service_name{};

    
    
  };

}
