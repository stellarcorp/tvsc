#include <string>

#include "App.h"
#include "gtest/gtest.h"

TEST(MoveOnlyFunctionTest, CanUseLambda) {
  uWS::App app{};
  app.get("/*", [](auto *response, auto *request) { response->end("Hello world!"); });
}

void simple_function(uWS::HttpResponse<false> *response, uWS::HttpRequest *request) { response->end("Hello world!"); }
TEST(MoveOnlyFunctionTest, CanUseFunction) {
  uWS::App app{};
  app.get("/*", &simple_function);
}

template <bool SSL>
void ssl_templated_function(uWS::HttpResponse<SSL> *response, uWS::HttpRequest *request) {
  response->end("Hello world!");
}
TEST(MoveOnlyFunctionTest, CanUseSslTemplatedFunction) {
  uWS::App app{};
  app.get("/*", &ssl_templated_function<false>);
}

TEST(MoveOnlyFunctionTest, CanUseSslTemplatedFunctionWithSsl) {
  uWS::SSLApp app{};
  app.get("/*", &ssl_templated_function<true>);
}

class Callable final {
 public:
  void operator()(uWS::HttpResponse<false> *response, uWS::HttpRequest *request) { response->end("Hello, world!"); }
};

TEST(MoveOnlyFunctionTest, CanUseCallableObject) {
  uWS::App app{};
  Callable callable{};
  app.get("/*", callable);
}

template <bool SSL>
class TemplatedCallable final {
 public:
  void operator()(uWS::HttpResponse<SSL> *response, uWS::HttpRequest *request) { response->end("Hello, world!"); }
};

TEST(MoveOnlyFunctionTest, CanUseTemplatedCallableObject) {
  uWS::App app{};
  TemplatedCallable<false> callable{};
  app.get("/*", callable);
}

TEST(MoveOnlyFunctionTest, CanUseTemplatedCallableObjectWithSsl) {
  uWS::SSLApp app{};
  TemplatedCallable<true> callable{};
  app.get("/*", callable);
}

class CallableWithTemplatedOp final {
 public:
  template <bool SSL>
  void operator()(uWS::HttpResponse<SSL> *response, uWS::HttpRequest *request) {
    response->end("Hello, world!");
  }
};

TEST(MoveOnlyFunctionTest, CanUseCallableObjectWithTemplatedOp) {
  uWS::App app{};
  CallableWithTemplatedOp callable{};
  app.get("/*", callable);
}

TEST(MoveOnlyFunctionTest, CanUseCallableWithTemplatedOpObjectWithSsl) {
  uWS::SSLApp app{};
  CallableWithTemplatedOp callable{};
  app.get("/*", callable);
}

class PatternIncludedCallable final {
 private:
  const std::string pattern_{"/service/hello"};

 public:
  const std::string &pattern() const { return pattern_; }

  template <bool SSL>
  void operator()(uWS::HttpResponse<SSL> *response, uWS::HttpRequest *request) {
    response->end("Hello, world!");
  }
};

TEST(MoveOnlyFunctionTest, CanUsePatternIncludedCallable) {
  uWS::App app{};
  PatternIncludedCallable callable{};
  app.get(callable.pattern(), callable);
}

TEST(MoveOnlyFunctionTest, CanUsePatternIncludedCallableWithSsl) {
  uWS::SSLApp app{};
  PatternIncludedCallable callable{};
  app.get(callable.pattern(), callable);
}
