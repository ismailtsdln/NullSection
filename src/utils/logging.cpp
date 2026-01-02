#include "utils/logging.hpp"
#include <iostream>

namespace nullsection::utils {

void Logger::Log(LogLevel level, const std::string &message) {
  std::string prefix;
  switch (level) {
  case LogLevel::DEBUG:
    prefix = "[DEBUG] ";
    break;
  case LogLevel::INFO:
    prefix = "[INFO]  ";
    break;
  case LogLevel::WARN:
    prefix = "[WARN]  ";
    break;
  case LogLevel::ERROR:
    prefix = "[ERROR] ";
    break;
  }
  std::cout << prefix << message << std::endl;
}

} // namespace nullsection::utils
