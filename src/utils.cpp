#include "utils.hpp"

std::mutex g_log_mutex;  //

bool saveFile(std::string_view content,const fs::path & file_path){
    //TODO 检测 文件是否存在 同名但不是文件夹
    //创建对应的文件夹
    fs::create_directories(file_path.parent_path());
    std::ofstream f(file_path);
    f<< content;
    f.close();
    return true;
}

std::string readFile(const char * path){
    std::stringstream ss;
    std::ifstream f(path);
    ss << f.rdbuf();
    return ss.str();
}
