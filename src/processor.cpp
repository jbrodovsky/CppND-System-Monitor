#include "processor.h"

#include <iostream>

// TODO: Return the aggregate CPU utilization
float Processor::Utilization() {
  float total_time = LinuxParser::Jiffies();
  float idle_time = LinuxParser::IdleJiffies();
  return (total_time - idle_time) / total_time;
}