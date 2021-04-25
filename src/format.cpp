#include "format.h"

#include <chrono>
#include <iomanip>
#include <sstream>
#include <string>

using std::string;

// Formats a long integer corresponding to a duration in seconds to HH:MM:SS
// using the chrono module from the standard library
string Format::ElapsedTime(long seconds) {
  std::chrono::seconds SS{seconds};
  std::chrono::hours HH = std::chrono::duration_cast<std::chrono::hours>(SS);
  SS -= std::chrono::duration_cast<std::chrono::seconds>(HH);
  std::chrono::minutes MM =
      std::chrono::duration_cast<std::chrono::minutes>(SS);
  SS -= std::chrono::duration_cast<std::chrono::seconds>(MM);

  std::stringstream out{};
  out << std::setw(2) << std::setfill('0') << HH.count() << ":" << std::setw(2)
      << std::setfill('0') << MM.count() << ":" << std::setw(2)
      << std::setfill('0') << SS.count();

  return out.str();
}