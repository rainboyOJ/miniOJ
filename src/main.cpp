#include "server.hpp"

int main(int argc,char * argv[]){
    HttpServer http(8080);
    
    //注册路由
    http.router.reg("/hello",
            [](http::request& a,http::reply& b ){
                b.set_content("hello world ,this by miniRouter!");
            }
    );
    //http::miniRouter::routerType xx =
            //[](http::request& a,http::reply& b ){
            //};


    http.go();
    return 0;
}
