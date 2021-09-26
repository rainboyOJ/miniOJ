#include "connection.hpp"
namespace http {

    mime_types::mapping mime_types::mappings[] = {
        { "gif", "image/gif" },
        { "htm", "text/html" },
        { "html", "text/html" },
        { "jpg", "image/jpeg" },
        { "png", "image/png" },
        { 0, 0 } // Marks end of list.
    };


    namespace status_strings {

        const std::string ok                    = "HTTP/1.0 200 OK\r\n";
        const std::string created               = "HTTP/1.0 201 Created\r\n";
        const std::string accepted              = "HTTP/1.0 202 Accepted\r\n";
        const std::string no_content            = "HTTP/1.0 204 No Content\r\n";
        const std::string multiple_choices      = "HTTP/1.0 300 Multiple Choices\r\n";
        const std::string moved_permanently     = "HTTP/1.0 301 Moved Permanently\r\n";
        const std::string moved_temporarily     = "HTTP/1.0 302 Moved Temporarily\r\n";
        const std::string not_modified          = "HTTP/1.0 304 Not Modified\r\n";
        const std::string bad_request           = "HTTP/1.0 400 Bad Request\r\n";
        const std::string unauthorized          = "HTTP/1.0 401 Unauthorized\r\n";
        const std::string forbidden             = "HTTP/1.0 403 Forbidden\r\n";
        const std::string not_found             = "HTTP/1.0 404 Not Found\r\n";
        const std::string internal_server_error = "HTTP/1.0 500 Internal Server Error\r\n";
        const std::string not_implemented       = "HTTP/1.0 501 Not Implemented\r\n";
        const std::string bad_gateway           = "HTTP/1.0 502 Bad Gateway\r\n";
        const std::string service_unavailable   = "HTTP/1.0 503 Service Unavailable\r\n";

        reply::const_buffer to_buffer(const std::string & str){
            return str.data();
        }
        reply::const_buffer to_buffer(const char * str){
            return str;
        }
        


        reply::const_buffer to_buffer(reply::status_type status)
        {
            switch (status)
            {
                case reply::ok:
                    return to_buffer(ok);
                case reply::created:
                    return to_buffer(created);
                case reply::accepted:
                    return to_buffer(accepted);
                case reply::no_content:
                    return to_buffer(no_content);
                case reply::multiple_choices:
                    return to_buffer(multiple_choices);
                case reply::moved_permanently:
                    return to_buffer(moved_permanently);
                case reply::moved_temporarily:
                    return to_buffer(moved_temporarily);
                case reply::not_modified:
                    return to_buffer(not_modified);
                case reply::bad_request:
                    return to_buffer(bad_request);
                case reply::unauthorized:
                    return to_buffer(unauthorized);
                case reply::forbidden:
                    return to_buffer(forbidden);
                case reply::not_found:
                    return to_buffer(not_found);
                case reply::internal_server_error:
                    return to_buffer(internal_server_error);
                case reply::not_implemented:
                    return to_buffer(not_implemented);
                case reply::bad_gateway:
                    return to_buffer(bad_gateway);
                case reply::service_unavailable:
                    return to_buffer(service_unavailable);
                default:
                    return to_buffer(internal_server_error);
            }
        }

    } // namespace status_strings

    namespace misc_strings {

        const char name_value_separator[] = ": ";
        const char crlf[]                 = "\r\n";

    } // namespace misc_strings

    std::vector<reply::const_buffer> reply::to_buffers()
    {
        std::vector<reply::const_buffer> buffers;
        buffers.push_back(status_strings::to_buffer(status));
        for (std::size_t i = 0; i < headers.size(); ++i)
        {
            header& h = headers[i];
            buffers.push_back(status_strings::to_buffer(h.name));
            buffers.push_back(status_strings::to_buffer(misc_strings::name_value_separator));
            buffers.push_back(status_strings::to_buffer(h.value));
            buffers.push_back(status_strings::to_buffer(misc_strings::crlf));
        }
        buffers.push_back(status_strings::to_buffer(misc_strings::crlf));
        buffers.push_back(status_strings::to_buffer(content));
        return buffers;
    }

