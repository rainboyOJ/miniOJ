#include "request.hpp"
#include "mime_types.hpp"

namespace http {

    #define CONTENT_LENGTH "Content-Length"
    //request_handler::request_handler(const std::string& doc_root)
    //{}

    void request_handler::handle_request(request& req, reply& rep)
    {
        // Decode url to path.
        std::string request_path;
        if (!url_decode(req.uri, request_path)) //uri解码
        {
            rep = reply::stock_reply(reply::bad_request);
            return;
        }
        req.uri = request_path;

        // Request path must be absolute and not contain "..".
        if (request_path.empty() || request_path[0] != '/'
                || request_path.find("..") != std::string::npos)
        {
            rep = reply::stock_reply(reply::bad_request);
            return;
        }

        // If path ends in slash (i.e. is a directory) then add "index.html".
        //if (request_path[request_path.size() - 1] == '/')
        //{
            //request_path += "index.html";
        //}
        
        return; //这里直接返回，后续的交给miniRouter去处理

        // Determine the file extension.
        //std::size_t last_slash_pos = request_path.find_last_of("/");
        //std::size_t last_dot_pos = request_path.find_last_of(".");
        //std::string extension;
        //if (last_dot_pos != std::string::npos && last_dot_pos > last_slash_pos)
        //{
            //extension = request_path.substr(last_dot_pos + 1);
        //}

        //// Open the file to send back.
        //std::string full_path = doc_root_ + request_path;
        //std::ifstream is(full_path.c_str(), std::ios::in | std::ios::binary);
        //if (!is)
        //{
            //rep = reply::stock_reply(reply::not_found);
            //return;
        //}

        //// Fill out the reply to be sent to the client.
        //rep.status = reply::ok;
        //char buf[512];
        //while (is.read(buf, sizeof(buf)).gcount() > 0)
            //rep.content.append(buf, is.gcount());
        //rep.headers.resize(2);
        //rep.headers[0].name = "Content-Length";
        //rep.headers[0].value = std::to_string(rep.content.size());
        //rep.headers[1].name = "Content-Type";
        //rep.headers[1].value = mime_types::extension_to_type(extension);
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







} //end namespace http
