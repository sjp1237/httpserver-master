#pragma once
#include<iostream>
#include<string>
#include<sys/types.h>
#include<sys/socket.h>

namespace Tool{
bool ReadLine(int fd,std::string& outstring);
bool CuttingString(std::string& s,std::string& s1,std::string& s2,std::string& seq);
};
