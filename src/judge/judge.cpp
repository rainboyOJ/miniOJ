#include "judge.hpp"

namespace  judge {

std::string readFile(const char * path){
    std::stringstream ss;
    std::ifstream f(path);
    ss << f.rdbuf();
    return ss.str();
}

void exec(const char* cmd,std::ostream& __out) {
    char buffer[128];
    FILE* pipe = popen(cmd, "r");
    if (!pipe) throw std::runtime_error(std::string("popen() failed!") + __FILE__ + " line: " +  std::to_string(__LINE__).c_str());
    try {
        while (fgets(buffer, sizeof buffer, pipe) != NULL) {
            __out << buffer;
        }
    } catch (...) {
        pclose(pipe);
        throw;
    }
    pclose(pipe);
    //return result;
}


    std::string_view result_to_string(RESULT_MEAN mean) {
        using namespace std::literals;
        switch(mean){
            case WRONG_ANSWER:              return "WRONG_ANSWER"sv;
            case CPU_TIME_LIMIT_EXCEEDED:   return "CPU_TIME_LIMIT_EXCEEDED"sv;
            case REAL_TIME_LIMIT_EXCEEDED:  return "REAL_TIME_LIMIT_EXCEEDED"sv;
            case MEMORY_LIMIT_EXCEEDED:     return "MEMORY_LIMIT_EXCEEDED"sv;
            case RUNTIME_ERROR:             return "RUNTIME_ERROR"sv;
            case SYSTEM_ERROR:              return "SYSTEM_ERROR"sv;
            default:    return "UNKOWN"sv;
        }
    }

    std::string_view lang_to_string(SUPORT_LANG lang) {
        using namespace std::literals;
        switch(lang){
            case SUPORT_LANG::CPP:                   return "cpp"sv;
            case SUPORT_LANG::PY3:                   return "python3"sv;
            default:    return "UNSUPORT"sv;
        }
    }

    std::tuple<SUPORT_LANG,std::string> string_to_lang(std::string_view lang){
        if( lang == "cpp" || lang == "c++" ) 
            return std::make_tuple(SUPORT_LANG::CPP,".cpp");
        if( lang == "py3" || lang == "python3" ) 
            return std::make_tuple(SUPORT_LANG::PY3,".py");
        return std::make_tuple(SUPORT_LANG::UNSUPORT,"");
    }

    judge_args getCompileArgs(const SUPORT_LANG lang, const fs::path& cwd,std::string_view code_name){
        switch(lang){
            case SUPORT_LANG::PY3:      return compile_PY3_args(cwd,code_name);
            case SUPORT_LANG::CPP:      return compile_CPP_args(cwd,code_name);
        }
    }

    judge_args getJudgeArgs(SUPORT_LANG lang,const fs::path& cwd,
                std::string_view code_name,
                std::string_view in_file_fullpath, //????????????
                std::string_view out_file, //????????????
                std::size_t __time, /* ms*/ std::size_t __memory /* mb */){
        switch(lang){
            case SUPORT_LANG::PY3:      return judge_PY3_args(cwd,code_name,in_file_fullpath,out_file,__time,__memory);
            case SUPORT_LANG::CPP:      return judge_CPP_args(cwd,code_name,in_file_fullpath,out_file,__time,__memory);
        }
    }


judge_args::judge_args(
                std::size_t max_cpu_time, std::size_t max_real_time, std::size_t max_process_number, std::size_t max_memory, std::size_t max_stack, std::size_t max_output_size,
                std::string seccomp_rule_name,
                const fs::path& cwd, const fs::path& input_path, const fs::path& output_path, const fs::path& error_path, const fs::path& log_path, const fs::path& exe_path,
                //std::vector<std::string> args,
                //std::vector<std::string> env,
                int gid, int uid)
            : max_cpu_time{max_cpu_time},
            max_real_time{max_real_time},
            max_process_number{max_process_number},
            max_memory{max_memory},
            max_stack{max_stack},
            max_output_size{max_output_size},
            seccomp_rule_name{std::move(seccomp_rule_name)},
            cwd{cwd},
            exe_path{exe_path},
            input_path{input_path},
            output_path{output_path},
            error_path{error_path},
            log_path{log_path},
            gid{gid},
            uid{uid} {}

judge_args::operator std::string() const { //??????string
            std::stringstream args_str;
            args_str << ' ';

            args_str <<"--max_cpu_time="<<max_cpu_time << ' ';
            args_str <<"--max_real_time="<<max_real_time<< ' ';
            args_str <<"--max_process_number="<<max_process_number<< ' ';
            args_str <<"--max_memory="<<max_memory<< ' ';
            args_str <<"--max_stack="<<max_stack<< ' ';
            args_str <<"--max_output_size="<<max_output_size<< ' ';
            args_str <<"--seccomp_rule_name="<<seccomp_rule_name<< ' ';

            args_str << "--cwd=" << cwd << ' ';
            args_str << "--input_path=" << input_path<< ' ';
            args_str << "--output_path=" << output_path<< ' ';
            args_str << "--error_path=" << error_path<< ' ';
            args_str << "--log_path=" << log_path<< ' ';
            args_str << "--exe_path=" << exe_path<< ' ';

            for (const auto& e : args) {
                args_str << "--args=" <<'"'  << e << '"' << ' ';
            }
            for (const auto& e : env) {
                args_str << "--env=" <<'"'  << e << '"' << ' ';
            }
            args_str << "--gid=" << gid << ' ';
            args_str << "--uid=" << uid << ' ';
            return args_str.str();
}

compile_PY3_args::compile_PY3_args (const fs::path& cwd,std::string_view code_name)
            :judge_args(
                    10*1000,90*1000,unlimit,unlimit,unlimit,128ull*(1ull<<30),
                    "null", //?????????sec
                    cwd,"/dev/null",
                    cwd/"compile_output",
                    cwd/"compile_error",
                    cwd/"compile_log",
                    "/usr/bin/python3",
                    0,0)
        {
            args = { "-m", "py_compile",cwd/code_name};
            env = {"PATH=/usr/bin"};
        }

compile_CPP_args::compile_CPP_args (const fs::path& cwd,std::string_view code_name)
            :judge_args(
                    10*1000,90*1000,unlimit,unlimit,unlimit,512ull*(1ull<<30),
                    "null", //?????????sec
                    cwd,"/dev/null",
                    cwd/"compile_output",
                    cwd/"compile_error",
                    cwd/"compile_log",
                    "/usr/bin/g++",
                    0,0)
        {
            args = { "-o",fs::path(code_name).stem(),std::string(code_name)};
            env = {"PATH=/usr/bin"};
        }

judge_PY3_args::judge_PY3_args(const fs::path cwd,
                std::string_view code_name,
                std::string_view in_file,
                std::string_view out_file,
                std::size_t __time, /* ms*/ std::size_t __memory /* mb */)
            :judge_args(
                    __time,10*__time,10,__memory*(1ull<<30),128_MB,128_MB,
                    "null", //?????????sec
                    cwd,
                    in_file,    /*input_path*/
                    cwd/out_file,
                    cwd/"judge_error",
                    cwd/"judge_log",
                    "/usr/bin/python3",
                    0,0) { 
                //log("judge_PY3_args",in_file);
                args = {cwd/code_name};
                env = {"PATH=/usr/bin"};
            }


judge_CPP_args::judge_CPP_args(const fs::path cwd,
                std::string_view code_name,
                std::string_view in_file,
                std::string_view out_file,
                std::size_t __time, /* ms*/ std::size_t __memory /* mb */)
            :judge_args(
                    __time,10*__time,10,__memory*(1ull<<30),128_MB,128_MB,
                    "null", //?????????sec
                    cwd,
                    in_file,    /*input_path*/
                    cwd/out_file,
                    cwd/"judge_error",
                    cwd/"judge_log",
                    fs::path(code_name).stem(),
                    0,0) { 
                //args = {};
                env = {"PATH=/usr/bin"};
            }