    namespace stock_replies {

        const char ok[] = "";
        const char created[] =
            "<html>"
            "<head><title>Created</title></head>"
            "<body><h1>201 Created</h1></body>"
            "</html>";
        const char accepted[] =
            "<html>"
            "<head><title>Accepted</title></head>"
            "<body><h1>202 Accepted</h1></body>"
            "</html>";
        const char no_content[] =
            "<html>"
            "<head><title>No Content</title></head>"
            "<body><h1>204 Content</h1></body>"
            "</html>";
        const char multiple_choices[] =
            "<html>"
            "<head><title>Multiple Choices</title></head>"
            "<body><h1>300 Multiple Choices</h1></body>"
            "</html>";
        const char moved_permanently[] =
            "<html>"
            "<head><title>Moved Permanently</title></head>"
            "<body><h1>301 Moved Permanently</h1></body>"
            "</html>";
        const char moved_temporarily[] =
            "<html>"
            "<head><title>Moved Temporarily</title></head>"
            "<body><h1>302 Moved Temporarily</h1></body>"
            "</html>";
        const char not_modified[] =
            "<html>"
            "<head><title>Not Modified</title></head>"
            "<body><h1>304 Not Modified</h1></body>"
            "</html>";
        const char bad_request[] =
            "<html>"
            "<head><title>Bad Request</title></head>"
            "<body><h1>400 Bad Request</h1></body>"
            "</html>";
        const char unauthorized[] =
            "<html>"
            "<head><title>Unauthorized</title></head>"
            "<body><h1>401 Unauthorized</h1></body>"
            "</html>";
        const char forbidden[] =
            "<html>"
            "<head><title>Forbidden</title></head>"
            "<body><h1>403 Forbidden</h1></body>"
            "</html>";
        const char not_found[] =
            "<html>"
            "<head><title>Not Found</title></head>"
            "<body><h1>404 Not Found</h1></body>"
            "</html>";
        const char internal_server_error[] =
            "<html>"
            "<head><title>Internal Server Error</title></head>"
            "<body><h1>500 Internal Server Error</h1></body>"
            "</html>";
        const char not_implemented[] =
            "<html>"
            "<head><title>Not Implemented</title></head>"
            "<body><h1>501 Not Implemented</h1></body>"
            "</html>";
        const char bad_gateway[] =
            "<html>"
            "<head><title>Bad Gateway</title></head>"
            "<body><h1>502 Bad Gateway</h1></body>"
            "</html>";
        const char service_unavailable[] =
            "<html>"
            "<head><title>Service Unavailable</title></head>"
            "<body><h1>503 Service Unavailable</h1></body>"
            "</html>";

        std::string to_string(reply::status_type status)
        {
            switch (status)
            {
                case reply::ok:
                    return ok;
                case reply::created:
                    return created;
                case reply::accepted:
                    return accepted;
                case reply::no_content:
                    return no_content;
                case reply::multiple_choices:
                    return multiple_choices;
                case reply::moved_permanently:
                    return moved_permanently;
                case reply::moved_temporarily:
                    return moved_temporarily;
                case reply::not_modified:
                    return not_modified;
                case reply::bad_request:
                    return bad_request;
                case reply::unauthorized:
                    return unauthorized;
                case reply::forbidden:
                    return forbidden;
                case reply::not_found:
                    return not_found;
                case reply::internal_server_error:
                    return internal_server_error;
                case reply::not_implemented:
                    return not_implemented;
                case reply::bad_gateway:
                    return bad_gateway;
                case reply::service_unavailable:
                    return service_unavailable;
                default:
                    return internal_server_error;
            }
        }

    } // namespace stock_replies

    reply reply::stock_reply(reply::status_type status) // 通常的返回
    {
        reply rep;
        rep.status  = status;
        rep.content = stock_replies::to_string(status);
        rep.headers.resize(2);
        rep.headers[0].name = "Content-Length";
        rep.headers[0].value = std::to_string(rep.content.size());
        rep.headers[1].name = "Content-Type";
        rep.headers[1].value = "text/html";
        return rep;
    }




    request_handler::request_handler(const std::string& doc_root)
        : doc_root_(doc_root)
    {}

