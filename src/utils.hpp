#pragma once

#ifndef UTILS_HPP
#define UTILS_HPP

#include <iostream>
#include <mutex>
#include <filesystem>
#include <fstream>
#include <string>
#include <cstring>

namespace  fs = std::filesystem;

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
#define log_one(name) log(#name,name)
#define  log_noendl(...) debug_out_noendl(std::cout,__FILE__,"Line:",__LINE__,":: ",__VA_ARGS__)
#define  log_raw(...) debug_out_noendl(std::cout,__VA_ARGS__)


//写文件
bool saveFile(std::string_view content,const fs::path & file_path);

std::string readFile( const fs::path& path);

/*base_64.h文件*/
#ifndef BASE_64_H
#define BASE_64_H
/**
 * Base64 编码/解码
 * @author liruixing
 */
class Base64{
private:
    std::string _base64_table;
    static const char base64_pad = '=';
public:
    Base64()
    {
        _base64_table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"; /*这是Base64编码使用的标准字典*/
    }
    /**
     * 这里必须是unsigned类型，否则编码中文的时候出错
     */
    std::string Encode(const char * str,int bytes);
    std::string Decode(const char *str,int bytes);
    void Debug(bool open = true);
};



#endif


#endif
