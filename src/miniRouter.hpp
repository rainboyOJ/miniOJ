#pragma once

#include <functional>
#include <string_view>
#include <type_traits>

#include "http/request.hpp"
#include "http/reply.hpp"

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

class miniRouter {
public:
public:
    using routerType = std::function<void(request&,reply&)>;


    template<http_method method = GET,typename Function>
    void reg(std::string url,Function&& __f){
        if constexpr ( method != POST && method != GET){
            throw std::invalid_argument("现在只支持 GET 与 POST");
        }
        routers.push_back( {method_name(method), url,std::forward<Function>(__f)} );
    }

    void default_router(request&,reply&);
    void operator()(request&,reply&);
    
    struct node {
        const std::string_view method;
        const std::string      uri;
        routerType  route;
    };

private:
    std::vector<node> routers;

};


} //namespace http end
