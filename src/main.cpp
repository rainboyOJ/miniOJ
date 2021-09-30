#include "server.hpp"
#include "judge/judge.hpp"
#include "judge/exec_judge.hpp"
#include "utils.hpp"

//judge::judge_queue jq; //定义一全局的评测队列
//judge::Make_code_path mcp("/home/rainboy/tmp/tmp"); //默认的位置
//judge::judge_msg_queue result_q(mcp);

auto problem_base = "/home/rainboy/mycode/RainboyOJ/problems/problems";
judge::JudgeWorker<100000,100000,2> Judger{
    "/home/rainboy/mycode/RainboyOJ/problems/problems",
    "/home/rainboy/tmp/tmp"
};

Base64 base_64;

int main(int argc,char * argv[]){

    //一个不停取出数据 进行评测的线程

    HttpServer http(8080,"../www"); // 端口,静态资源地址


    //获取一个题目
    http.router.reg<http::GET>(std::regex("/problem/(\\d+)"),[&http](http::request& req,http::reply& rep ){

            auto pid  = req.sm[1].str();
            //得到这个题目的content.md
            auto content_path = fs::path(problem_base) / pid / "content.md";
            log_one(content_path);
            if( !  fs::exists(content_path) ){
                rep = http::reply::stock_reply(http::reply::not_found);
                return;
            }
            auto ss = readFile(content_path);
            //log_one( base_64.Encode(ss.str().c_str(), ss.str().size()));

            //读取article.html
            auto article_html_path = http.doc_root  / "article.html";

            auto res = std::regex_replace(
                    readFile(article_html_path),
                    std::regex("_replace_markdown_"),
                    base_64.Encode(ss.c_str(), ss.size()),
                    std::regex_constants::format_first_only);
            log_one(res);

            rep.set_content(res.c_str());

            });



    //获取一个 测试的结果
    http.router.reg<http::GET>(std::regex("/result/(\\d+)/json"),[&http](http::request& req,http::reply& rep ){
            auto id = atoi(req.sm[1].str().c_str());
            auto ss = Judger.judge_result_to_json(id);
            rep.set_content_json(ss.c_str());
        });

    http.router.reg<http::GET>(std::regex("/result/(\\d+)"),[&http](http::request& req,http::reply& rep ){
            //auto id = atoi(req.sm[1].str().c_str());
            auto html_path = http.doc_root  / "result.html";
            rep.set_content( readFile(html_path) );
        });


    //注册路由
    http.router.reg<http::POST>("/judge",
            [](http::request& a,http::reply& b ){
                //b.set_content("hello world ,this by miniRouter!");
                std::string_view lang;
                std::string_view pid;
                for (const auto& e : a.headers) {
                    if(e.name == "lang")
                        lang = e.value;
                    else if(e.name == "pid")
                        pid = e.value;
                }
                log("lang",lang,"pid",pid);
                //1. 是否是支持的语言
                judge::SUPORT_LANG slang = std::get<judge::SUPORT_LANG>(judge::string_to_lang(lang));
                if( slang == judge::SUPORT_LANG::UNSUPORT ){
                    b.set_content_json(R"raw({"code":-1,"msg":"not support lang!"})raw");
                    return;
                }
                log("clinet ip ",a.remote_ip);

                int id ;
                try {
                    id = Judger.enque(a.content, a.remote_ip, pid, lang);
                }
                catch(std::exception& e){
                    log("exception");
                    return;
                }

                //b.set_content(a.content);
                std::stringstream ss;
                ss << R"raw({"code":0,"id":)raw" <<  id  << R"raw(})raw" ;
                //log("code_path",result_q.get_msg(id).code_path);
                b.set_content_json(
                        ss.str()
                        );
            }
    );

    http.go();
    return 0;
}
