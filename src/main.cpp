#include "server.hpp"

int main(int argc,char * argv[]){
    HttpServer http(8080);
    http.go();
    return 0;
}
