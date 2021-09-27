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
#include "miniRouter.hpp"

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

        void go();
    private:
        int startup(u_short * pport); //对server建立一个监听的socket

        int       server_sock{-1};
        int       client_sock{-1};
        u_short   port{4000};

        struct    sockaddr_in client_name;
        socklen_t client_name_len{sizeof(client_name)};

        std::string doc_root;

        // 创建一个request_handle
        http::request_handler request_handler;

        //创建一个线程池
        THREAD_POOL::threadpool th_pool{4};

    public:
        http::miniRouter      router;

};

