#include "linux_parser.h"

#include <dirent.h>
#include <unistd.h>

#include <sstream>
#include <string>
#include <vector>

using std::stof;
using std::stol;
using std::string;
using std::to_string;
using std::vector;

// An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, kernel, version;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
// Standard linux gcc doesn't support std::filesystem on ubuntu yet (introduced
// in c++17)
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

// Read and return the system memory utilization; returns a float that is
// the percentage of memory used
float LinuxParser::MemoryUtilization() {
  float mem_total = FindValueByKey<float>("MemTotal:", kMeminfoFilename);
  float mem_free = FindValueByKey<float>("MemFree:", kMeminfoFilename);
  return (mem_total - mem_free) / mem_total;
}

// Read and return the system uptime
long LinuxParser::UpTime() {
  string line, uptime, idle_time;
  std::ifstream stream(kProcDirectory + kUptimeFilename);
  if (stream.is_open() && std::getline(stream, line)) {
    std::istringstream linestream(line);
    linestream >> uptime;
    try {
      return stol(uptime);
    } catch (const std::invalid_argument& arg) {
      return 0.0;
    }
  }
  return 0.0;
}

// Read and return the number of jiffies for the system
long LinuxParser::Jiffies() {
  long total = 0;
  for (std::string s : LinuxParser::CpuUtilization()) {
    total += stol(s);
  }
  return total;
}

// Read and return the number of active jiffies for a PID
long LinuxParser::ActiveJiffies(int pid [[maybe_unused]]) { return 0; }

// Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() {
  long active = 0;
  vector<std::string> s = LinuxParser::CpuUtilization();
  active += stol(s[0]);
  active += stol(s[1]);
  return active;
}

// Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() {
  long idle = 0;
  vector<std::string> s = LinuxParser::CpuUtilization();
  idle += stol(s[3]);
  idle += stol(s[4]);
  return idle;
}

// Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() {
  vector<std::string> CPU_Util;
  std::string line, key, user, nice, sys, idle, iowait, irq, softirq, steal,
      guest, guest_nice;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open() && std::getline(stream, line)) {
    std::istringstream linestream(line);
    // std::cout<<"LINE: "<<line<<"\n";
    linestream >> key >> user >> nice >> sys >> idle >> iowait >> irq >>
        softirq >> steal >> guest >> guest_nice;
    CPU_Util.push_back(user);
    CPU_Util.push_back(nice);
    CPU_Util.push_back(sys);
    CPU_Util.push_back(idle);
    CPU_Util.push_back(guest_nice);
  }
  return CPU_Util;
}

// Returns the process's CPU utilization as a percent of total CPU activity
float LinuxParser::CpuUtilization(int pid) {
  long uptime = LinuxParser::UpTime();
  // see for explaination:
  // https://stackoverflow.com/questions/16726779/how-do-i-get-the-total-cpu-usage-of-an-application-from-proc-pid-stat
  long utime = 0;
  long stime = 0;
  long cutime = 0;
  long cstime = 0;
  long starttime = 0;
  string line;
  std::ifstream stream(kProcDirectory + "/" + to_string(pid) + "/" +
                       kStatFilename);
  if (stream.is_open() && std::getline(stream, line)) {
    std::istringstream linestream(line);
    try {
      string value;
      for (int i = 1; i < 23; i++) {
        linestream >> value;

        switch (i) {
          case 14:
            utime = stol(value);
            break;
          case 15:
            stime = stol(value);
            break;
          case 16:
            cutime = stol(value);
            break;
          case 17:
            cstime = stol(value);
            break;
          case 22:
            starttime = stol(value);
            break;
          default:
            break;
        }
      }
    } catch (const std::invalid_argument& arg) {
      // The above block usually works just fine, but occasionally a stol(...)
      // call will throw an exception somewhere and this is the only place I can
      // narrow it down to.
    }
  }

  long total_time = utime + stime + cutime + cstime;
  float seconds = uptime - (starttime / sysconf(_SC_CLK_TCK));
  return (total_time / sysconf(_SC_CLK_TCK)) / seconds;
}
// Read and return the total number of processes
int LinuxParser::TotalProcesses() {
  return FindValueByKey<int>("processes", kStatFilename);
}

// Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  return FindValueByKey<int>("procs_running", kStatFilename);
}

// Read and return the command associated with a process
string LinuxParser::Command(int pid) {
  string line = "";
  std::ifstream stream(kProcDirectory + "/" + to_string(pid) +
                       kCmdlineFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    return line;
  }
  return line;
}

// Read and return the memory used by a process
string LinuxParser::Ram(int pid) {
  return FindValueByKey<string>("VmData:",
                                "/" + to_string(pid) + kStatusFilename);
  // Switched to VmData as VmSize returns virtual memory not the physical RAM on
  // my machine
}

// Read and return the user ID associated with a process
string LinuxParser::Uid(int pid) {
  return FindValueByKey<string>("Uid:", "/" + to_string(pid) + kStatusFilename);
}

// Read and return the user associated with a process
string LinuxParser::User(int pid) {
  string uid = Uid(pid);
  string line, key, other;
  string value = "";
  std::ifstream stream(kPasswordPath);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      while (linestream >> value >> other >> key) {
        if (key == uid) {
          return value;
        }
      }
    }
  }
  return value;
}

// TODO: Read and return the uptime of a process
long LinuxParser::UpTime(int pid) {
  // see for explaination:
  // https://stackoverflow.com/questions/16726779/how-do-i-get-the-total-cpu-usage-of-an-application-from-proc-pid-stat
  string line;
  std::ifstream stream(kProcDirectory + "/" + to_string(pid) + "/" +
                       kStatFilename);
  if (stream.is_open() && std::getline(stream, line)) {
    // std::istringstream linestream(line);
    auto utime = line.at(13);
    auto stime = line.at(14);
    auto cutime = line.at(15);
    auto cstime = line.at(16);
    // auto starttime = line.at(22);
    return utime + stime + cutime + cstime;
  }
  stream.close();
  return 0;
}

// Generic function for repetedly used filestream searching. Finds the value
// stored for the string key_filter in the file specified by filename under the
// /proc/ directory
template <typename T>
T LinuxParser::FindValueByKey(std::string const& key_filter,
                              std::string const& filename) {
  std::string line, key;
  T value;

  std::ifstream stream(kProcDirectory + filename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == key_filter) {
          return value;
        }
      }
    }
  }
  return value;
}