#pragma once
#include <string>
#include <vector>
#include <type_traits>
#include <filesystem>
#include <fstream>

#include "header.hpp"
#include "mime_types.hpp"

namespace http {

/// A reply to be sent to a client.
struct reply
{
    using const_buffer = const char *;
  /// The status of the reply.
  enum status_type
  {
    unset                    = 0,
    ok                    = 200,
    created               = 201,
    accepted              = 202,
    no_content            = 204,
    multiple_choices      = 300,
    moved_permanently     = 301,
    moved_temporarily     = 302,
    not_modified          = 304,
    bad_request           = 400,
    unauthorized          = 401,
    forbidden             = 403,
    not_found             = 404,
    internal_server_error = 500,
    not_implemented       = 501,
    bad_gateway           = 502,
    service_unavailable   = 503
  } status{unset};

  /// The headers to be included in the reply.
  std::vector<header> headers;

  /// The content to be sent in the reply.
  std::string content;

  /// Convert the reply into a vector of buffers. The buffers do not own the
  /// underlying memory blocks, therefore the reply object must remain valid and
  /// not be changed until the write operation has completed.
  std::vector<const_buffer> to_buffers(); //TODO

  /// Get a stock reply.
  static reply stock_reply(status_type status);

  //template<typename T,
      //std::enable_if_t<
          //std::is_same_v< std::remove_reference_t<std::remove_cv_t<T>> , std::string> , bool> = true
      //>
  void set_content(std::string_view _content){

        status  = ok;
        content = _content;
        headers.resize(2);
        headers[0].name = "Content-Length";
        headers[0].value = std::to_string(content.size());
        headers[1].name = "Content-Type";
        headers[1].value = "text/html;charset=utf-8";
  }

  void set_content_json(std::string_view _content){
      set_content(_content);
      headers[1].value= "application/json";
  }

  void set_content_by_path(const std::filesystem::path& path){
        status = reply::ok;
        char buf[512];

        std::ifstream is(path.c_str(), std::ios::in | std::ios::binary);
        while (is.read(buf, sizeof(buf)).gcount() > 0)
            content.append(buf, is.gcount());
        headers.resize(2);
        headers[0].name = "Content-Length";
        headers[0].value = std::to_string(content.size());
        headers[1].name = "Content-Type";
        headers[1].value = mime_types::extension_to_type(path.extension().c_str());
  }

};
} //end namespace http

