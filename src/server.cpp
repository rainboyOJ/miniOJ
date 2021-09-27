#include "server.hpp"

void HttpServer::go() {
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
            auto conn = std::make_shared<http::connection>(client_sock,request_handler,router);
            th_pool.commit([conn](){ conn->start(); });
        }
    }

}

int HttpServer::startup(u_short * pport){
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
