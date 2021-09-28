#pragma once

#include <string>
#include <string_view>
#include <exception>
#include <stdexcept>
#include <sstream>
#include <fstream>
#include <vector>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <regex>
#include "check.hpp"
#include "../utils.hpp"

namespace  judge {

std::string readFile(std::string_view path);

#define __print_result(node,RESULT) std::cout << std::setw(12) << #node ": " << RESULT.node <<'\n';
#define print_result(RESULT)\
    __print_result(cpu_time,RESULT);\
    __print_result(real_time,RESULT);\
    __print_result(memory,RESULT);\
    __print_result(signal,RESULT);\
    __print_result(exit_code,RESULT);\
    __print_result(error,RESULT);\
    __print_result(result,RESULT);

    using ull = unsigned long long;
    constexpr ull operator ""_MB(ull a){
        return a*1024*1024*1024;
    }

    constexpr std::size_t operator ""_SEC(ull a){
        return a*1000;
    }


    namespace fs = std::filesystem;
    const std::string judge_bin = "/usr/bin/judger_core";
    const int unlimit = 0;

    enum {
        SUCCESS             = 0,
        INVALID_CONFIG      = -1,
        FORK_FAILED         = -2,
        PTHREAD_FAILED      = -3,
        WAIT_FAILED         = -4,
        ROOT_REQUIRED       = -5,
        LOAD_SECCOMP_FAILED = -6,
        SETRLIMIT_FAILED    = -7,
        DUP2_FAILED         = -8,
        SETUID_FAILED       = -9,
        EXECVE_FAILED       = -10,
        SPJ_ERROR           = -11,
        COMPILE_FAIL        = -12 // TODO
    };

    enum RESULT_MEAN {
        WRONG_ANSWER             = -1,
        CPU_TIME_LIMIT_EXCEEDED  = 1,
        REAL_TIME_LIMIT_EXCEEDED = 2,
        MEMORY_LIMIT_EXCEEDED    = 3,
        RUNTIME_ERROR            = 4,
        SYSTEM_ERROR             = 5
    };

    enum class STATUS : int {
        WAITING,
        PROBLEM_ERROR,
        PROBLEM_DATA_NOT_EXISTS,
        JUDGING,
        COMPILE_ERROR,
        END
    };
    std::string_view STATUS_to_string(STATUS s);

    std::string_view result_to_string(RESULT_MEAN mean);

    // 存结果 POD
    struct result { int cpu_time;
        int real_time;
        long memory;
        int signal;
        int exit_code;
        int error;
        int result;
    };

    //更好的result
    //struct result_detail {
        //int cpu_time;       //ms
        //int real_time;      //ms
        //float memory;       //mb
        //int signal;
        //int exit_code;
        //std::string error;
        //std::string result; //
        //result_detail& operator=(struct result& r){
            //cpu_time = r.cpu_time;
            //real_time = r.real_time;
            //memory = r.memory/1024.0/ 1024.0/1024.0; //todo
            //exit_code  = r.exit_code;
            //signal = r.signal;
            //error =  //error 与result 到底是什么意思
            //return *this;
        //}
    //};



    void exec(const char* cmd,std::ostream& __out);

    //支持的语言
    enum class SUPORT_LANG {
        CPP,
        PY3,
        UNSUPORT
    };

    //  judge compile args factory by lang
    enum class JUDGE_ARGS_TYPE {
        COMPILE,
        JUDGE,
    };
    constexpr auto COMPILE = JUDGE_ARGS_TYPE::COMPILE;
    constexpr auto JUDGE = JUDGE_ARGS_TYPE::COMPILE;

    std::string_view lang_to_string(SUPORT_LANG lang);

    std::tuple<SUPORT_LANG,std::string> string_to_lang(std::string_view lang);

    //基类
    struct judge_args {
        explicit judge_args(
                std::size_t max_cpu_time, std::size_t max_real_time, std::size_t max_process_number, std::size_t max_memory, std::size_t max_stack, std::size_t max_output_size,
                std::string seccomp_rule_name,
                const fs::path& cwd, const fs::path& input_path, const fs::path& output_path, const fs::path& error_path, const fs::path& log_path, const fs::path& exe_path,
                int gid, int uid);

