// 用于执行judge
#pragma once

#include <atomic>
#include <string>
#include <array>
#include <atomic>
#include <type_traits>
#include <filesystem>


#include "judge.hpp"

namespace judge {

    //评测队列，可进可出，存等待评测的代码
    template<int size = 100000>
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
                    int pos = tail.fetch_add(1) % m; // 得到这个位置

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
        RESULT_MEAN status;
        std::string code_path;  //
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
                        RESULT_MEAN status = RESULT_MEAN::JUDGING ){ //入队
                    int pos;
                    if( (pos = head.fetch_add(1) ) >= size){ //超过的大小
                        head.fetch_sub(1);
                        return -1;
                    }
                    msg_que[pos].request_ip = request_ip;
                    msg_que[pos].pid        = pid;
                    msg_que[pos].lang       = lang;
                    msg_que[pos].status     = RESULT_MEAN::JUDGING;
                    msg_que[pos].code_path  = _mcp(pos,lang) ;
                    return pos;
                }
                bool deque(); //不能出队
                int get_now_head() const { return head;}
                std::string_view get_code_path_by_id(int id){
                    //TODO 保证 id <=head
                }
                std::array<msg,size> msg_que; //存结果
            private:
                const Make_code_path& _mcp;
                std::atomic_int head{0};
        };

}
