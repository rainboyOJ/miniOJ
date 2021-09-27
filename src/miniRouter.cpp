#include "miniRouter.hpp"

namespace http {


    //template<http_method method>
    //void miniRouter::reg(routerType& __f){
    //}
    void miniRouter::default_router(request& req,reply& rep){
        //TODO 查找相应的文件
        rep = reply::stock_reply(reply::status_type::not_found);
    }

    void miniRouter::operator()(request& req,reply& rep){
        for (const auto& e : routers) {
            if( e.method == req.method && req.uri == e.uri){
                e.route(req,rep);
                return;
            }
        }
        default_router(req, rep);

    }
} //namespace http end
