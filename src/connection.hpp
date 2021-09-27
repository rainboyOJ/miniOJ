// Author:Rainboy
// 连接
#pragma once
#ifndef __HTTP_CONNECTION_HPP
#define __HTTP_CONNECTION_HPP

#include <string>
#include <memory>
#include <vector>
#include <array>
#include <cstring>

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <iostream>

#include "http/request.hpp"
#include "miniRouter.hpp"


namespace http {


class connection : public std::enable_shared_from_this<connection>
{
public:
    using connection_ptr = std::shared_ptr<connection>;
    using socket_type    = int;
public:
  explicit connection(socket_type socket, request_handler& handler,miniRouter& router)
    :client_socket{socket},request_handler_{handler},
      router{router}
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
              router(request_,reply_);
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

  void get_client_ip(){
      struct sockaddr_in peerAddr;
      socklen_t peerLen;
      getpeername(client_socket, (struct sockaddr *)&peerAddr, &peerLen); //获取connfd表示的连接上的对端地址
      inet_ntop(AF_INET, &peerAddr.sin_addr, client_ip, sizeof(client_ip));
      //ntohs(peerAddr.sin_port));
  }



  request_handler& request_handler_; /// The handler used to process the incoming request.
  miniRouter& router;
  std::array<char, 8192> buffer_; /// Buffer for incoming data.
  request request_; /// The incoming request.
  request_parser request_parser_{}; /// The parser for the incoming request.
  reply reply_; /// The reply to be sent back to the client.
  int client_socket{-1}; /// raw socket
  char client_ip[INET_ADDRSTRLEN];//保存点分十进制的地址


};

} //end namespace http

#endif
