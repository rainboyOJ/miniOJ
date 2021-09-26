#pragma once

#ifndef UTILS_HPP
#define UTILS_HPP

#include <iostream>
#include <mutex>
extern std::mutex g_log_mutex;  //
template<char Delimiter = ' ',typename... Args>
void debug_out(std::ostream &os, Args&&... args){
    //std::lock_guard<std::mutex> lock(g_log_mutex);
    ( (os << args << Delimiter),... ) <<std::endl;
}

template<char Delimiter = '\0',typename... Args>
void debug_out_noendl(std::ostream &os, Args&&... args){
    //std::lock_guard<std::mutex> lock(g_log_mutex);
    ( (os << args << Delimiter),... );
}

#define  log(...) debug_out(std::cout,__FILE__,"Line:",__LINE__,":: ",__VA_ARGS__)
#define  log_noendl(...) debug_out_noendl(std::cout,__FILE__,"Line:",__LINE__,":: ",__VA_ARGS__)
#define  log_raw(...) debug_out_noendl(std::cout,__VA_ARGS__)


#endif