    result __judger(judge_args args){
        std::stringstream ss;
        //log("??????",(judge_bin + static_cast<std::string>(args)).c_str() );
        //std::cout << std::endl ;
        exec( ( judge_bin + static_cast<std::string>(args)).c_str() ,ss);
        result RESULT;
        ss >> RESULT.cpu_time;
        ss >> RESULT.real_time;
        ss >> RESULT.memory;
        ss >> RESULT.signal;
        ss >> RESULT.exit_code;
        ss >> RESULT.error;
        ss >> RESULT.result;
        return RESULT;
    }


bool Judger::compile(judge_args & args){
    //compile_python_args(work_path, code_file_name.c_str());
    auto res = __judger(args);
    print_result(res);
    return res.exit_code == 0 && res.error == 0;
}


std::string_view STATUS_to_string(STATUS s){
    using namespace std::literals;
    switch(s){
        case STATUS::WAITING:       return "WAITING"sv;
        case STATUS::JUDGING:       return "JUDGING"sv;
        case STATUS::COMPILE_ERROR: return "COMPILE_ERROR"sv;
        case STATUS::END:       return "END"sv;
    }
}

//TODO return std::vector<result>
//????????? msg??????
auto Judger::run()->
        std::tuple<STATUS,std::string,std::vector<result>> 
{
    //??????
    try {

        auto args = getCompileArgs(lang, work_path, code_name);
        if( !compile(args) ){
            std::string msg = readFile(args.error_path.c_str());
            if( msg.length() == 0)
                msg =  readFile(args.log_path.c_str());
            auto a = std::make_tuple(1,2,3);
            return std::make_tuple(STATUS::COMPILE_ERROR,msg, std::vector<result> {});
        }
    }
    catch(...){
        throw std::runtime_error("compile throw error");
    }

    try {
        Problem p(problem_base,pid); //??????pid?????? ????????????
        std::vector<result> results{};
        auto time_limit = 1000;
        auto memory_limit = 128;

        for(int i=0;i<p.input_data.size();++i){    // ??????????????????
            auto& in_file = p.input_data[i].second;
            auto& out_file = p.output_data[i].second;
            //log("in_file",in_file);
            //log("out_file",out_file);
            std::string user_out_file = "out"+std::to_string(i);
            auto args = getJudgeArgs(lang, work_path, code_name, in_file, user_out_file, time_limit, 128+memory_limit);
            //log(i, static_cast<std::string>(args) );
            auto res = __judger(args);
            if( res.error != 0  || res.result !=0)
                results.push_back(res);
            else  {
                //??????memory
                if( res.memory >= 1024ull * 1024 *1024 * memory_limit){
                    res.result = MEMORY_LIMIT_EXCEEDED;
                }
                else {
                    //res.result = MEMORY_LIMIT_EXCEEDED;
                    //????????????
                        log_one(args.output_path);
                        log_one(out_file);
                    if( !cyaron::Check::noipstyle_check(args.output_path.c_str(), out_file) ){
                        res.result = WRONG_ANSWER;
                    }
                }
                results.push_back(res);
            }
        }
        return std::make_tuple(STATUS::END,"", std::move(results));
    }
    catch(...){
    }
}

}; // namespace judge
