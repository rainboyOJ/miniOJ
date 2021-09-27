#include "server.hpp"
#include "judge/judge.hpp"
#include "judge/exec_judge.hpp"

int main(int argc,char * argv[]){
    HttpServer http(8080);
    
    //注册路由
    http.router.reg<http::POST>("/judge",
            [](http::request& a,http::reply& b ){
                //b.set_content("hello world ,this by miniRouter!");
                b.set_content(a.content);
            }
    );
    //http::miniRouter::routerType xx =
            //[](http::request& a,http::reply& b ){
            //};


    http.go();
    return 0;
}
