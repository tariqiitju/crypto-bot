#ifndef _FILE_IO_H
#define _FILE_IO_H

#include "spdlog/spdlog.h"
#include <filesystem>
#include <fstream>
#include <string>

class fileutils{
public:
static int create_file(const std::filesystem::path &directory,const std::string &file_name){
    spdlog::debug("trying to create file");
    spdlog::debug("creating file {} in direcory {}",file_name, directory.generic_string());
    if(!std::filesystem::exists(directory)){
        spdlog::error("file creation erros. {} directory does not exist.", directory.generic_string().c_str() );
        return -1;
    }
    if(! std::filesystem::is_directory(directory)){
        spdlog::error("file creation erros. {} is not a directory.", directory.generic_string().c_str());
        return -2;
    }

    std::string file_path(directory.c_str());

    file_path += "/" + file_name;


    if(std::filesystem::exists(file_path)){
        spdlog::error("file creation erros. {} already exist.", file_path);
        return -3;
    }

    try {
        std::ofstream file(file_path,std::ios::binary | std::ios::out);
        file.close();
    } catch (std::exception &ex){
        spdlog::error("execption while creating {1}. message: {2}",file_path, ex.what());
        return -4;
    }
    return 0;
}


static std::string read_string_from_file(std::ifstream &file){
    // Assuming string preceeded by 1 byte size parameter
    if(!file.is_open()){
        spdlog::error("Error reading from file. file is not opened");
        return nullptr;
    }
    if(!file.good()){
        spdlog::error("Error reading from file. file is not good condition");
        return nullptr;
    }
    char sz;
    file.read(&sz,sizeof(char));
    char buff[sz + 1];
    file.read(buff,sz);
    buff[sz] = 0;
    std::string ret(buff);
    return ret;
}

static int write_string_to_file(std::ofstream &file,const std::string &s){
    // Assuming string preceeded by 1 byte size parameter
    if(!file.is_open()){
        spdlog::error("Error reading from file. file is not opened");
        return -1;
    }
    if(!file.good()){
        spdlog::error("Error reading from file. file is not in good condition");
        return -2;
    }
    char sz = s.size();
    try{
        file.write(&sz,sizeof(char));
        file.write(s.c_str(),s.size());
    } catch (std::exception &ex){
        spdlog::error("error while writing to file with exception {}",ex.what());
        return -1;
    }
    return 0;
}
static uint64_t read_uint64(std::ifstream &file){
    if(!file.is_open()){
        spdlog::error("Error reading from file. file is not opened");
        return 0;
    }
    if(!file.good()){
        spdlog::error("Error reading from file. file is not good condition");
        return 0;
    }
    uint64_t value;
    file.read( (char *) &value, sizeof(uint64_t));
    return value;
}


static int write_uint64(std::ofstream &file, uint64_t num){
    if(!file.is_open()){
        spdlog::error("Error reading from file. file is not opened");
        return -1;
    }
    if(!file.good()){
        spdlog::error("Error reading from file. file is not good condition");
        return -2;
    }
    file.write( (char *) &num, sizeof(uint64_t));
    return 0;
}





static double read_double(std::ifstream &file){
    if(!file.is_open()){
        spdlog::error("Error reading from file. file is not opened");
        return 0;
    }
    if(!file.good()){
        spdlog::error("Error reading from file. file is not good condition");
        return 0;
    }
    double value;
    file.read( (char *) &value, sizeof(double));
    return value;
}
static int write_double(std::ofstream &file, double num){
    if(!file.is_open()){
        spdlog::error("Error reading from file. file is not opened");
        return -1;
    }
    if(!file.good()){
        spdlog::error("Error reading from file. file is not good condition");
        return -2;
    }
    file.write( (char *) &num, sizeof(double));
    return 0;
}

};


#endif
