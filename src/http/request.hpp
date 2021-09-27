#pragma once
#include <string>
#include <vector>
#include <tuple>
#include <sstream>
#include <fstream>

#include "header.hpp"
#include "reply.hpp"

namespace http {

/// A request received from a client.
struct request
{
  std::string method;
  std::string uri;
  std::string remote_ip;

  int http_version_major;
  int http_version_minor;

  std::vector<header> headers;

  bool has_content{false};
  std::string content{};
  std::size_t content_size{0};
};


///// ========================== request_handler type
/// The common handler for all incoming requests.
class request_handler
{
public:
  request_handler(const request_handler&)            = delete;
  request_handler& operator=(const request_handler&) = delete;

  /// Construct with a directory containing files to be served.
  explicit request_handler(const std::string& doc_root);

  /// Handle a request and produce a reply.
  void handle_request(const request& req, reply& rep);

private:
  /// The directory containing the files to be served.
  std::string doc_root_;

  /// Perform URL-decoding on a string. Returns false if the encoding was invalid.
  static bool url_decode(const std::string& in, std::string& out);
};
///// ========================== request_handler type end

///// ========================== request_parser type
/// Parser for incoming requests.
class request_parser
{
public:
  /// Construct ready to parse the request method.
  request_parser() : state_(method_start) {}

  /// Reset to initial parser state.
  void reset(){ state_ = method_start; };

  /// Result of parse.
  enum result_type { good, bad, indeterminate };

  /// Parse some data. The enum return value is good when a complete request has
  /// been parsed, bad if the data is invalid, indeterminate when more data is
  /// required. The InputIterator return value indicates how much of the input
  /// has been consumed.
  template <typename InputIterator>
  std::tuple<result_type, InputIterator> parse(request& req,
      InputIterator begin, InputIterator end)
  {
    while (begin != end)
    {
      result_type result = consume(req, *begin++);
      if (result == good || result == bad)
        return std::make_tuple(result, begin);
    }
    return std::make_tuple(indeterminate, begin);
  }

private:
  /// Handle the next character of input.
  result_type consume(request& req, char input);//太大了放在 connection.cpp 里

  /// Check if a byte is an HTTP character.
  static bool is_char(int c) { return c >= 0 && c <= 127; }

  /// Check if a byte is an HTTP control character.
  static bool is_ctl(int c) { return (c >= 0 && c <= 31) || (c == 127); }

  /// Check if a byte is defined as an HTTP tspecial character.
  static bool is_tspecial(int c) {
      switch (c) {
          case '(': case ')': case '<': case '>': case '@':
          case ',': case ';': case ':': case '\\': case '"':
          case '/': case '[': case ']': case '?': case '=':
          case '{': case '}': case ' ': case '\t':
              return true;
          default:
              return false;
      }
  }

  /// Check if a byte is a digit.
  static bool is_digit(int c) { return c >= '0' && c <= '9'; }

  /// The current state of the parser.
  enum state
  {
    method_start,
    method,
    uri,
    http_version_h,
    http_version_t_1,
    http_version_t_2,
    http_version_p,
    http_version_slash,
    http_version_major_start,
    http_version_major,
    http_version_minor_start,
    http_version_minor,
    expecting_newline_1,
    header_line_start,
    header_lws,
    header_name,
    space_before_header_value,
    header_value,
    expecting_newline_2,
    expecting_newline_3,
    content
  } state_;
};
///// ========================== request_parser type end

} //end namespace http
