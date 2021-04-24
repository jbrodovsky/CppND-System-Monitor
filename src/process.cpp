#include "process.h"

#include <unistd.h>

#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "linux_parser.h"

using std::string;
using std::to_string;
using std::vector;

// Construct a process from a given process id
Process::Process(int pid) : process_id_(pid) {
  Command();
  Ram();
  User();
  UpTime();
}

// DONE: Return this process's ID
int Process::Pid() { return process_id_; }

// DONE: Return this process's CPU utilization
float Process::CpuUtilization() { 
  //long uptime = LinuxParser::UpTime();
  //vector<float> values = LinuxParser::CpuUtilization(Pid());
  //cpu_usage_ = LinuxParser::ActiveJiffies(Pid()) / LinuxParser::CpuUtilization();
  
  cpu_usage_ = LinuxParser::ActiveJiffies(Pid());// / (LinuxParser::Jiffies() - LinuxParser::IdleJiffies());
  return cpu_usage_; 
}

// DONE: Return the command that generated this process
string Process::Command() {
  command_ = LinuxParser::Command(process_id_);
  if (command_.length() <= 50) {
    return command_;
  } else {
    return command_.substr(0, 46) + "...";
  }
}

// DONE: Return this process's memory utilization
string Process::Ram() {
  float ram_f_ = std::stof(LinuxParser::Ram(process_id_))*0.001;
  ram_ = std::to_string(ram_f_);
  ram_ = ram_.substr(0, 5);
  return ram_;
}

// DONE: Return the user (name) that generated this process
string Process::User() {
  user_ = LinuxParser::User(Pid());
  if(user_.length()<6){
    return user_;
  }
  else{
    return user_.substr(0,4)+"...";
  }
}

// DONE: Return the age of this process (in seconds)
long int Process::UpTime() { return LinuxParser::UpTime(Pid()); }

// TODO: Overload the "less than" comparison operator for Process objects
// REMOVE: [[maybe_unused]] once you define the function
bool Process::operator<(Process const& a) const {
  return cpu_usage_ < a.cpu_usage_;
}