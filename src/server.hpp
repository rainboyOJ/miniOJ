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
        explicit HttpServer(u_short port,std::string doc_root = ".",
                unsigned int pool_size = std::max( std::thread::hardware_concurrency(), 1u) )
            :port{port},doc_root{doc_root},
            th_pool{pool_size},
            router{*this}
        {}

        HttpServer(const HttpServer & ) = delete;
        HttpServer& operator=(const HttpServer & ) =delete;
        
        ~HttpServer(){
            std::cout << "Bye bye." << std::endl;
            close(server_sock);
        }

        using routerType = http::miniRouter<HttpServer>;
        //typedef http::miniRouter<HttpServer> routerType;

        void go();
    public:
        fs::path doc_root;
    private:
        int startup(u_short * pport); //对server建立一个监听的socket

        int       server_sock{-1};
        int       client_sock{-1};
        u_short   port{4000};

        struct    sockaddr_in client_name;
        socklen_t client_name_len{sizeof(client_name)};


        // 创建一个request_handle
        http::request_handler request_handler;

        //创建一个线程池
        THREAD_POOL::threadpool th_pool{4};

    public:
        routerType router;

};

