#pragma once

#include <limits>
#include <vector>

#include "glog/logging.h"
#include "grpcpp/support/status.h"
#include "grpcpp/support/sync_stream.h"

namespace tvsc::services {

/**
 * Testing version of a client reader for creating client-side stubs for tests.
 *
 * This class is based on the ideas in
 * https://github.com/grpc/grpc/blob/master/test/cpp/end2end/mock_test.cc and
 * https://github.com/grpc/grpc/blob/master/include/grpcpp/test/mock_stream.h These files contain
 * tests for gRPC using gmock.
 *
 * For server response streaming, override the <RpcMethodName>Raw() function in the stub
 * (mocked or otherwise) to return a pointer to an instance of this reader.
 */
// TODO(james): Design concepts (possibly augmenting this class) to handle client-side streaming and
// bidirectional streaming. Currently, this class is only configured for server-side streaming.
template <typename ResponseT>
class TestReader final : public grpc::ClientReaderInterface<ResponseT> {
 private:
  std::vector<ResponseT> responses_{};

  ResponseT consume_message() {
    ResponseT response = responses_.front();
    responses_.erase(responses_.begin());
    return response;
  }

  const ResponseT& peek() const { return responses_.front(); }

  bool has_more_messages() const { return !responses_.empty(); }

 public:
  TestReader() { LOG(INFO) << "TestReader::TestReader()"; }

  void add_response(const ResponseT& response) { responses_.emplace_back(response); }

  bool NextMessageSize(uint32_t* size) override {
    if (has_more_messages()) {
      *size = peek().ByteSizeLong();
      return true;
    } else {
      *size = std::numeric_limits<uint32_t>::max();
      return false;
    }
  }

  void WaitForInitialMetadata() override {
    // Do nothing.
  }

  bool Read(ResponseT* response) override {
    LOG(INFO) << "TestReader::Read()";
    if (has_more_messages()) {
      *response = consume_message();
      return true;
    } else {
      return false;
    }
  }

  grpc::Status Finish() override {
    LOG(INFO) << "TestReader::Finish()";
    return grpc::Status::OK;
  }
};

}  // namespace tvsc::services
