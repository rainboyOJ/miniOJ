// Author:Rainboy
// 连接
#pragma once
#ifndef __HTTP_CONNECTION_HPP
#define __HTTP_CONNECTION_HPP

#include <string>
#include <memory>
#include <vector>
#include <array>
#include <sstream>
#include <fstream>
#include <cstring>

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <iostream>

#define CONTENT_LENGTH "Content-Length"

namespace http {


///// ========================== mime_types 
    struct mime_types {
        using mapping =  struct { const char* extension; const char* mime_type; };
        static mapping mappings[]; 

        static std::string extension_to_type(const std::string& extension)
        {
            for (mapping* m = mappings; m->extension; ++m)
            {
                if (m->extension == extension)
                {
                    return m->mime_type;
                }
            }
            return "text/plain";
        }
    }; // struct mime_types end

///// ========================== mime_types end


///// ========================== header
struct header
{
  std::string name;
  std::string value;
};
///// ========================== header end



///// ========================== reply type
/// A reply to be sent to a client.
struct reply
{
    using const_buffer = const char *;
  /// The status of the reply.
  enum status_type
  {
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
  } status;

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
};
///// ========================== reply type end


///// ========================== request type
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
///// ========================== request type end


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



class connection : public std::enable_shared_from_this<connection>
{
public:
    using connection_ptr = std::shared_ptr<connection>;
    using socket_type    = int;
public:
  explicit connection(socket_type socket, request_handler& handler)
    :client_socket{socket},request_handler_{handler}
  {}

  socket_type& socket(){ return client_socket;};
  ~connection(){
      if( client_socket !=-1) close(client_socket);
  }

  inline void start(){
      get_client_ip();
      std::cout << "client_ip: "<< client_ip <<std::endl;
      handle_read();
  };

private:
  /// Handle completion of a read operation.
  void handle_read(){
      while ( 1 ) {
          int read_size = read_some();
          if(read_size <=0 ) break; // TODO 处理错误
          request_parser::result_type result;

          std::tie(result,std::ignore) = 
              request_parser_.parse(request_,buffer_.data(),buffer_.data()+read_size);


          if (result == request_parser::good)
          {
              request_handler_.handle_request(request_, reply_); //根据请求 得到replay
              //得到ip
              //request_.remote_ip = this->socket().remote_endpoint().address().to_string();
              //log("request_.remote_ip ",request_.remote_ip );

              //写入
              handle_write(); //TODO
              return;
          }
          else if (result == request_parser::bad)
          {
              reply_ = reply::stock_reply(reply::bad_request);
              handle_write(); //TODO
              return;
          }
          //继续读取
      }

  };

  inline int read_some(){
      return recv(client_socket, buffer_.data(), buffer_.size(), 0);
  }

  /// Handle completion of a write operation.
  void handle_write(){
      //Writen(client_socket, reply_.to_buffers(), );
      for (const auto& e : reply_.to_buffers()) {
          Writen(client_socket, e, strlen(e));
      }
  };

  bool Writen(const int sockfd,const char *buffer,const size_t n)
  {
      int nLeft,idx,nwritten;
      nLeft = n;  
      idx = 0;
      while(nLeft > 0 )
      {    
          if ( (nwritten = send(sockfd, buffer + idx,nLeft,0)) <= 0) return false;      

          nLeft -= nwritten;
          idx += nwritten;
      }

      return true;
  }
  void get_client_ip();

  request_handler& request_handler_; /// The handler used to process the incoming request.
  std::array<char, 8192> buffer_; /// Buffer for incoming data.
  request request_; /// The incoming request.
  request_parser request_parser_{}; /// The parser for the incoming request.
  reply reply_; /// The reply to be sent back to the client.
  int client_socket{-1}; /// raw socket
  char client_ip[INET_ADDRSTRLEN];//保存点分十进制的地址

};

}; //end namespace http

#endif
