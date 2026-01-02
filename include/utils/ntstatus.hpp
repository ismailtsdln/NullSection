#pragma once

#include <string>
#include <windows.h>

namespace nullsection::utils {

class NtStatusTranslator {
public:
  static std::string ToString(NTSTATUS status);
};

} // namespace nullsection::utils
