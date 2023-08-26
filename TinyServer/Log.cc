#include"Log.hpp"


void Log(const std::string level,const std::string message,std::string file,int line)
{
  std::cout<<"["<<level<<"]["<<time(nullptr)<<"]["<<message<<"]["<<file<<"]["<<line<<"]"<<std::endl;
}
