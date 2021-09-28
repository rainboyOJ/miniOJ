#include "server.hpp"
#include "judge/judge.hpp"
#include "judge/exec_judge.hpp"
#include "utils.hpp"

//judge::judge_queue jq; //定义一全局的评测队列
//judge::Make_code_path mcp("/home/rainboy/tmp/tmp"); //默认的位置
//judge::judge_msg_queue result_q(mcp);

judge::JudgeWorker<100000,100000,2> Judger{
    "/home/rainboy/mycode/RainboyOJ/problems/problems",
    "/home/rainboy/tmp/tmp"
};

int main(int argc,char * argv[]){

    //一个不停取出数据 进行评测的线程

    HttpServer http(8080);

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
