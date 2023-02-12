#pragma once

#include <filesystem>
#include <string>
#include <string_view>

#include "App.h"
#include "buffer/buffer.h"
#include "gflags/gflags.h"
#include "glog/logging.h"
#include "io/file_reader.h"

DECLARE_string(doc_root);

namespace tvsc::http {

inline std::filesystem::path compute_filename(const std::string& base_path, std::string_view url) {
  std::filesystem::path path{base_path};
  std::filesystem::path url_path{url};
  if (url_path.is_absolute()) {
    url_path = url_path.relative_path();
  }
  path.append(url_path.native());
  return path;
}

template <bool SSL>
void stream_static_file(const std::filesystem::path& filename, uWS::HttpResponse<SSL>* response,
                        uWS::HttpRequest* request) {
  constexpr size_t BUFFER_SIZE{1024};
  constexpr char HTTP_404[]{"404 Not Found"};

  io::FileReader<char> file_reader{filename};
  if (file_reader()) {
    response->writeStatus(uWS::HTTP_200_OK);
    buffer::Buffer<char, BUFFER_SIZE> buffer{};
    while (!file_reader.eof()) {
      size_t bytes_read = file_reader.read(buffer);
      response->write(std::string_view{buffer.data(), bytes_read});
    }
  } else {
    response->writeStatus(HTTP_404);
    response->write(file_reader.error_message());
  }
  response->end();
}

template <bool SSL>
void stream_file(uWS::HttpResponse<SSL>* response, uWS::HttpRequest* request) {
  LOG(INFO) << "Serving file. url: " << request->getUrl();
  stream_static_file(compute_filename(FLAGS_doc_root, request->getUrl()), response, request);
}

template <bool SSL>
void serve_static_files(const std::string& pattern, uWS::TemplatedApp<SSL>& app) {
  app.get(pattern, stream_file<SSL>);
}

template <bool SSL>
void serve_homepage(std::string_view homepage_filename, uWS::TemplatedApp<SSL>& app) {
  const std::filesystem::path path{compute_filename(FLAGS_doc_root, homepage_filename)};
  app.get("/", [path](uWS::HttpResponse<SSL>* response, uWS::HttpRequest* request) {
    LOG(INFO) << "Serving homepage. path: " << path.native();
    stream_static_file<SSL>(path, response, request);
  });
}

template <bool SSL>
void serve_favicon(std::string_view favicon_filename, uWS::TemplatedApp<SSL>& app) {
  const std::filesystem::path path{compute_filename(FLAGS_doc_root, favicon_filename)};
  app.get("/favicon.ico", [path](uWS::HttpResponse<SSL>* response, uWS::HttpRequest* request) {
    LOG(INFO) << "Serving favicon. path: " << path.native();
    stream_static_file<SSL>(path, response, request);
  });
}

}  // namespace tvsc::http
