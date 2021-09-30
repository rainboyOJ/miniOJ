// 用于执行judge
#pragma once

#include <atomic>
#include <string>
#include <vector>
#include <thread>
#include <array>
#include <atomic>
#include <type_traits>
#include <filesystem>
#include <condition_variable>


#include "judge.hpp"
#include "../utils.hpp"

namespace judge {

    //评测队列，可进可出，存等待评测的代码
    template<size_t size = 100000>
        class judge_queue {
            //explicit judge_queue(){}
            public:

                bool enque(int id){
                    if( writeableCnt.fetch_sub(1) <= 0){
                        writeableCnt.fetch_add(1);
                        return false;
                    }
                    int m = size;
                    tail.compare_exchange_strong(m,0); //  因为 tail 只能加1 
                    int pos = tail.fetch_add(1) % size; // 得到这个位置

                    // change
                    q[pos] = id;

                    readableCnt.fetch_add(1);
                    return true;
                }
                bool deque(int * id){
                    if( readableCnt.fetch_sub(1) <= 0) {
                        readableCnt.fetch_add(1);
                        return false;
                    }

                    int m = size;
                    head.compare_exchange_strong(m, 0);
                    int pos = head.fetch_add(1) % size;
                    *id = q[pos];

                    writeableCnt.fetch_add(1);

                    return true;
                }
            private:
                std::array<int, size> q;
                std::atomic_int head{0};
                std::atomic_int tail{0};
                std::atomic_int readableCnt{0};
                std::atomic_int writeableCnt{size};
        };


    struct msg {
        std::string request_ip; //请求ip
        std::string pid;        //pid
        std::string lang;       //代码语言 c++ python3
        std::string code_path;  //

        STATUS status;      //状态
        std::string msg;    //结果的描述
        std::vector<result> results;
    };


    //namespace fs = std::filesystem;
    struct Make_code_path {
        explicit Make_code_path(std::string_view _path)
            : base_path{_path}
        {}

        std::filesystem::path operator()(int id,std::string_view lang) const{
            return base_path / std::to_string(id) 
                / (std::string("main") + std::get<1>(string_to_lang(lang)));
        }

        std::filesystem::path base_path;
    };



    //存评测结果的队列，只能入队的队列
    template<size_t size=100000>
        class judge_msg_queue {
            public:
                explicit judge_msg_queue(const Make_code_path& _mcp)
                    :_mcp{_mcp}
                {}

                int enque( std::string_view request_ip ,
                        std::string_view pid,
                        std::string_view lang,
                        STATUS status = STATUS::JUDGING ){ //入队
                    int pos;
                    if( (pos = head.fetch_add(1) ) >= size){ //超过的大小
                        head.fetch_sub(1);
                        return -1;
                    }
                    msg_que[pos].request_ip = request_ip;
                    msg_que[pos].pid        = pid;
                    msg_que[pos].lang       = lang;
                    msg_que[pos].status     = STATUS::JUDGING;
                    msg_que[pos].code_path  = _mcp(pos,lang) ;
                    return pos;
                }
                bool deque(); //不能出队
                int get_now_head() const { return head;}
                std::string_view get_code_path_by_id(int id){
                    //TODO 保证 id <=head
                }

                msg& get_msg(int id){
                    if( id < 0 ||  id >= head.load())
                        throw std::invalid_argument("invalid_argument id");
                    return msg_que[id];
                }
            private:
                std::array<msg,size> msg_que; //存结果
                const Make_code_path& _mcp;
                std::atomic_int head{0};
        };


    template<size_t jsq_size,size_t jq_size,size_t poolsize>
    class JudgeWorker{
    public:
        explicit JudgeWorker(std::string_view problem_base,
                std::string_view judge_base_path
                )
            :problem_base{problem_base},
             mcp{judge_base_path}
        {
            addThread(poolsize);
        }