    void request_handler::handle_request(const request& req, reply& rep)
    {
        // Decode url to path.
        std::string request_path;
        if (!url_decode(req.uri, request_path)) //uri解码
        {
            rep = reply::stock_reply(reply::bad_request);
            return;
        }

        // Request path must be absolute and not contain "..".
        if (request_path.empty() || request_path[0] != '/'
                || request_path.find("..") != std::string::npos)
        {
            rep = reply::stock_reply(reply::bad_request);
            return;
        }

        // If path ends in slash (i.e. is a directory) then add "index.html".
        if (request_path[request_path.size() - 1] == '/')
        {
            request_path += "index.html";
        }

        // Determine the file extension.
        std::size_t last_slash_pos = request_path.find_last_of("/");
        std::size_t last_dot_pos = request_path.find_last_of(".");
        std::string extension;
        if (last_dot_pos != std::string::npos && last_dot_pos > last_slash_pos)
        {
            extension = request_path.substr(last_dot_pos + 1);
        }

        // Open the file to send back.
        std::string full_path = doc_root_ + request_path;
        std::ifstream is(full_path.c_str(), std::ios::in | std::ios::binary);
        if (!is)
        {
            rep = reply::stock_reply(reply::not_found);
            return;
        }

        // TODO 显然可以在这里加一个mini的路由功能
        // 就在这里加 显然有两种路由
        // get post judge {'id','code':''}
        // get index_
        // Fill out the reply to be sent to the client.
        rep.status = reply::ok;
        char buf[512];
        while (is.read(buf, sizeof(buf)).gcount() > 0)
            rep.content.append(buf, is.gcount());
        rep.headers.resize(2);
        rep.headers[0].name = "Content-Length";
        rep.headers[0].value = std::to_string(rep.content.size());
        rep.headers[1].name = "Content-Type";
        rep.headers[1].value = mime_types::extension_to_type(extension);
    }

    bool request_handler::url_decode(const std::string& in, std::string& out)
    {
        out.clear();
        out.reserve(in.size());
        for (std::size_t i = 0; i < in.size(); ++i)
        {
            if (in[i] == '%')
            {
                if (i + 3 <= in.size())
                {
                    int value = 0;
                    std::istringstream is(in.substr(i + 1, 2));
                    if (is >> std::hex >> value)
                    {
                        out += static_cast<char>(value);
                        i += 2;
                    }
                    else
                    {
                        return false;
                    }
                }
                else
                {
                    return false;
                }
            }
            else if (in[i] == '+') // [URL中带加号的处理](https://blog.csdn.net/z69183787/article/details/35987711)
            {
                out += ' ';
            }
            else
            {
                out += in[i];
            }
        }
        return true;
    }




