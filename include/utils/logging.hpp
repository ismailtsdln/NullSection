#pragma once

#include <iostream>
#include <string>

namespace nullsection::utils {

enum class LogLevel { DEBUG, INFO, WARN, ERROR };

class Logger {
public:
  static void Log(LogLevel level, const std::string &message);
};

} // namespace nullsection::utils