        void set_judge_base_path(std::string_view judge_base_path){
            mcp.base_path = judge_base_path;
        }
        int enque(
                std::string_view code,
                std::string_view request_ip ,
                std::string_view pid,
                std::string_view lang,
                STATUS status = STATUS::WAITING
                ){ 
            auto id = jsq.enque(request_ip,pid,lang,status);

            if( id == -1) throw std::runtime_error("结果队列存满了");

            //写入文件
            saveFile(code,jsq.get_msg(id).code_path);
            jq.enque(id); //加入评测队列
            _task_cv.notify_one();
            log("加入评测队列");
            return id;
        } 
        // 通过结果id 来得到结果
        std::string judge_result_to_json(int id){

            try {
                //typename judge_msg_queue<jsq_size>::msg __res{};
                auto& __res = jsq.get_msg(id);
                std::stringstream json;
                json << "{";
                json << R"("status": ")"; 
                json << STATUS_to_string(__res.status)<< "\","; //请求ip
                json << R"("msg": ")";
                json << __res.msg<< "\"";  //

                if( __res.status != STATUS::END ){
                    json << "}";
                    return json.str();
                }
                json << ","; // msg的,

                json << R"("request_ip": ")"; 
                json << __res.request_ip << "\","; //请求ip
                json << R"("pid": ")"; 
                json << __res.pid << "\",";        //pid
                json << R"("lang": ")"; 
                json << __res.lang << "\",";       //代码语言 c++ python3
                json << R"("code_path": ")";
                json << __res.code_path << "\",";  //


                json << R"("results":[)";

                for (int i = 0; i < __res.results.size() ;i++) {
                    auto & e = __res.results[i];
                    json << "{";
                    json << "\"cpu_time\":"<<   e.cpu_time<<",";
                    json << "\"real_time\":"<<  e.real_time<<",";
                    json << "\"memory\":"<<     e.memory<<",";
                    json << "\"signal\":"<<     e.signal<<",";
                    json << "\"exit_code\":"<<  e.exit_code<<",";
                    json << "\"error\":"<<      e.error<<",";
                    json << "\"result\":"<<     e.result;
                    json << "}";
                    if( i != __res.results.size()-1)
                        json << ",";
                }

                json << "]}";
                return json.str();
            }
            catch(std::exception & e){
                //TODO
            }
        }

    private:
        void addThread(unsigned int size){
            for(int i=1;i<=size;++i){
                _pool.emplace_back(&JudgeWorker::doWork,this);
            }
        }
        void doWork(){
            while ( _run ) {
                {
                    int id;
                    {
                        std::unique_lock<std::mutex> lck{_lock};
                        _task_cv.wait(lck,[&id,this](){
                                return this->jq.deque(&id);
                                });
                        if(!_run) return;
                    }
                    _idlThrNum.fetch_sub(1);
                    judge(id); //执行任务
                    _idlThrNum.fetch_add(1);
                }
            }
        }
        void judge(int id){
            log("执行评测",id);
            auto &msg = jsq.get_msg(id);
            Judger jg(
                    msg.code_path,
                    std::get<judge::SUPORT_LANG>( judge::string_to_lang(msg.lang) ),
                    msg.pid,
                    problem_base
                    );
            auto res = jg.run();
            msg.msg = std::get<std::string>(res);
            msg.results = std::move(std::get<std::vector<result>>(res) );
            msg.status = std::get<STATUS>(res);
            log(msg.msg);
            log(msg.results.size());
            for(int i=0;i<=msg.results.size()-1;++i){
                log_one(i);
                print_result(msg.results[i]);
            }

            log( STATUS_to_string(msg.status));
        }


        Make_code_path mcp; //默认的位置
        judge_queue<jq_size>      jq;
        judge_msg_queue<jsq_size> jsq{mcp};


        std::vector<std::thread> _pool;
        std::condition_variable _task_cv;//条件阻塞
        std::mutex              _lock;
        std::atomic<bool>       _run{true};    //线程池是否执行
        std::atomic<int>        _idlThrNum{0}; //空闲线程数量
        std::string problem_base; //题目路径

    };

}