        std::size_t max_cpu_time;
        std::size_t max_real_time;
        std::size_t max_process_number;
        std::size_t max_memory;//  512mb
        std::size_t max_stack;
        std::size_t max_output_size;
        std::string seccomp_rule_name;
        fs::path cwd;
        fs::path input_path;
        fs::path output_path;
        fs::path error_path;
        fs::path log_path;
        fs::path exe_path;

        std::vector<std::string> args;
        std::vector<std::string> env;
        int gid;
        int uid;

        operator std::string() const; 
    };



    //参数 编译的目录 评测的代码名字
#define create_compile_args(name) struct compile_##name##_args : public judge_args {\
        compile_##name##_args() = delete;\
        explicit compile_##name##_args(const fs::path& cwd,std::string_view code_name);\
    };\

    create_compile_args(PY3)
    create_compile_args(CPP)

    judge_args getCompileArgs(const SUPORT_LANG lang, const fs::path& cwd,std::string_view code_name);


#define create_judge_name(name) struct judge_##name##_args: public judge_args {\
        judge_##name##_args() = delete;\
        explicit judge_##name##_args(const fs::path cwd,\
                std::string_view code_name,\
                std::string_view in_file_fullpath,\
                std::string_view out_file,\
                std::size_t __time, /* ms*/ std::size_t __memory /* mb */);\
    };

    create_judge_name(PY3)
    create_judge_name(CPP)

    judge_args getJudgeArgs(SUPORT_LANG lang,const fs::path& cwd,
                std::string_view code_name,
                std::string_view in_file_fullpath, //完整路径
                std::string_view out_file, //只要名字
                std::size_t __time, /* ms*/ std::size_t __memory /* mb */);


    //得到
    result __judger(judge_args args);

    struct Problem {
        Problem()=delete;
        explicit Problem(const std::string_view path,const std::string_view pid)
        {
            //检查路径是否存在
            auto p_path = fs::path(path) / pid;
            //log("p_path",p_path);
            if(not fs::exists(p_path) )
                throw std::runtime_error(p_path.string() + " 不存在!");

            auto data_path = p_path / "data";
            if(not fs::exists(data_path) )
                throw std::runtime_error(data_path.string() + " 不存在!");
            //对数据进行检查

            for ( const auto& e : fs::directory_iterator(data_path) ) {
                auto filename = fs::path(e).filename();
                std::cmatch cm;
                if( std::regex_match(filename.c_str(),cm,input_regex ) ){
                    input_data.emplace_back(stoi(cm[1].str()),e.path());
                }
                else if(  std::regex_match(filename.c_str(),cm,output_regex ) ){
                    output_data.emplace_back(stoi(cm[1].str()),e.path());
                }
            }
            if( input_data.size() != output_data.size() )
                throw std::runtime_error(p_path.string() + " 输入输出数据个数不匹配");

            sort(input_data.begin(),input_data.end());
            sort(output_data.begin(),output_data.end());
        };

        std::vector<std::pair<int,std::string>> input_data; //数据
        std::vector<std::pair<int,std::string>> output_data; //数据
        const std::regex input_regex{"[A-Za-z_]+(\\d+)\\.in"};
        const std::regex output_regex{"[A-Za-z_]+(\\d+)\\.(out|ans)"};
    };

    struct Judger{
        explicit Judger
        (
         std::string_view code_full_path, //代码的路径
         SUPORT_LANG lang,      //语言
         std::string_view pid,       //problem id
         std::string_view problem_base
        ):work_path{ fs::path(code_full_path).parent_path() },
            code_name{fs::path(code_full_path).filename()},
            lang{lang},
            pid{pid},
            problem_base{problem_base}
            {
                //创建对应的文件夹
                //if( !  fs::create_directories(work_path) ){
                    //throw std::runtime_error(std::string("创建对应的文件夹 失败") + work_path.string());
                //}
                //定入代码
                //std::ofstream __code(code_path.c_str());
                //__code << code;
                //__code.close();
            }

        
        std::tuple<STATUS,std::string,std::vector<result>> 
            run();       //开始评测
        bool compile(judge_args & args);
        const SUPORT_LANG lang; // 评测的语言
        const std::string pid;  // pid
        std::string_view problem_base;

        //哪里的位置，开始评测
        fs::path    work_path;
        std::string code_name;

    };



}; // namespace judge