    request_parser::result_type request_parser::consume(request& req, char input)
    {
        switch (state_)
        {
            case content:
                req.content += input;
                return 
                    req.content.size() == req.content_size ? good :indeterminate;
            case method_start:
                if (!is_char(input) || is_ctl(input) || is_tspecial(input))
                {
                    return bad;
                }
                else
                {
                    state_ = method;
                    req.method.push_back(input);
                    return indeterminate;
                }
            case method:
                if (input == ' ')
                {
                    state_ = uri;
                    return indeterminate;
                }
                else if (!is_char(input) || is_ctl(input) || is_tspecial(input))
                {
                    return bad;
                }
                else
                {
                    req.method.push_back(input);
                    return indeterminate;
                }
            case uri:
                if (input == ' ')
                {
                    state_ = http_version_h;
                    return indeterminate;
                }
                else if (is_ctl(input))
                {
                    return bad;
                }
                else
                {
                    req.uri.push_back(input);
                    return indeterminate;
                }
            case http_version_h:
                if (input == 'H')
                {
                    state_ = http_version_t_1;
                    return indeterminate;
                }
                else
                {
                    return bad;
                }
            case http_version_t_1:
                if (input == 'T')
                {
                    state_ = http_version_t_2;
                    return indeterminate;
                }
                else
                {
                    return bad;
                }
            case http_version_t_2:
                if (input == 'T')
                {
                    state_ = http_version_p;
                    return indeterminate;
                }
                else
                {
                    return bad;
                }
            case http_version_p:
                if (input == 'P')
                {
                    state_ = http_version_slash;
                    return indeterminate;
                }
                else
                {
                    return bad;
                }
            case http_version_slash:
                if (input == '/')
                {
                    req.http_version_major = 0;
                    req.http_version_minor = 0;
                    state_ = http_version_major_start;
                    return indeterminate;
                }
                else
                {
                    return bad;
                }
            case http_version_major_start:
                if (is_digit(input))
                {
                    req.http_version_major = req.http_version_major * 10 + input - '0';
                    state_ = http_version_major;
                    return indeterminate;
                }
                else
                {
                    return bad;
                }
            case http_version_major:
                if (input == '.')
                {
                    state_ = http_version_minor_start;
                    return indeterminate;
                }
                else if (is_digit(input))
                {
                    req.http_version_major = req.http_version_major * 10 + input - '0';
                    return indeterminate;
                }
                else
                {
                    return bad;
                }
            case http_version_minor_start:
                if (is_digit(input))
                {
                    req.http_version_minor = req.http_version_minor * 10 + input - '0';
                    state_ = http_version_minor;
                    return indeterminate;
                }
                else
                {
                    return bad;
                }
            case http_version_minor:
                if (input == '\r')
                {
                    state_ = expecting_newline_1;
                    return indeterminate;
                }
                else if (is_digit(input))
                {
                    req.http_version_minor = req.http_version_minor * 10 + input - '0';
                    return indeterminate;
                }
                else
                {
                    return bad;
                }
            case expecting_newline_1:
                if (input == '\n')
                {
                    state_ = header_line_start;
                    return indeterminate;
                }
                else
                {
                    return bad;
                }
            case header_line_start:
                if (input == '\r')
                {
                    state_ = expecting_newline_3;
                    return indeterminate;
                }
                else if (!req.headers.empty() && (input == ' ' || input == '\t'))
                {
                    state_ = header_lws;
                    return indeterminate;
                }
                else if (!is_char(input) || is_ctl(input) || is_tspecial(input))
                {
                    return bad;
                }
                else
                {
                    req.headers.push_back(header());
                    req.headers.back().name.push_back(input);
                    state_ = header_name;
                    return indeterminate;
                }
            case header_lws:
                if (input == '\r')
                {
                    state_ = expecting_newline_2;
                    return indeterminate;
                }
                else if (input == ' ' || input == '\t')
                {
                    return indeterminate;
                }
                else if (is_ctl(input))
                {
                    return bad;
                }
                else
                {
                    state_ = header_value;
                    req.headers.back().value.push_back(input);
                    return indeterminate;
                }
            case header_name:
                if (input == ':')
                {
                    state_ = space_before_header_value;
                    if( req.headers.back().name == CONTENT_LENGTH)
                        req.has_content = true;
                    return indeterminate;
                }
                else if (!is_char(input) || is_ctl(input) || is_tspecial(input))
                {
                    return bad;
                }
                else
                {
                    req.headers.back().name.push_back(input);
                    return indeterminate;
                }
            case space_before_header_value:
                if (input == ' ')
                {
                    state_ = header_value;
                    return indeterminate;
                }
                else
                {
                    return bad;
                }
            case header_value:
                if (input == '\r')
                {
                    state_ = expecting_newline_2;
                    if(req.headers.back().name == CONTENT_LENGTH){
                        req.content_size = atoll(req.headers.back().value.c_str());
                    }
                    return indeterminate;
                }
                else if (is_ctl(input))
                {
                    return bad;
                }
                else
                {
                    req.headers.back().value.push_back(input);
                    return indeterminate;
                }
            case expecting_newline_2:
                if (input == '\n')
                {
                    state_ = header_line_start;
                    return indeterminate;
                }
                else
                {
                    return bad;
                }
            case expecting_newline_3:
                if( input == '\n'){
                    if( req.has_content){
                        state_ = content;
                        return indeterminate;
                    }
                    return good;
                }
                else
                    return bad;
                //return (input == '\n') ?  : bad;
            default:
                return bad;
        }
    }
}
