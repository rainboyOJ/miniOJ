#pragma once

#include <functional>
#include <string_view>
#include <variant>
#include <regex>
#include <type_traits>

#include "http/request.hpp"
#include "http/reply.hpp"
#include "utils.hpp"

namespace http {


	enum class http_method {
        UNKNOW,
		DEL,
		GET,
		HEAD,
		POST, PUT,
		CONNECT,
		OPTIONS,
		TRACE
	};
	constexpr inline const auto GET     = http_method::GET;
	constexpr inline const auto POST    = http_method::POST;
	constexpr inline const auto DEL     = http_method::DEL;
	constexpr inline const auto HEAD    = http_method::HEAD;
	constexpr inline const auto PUT     = http_method::PUT;
	constexpr inline const auto CONNECT = http_method::CONNECT;
	constexpr inline const auto TRACE   = http_method::TRACE;
	constexpr inline const auto OPTIONS = http_method::OPTIONS;

	inline constexpr std::string_view method_name(http_method mthd) {
        using namespace std::literals;
		switch (mthd)
		{
		case http_method::DEL:
			return "DELETE"sv;
			break;
		case http_method::GET:
			return "GET"sv;
			break;
		case http_method::HEAD:
			return "HEAD"sv;
			break;
		case http_method::POST:
			return "POST"sv;
			break;
		case http_method::PUT:
			return "PUT"sv;
			break;
		case http_method::CONNECT:
			return "CONNECT"sv;
			break;
		case http_method::OPTIONS:
			return "OPTIONS"sv;
			break;
		case http_method::TRACE:
			return "TRACE"sv;
			break;
		default:
			return "UNKONWN"sv;
			break;
		}
	}

template<typename HttpServerType>
class miniRouter {
public:
    explicit miniRouter(HttpServerType& server)
        :server{server}
    {}

public:
    using routerType = std::function<void(request&,reply&)>;
    using uriType    = std::variant<std::string,std::regex>;


    template<http_method method = GET,typename Function>
    void reg(uriType uri,Function&& __f){
        if constexpr ( method != POST && method != GET){
            throw std::invalid_argument("现在只支持 GET 与 POST");
        }
        routers.push_back( {method_name(method), std::move(uri),std::forward<Function>(__f)} );
    }

    void default_router(request& req,reply& rep) const{
        //TODO 查找相应的文件
        //1.判断是否有后缀
        std::string_view uri_view = req.uri;
        if( uri_view[0] == '/') uri_view.remove_prefix(1); // remove slash

        fs::path file_path = fs::path(server.doc_root) / uri_view;
        if(!file_path.has_extension())
            file_path  = file_path.string() + ".html";

        if( fs::exists(file_path))
            rep.set_content_by_path(file_path);
        else
            rep = reply::stock_reply(reply::status_type::not_found);
    }

    
    void operator()(request& req,reply& rep) const{
        for (const auto& e : routers) {
            if( e.method == req.method ){
                if( std::holds_alternative<std::string>(e.uri) ){
                    if(std::get<std::string>(e.uri) == req.uri) {
                        e.route(req,rep);
                        return;
                    }
                }
                else { //regx
                    auto reg = std::get<std::regex>(e.uri);
                    if( std::regex_match(req.uri,req.sm,reg) ) //匹配成功
                    {
                        e.route(req,rep);
                        return;
                    }
                }
            }
        }
        default_router(req, rep);
    }   
    
    struct node {
        const std::string_view method;
        //const std::string      uri;
        uriType uri;
        routerType  route;
    };

private:
    std::vector<node> routers;
    const HttpServerType& server; //拥有这个router的服务的引用

};


} //namespace http end
