// Author:Rainboy
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>
#include <strings.h>
#include <string.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdint.h>


#include "mythreadpool.hpp"
#include "connection.hpp"

class HttpServer {
    public:
        explicit HttpServer(u_short port,std::string doc_root = ".",unsigned int pool_size = 4)
            :port{port},doc_root{doc_root},
            request_handler{doc_root},th_pool{pool_size}
        {}

        HttpServer(const HttpServer & ) = delete;
        HttpServer& operator=(const HttpServer & ) =delete;
        ~HttpServer(){
            std::cout << "Bye bye." << std::endl;
            close(server_sock);
        }

        void go(){
            server_sock = startup(&port);
            std::cout << "httpSever 运行在端口: " <<  port <<std::endl;

            while (1)
            {
                client_sock = accept(server_sock,
                        (struct sockaddr *)&client_name,
                        &client_name_len); //从队列中取一条
                if (client_sock == -1)
                    throw std::runtime_error("accept failed, LINE: " + std::to_string(__LINE__));
                    
                {
                    auto conn = std::make_shared<http::connection>(client_sock,request_handler);
                    th_pool.commit([conn](){ conn->start(); });
                }
            }

        }
    private:
        int startup(u_short * pport){
            int httpd = 0;
            int on    = 1;
            struct sockaddr_in name;

            httpd = socket(AF_INET, SOCK_STREAM, 0); // 创建一个socket
            if (httpd == -1)
                throw std::runtime_error("创建httpSever失败, LINE: " + std::to_string(__LINE__));

            memset(&name, 0, sizeof(name));     //清空地址结构体
            name.sin_family      = AF_INET;
            name.sin_port        = htons(*pport); // host to network short
            name.sin_addr.s_addr = htonl(INADDR_ANY); //任意地址
            if ((setsockopt(httpd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))) < 0)  //重用地址
            {  
                throw std::runtime_error("setsockopt failed, LINE: " + std::to_string(__LINE__));
            }
            if (bind(httpd, (struct sockaddr *)&name, sizeof(name)) < 0) //绑定
                throw std::runtime_error("bind failed, LINE: " + std::to_string(__LINE__));
            if (*pport == 0)  /* if dynamically allocating a port */
            {
                socklen_t namelen = sizeof(name);
                if (getsockname(httpd, (struct sockaddr *)&name, &namelen) == -1)
                throw std::runtime_error("getsockname failed, LINE: " + std::to_string(__LINE__));
                *pport = ntohs(name.sin_port);
            }
            if (listen(httpd, 5) < 0) //注意这个5，再看一遍视频
                throw std::runtime_error("listen failed, LINE: " + std::to_string(__LINE__));
            return(httpd); //返回创建的socketr 
        }
        int server_sock{-1};
        int client_sock{-1};
        u_short port{4000};
        struct sockaddr_in client_name;
        socklen_t  client_name_len{sizeof(client_name)};

        std::string doc_root;

        // 创建一个request_handle
        http::request_handler request_handler;

        //创建一个线程池
        THREAD_POOL::threadpool th_pool{4};

};

