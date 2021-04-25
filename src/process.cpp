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

// Return this process's ID
int Process::Pid() { return process_id_; }

// Return this process's CPU utilization
float Process::CpuUtilization() {
  cpu_usage_ = LinuxParser::CpuUtilization(Pid());
  return cpu_usage_;
}

// Return the command that generated this process
string Process::Command() {
  command_ = LinuxParser::Command(process_id_);
  if (command_.length() <= 50) {
    return command_;
  } else {
    return command_.substr(0, 46) + "...";
  }
}

// Return this process's memory utilization
string Process::Ram() {
  float ram_f_ = std::stof(LinuxParser::Ram(process_id_)) * 0.001;
  ram_ = std::to_string(ram_f_);
  ram_ = ram_.substr(0, 5);
  return ram_;
}

// Return the user (name) that generated this process
string Process::User() {
  user_ = LinuxParser::User(Pid());
  if (user_.length() < 6) {
    return user_;
  } else {
    return user_.substr(0, 4) + "...";
  }
}

// Return the age of this process (in seconds)
long int Process::UpTime() {
  up_time_ = LinuxParser::UpTime() - LinuxParser::UpTime(Pid());
  return up_time_;
}

// Overload the "less than" comparison operator for Process objects
bool Process::operator<(Process const& a) const {
  return cpu_usage_ < a.cpu_usage_;
}