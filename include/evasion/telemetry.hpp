#pragma once

#include <string>
#include <windows.h>

namespace nullsection::evasion {

class Telemetry {
public:
  static void AnalyzeHandleTable(HANDLE processHandle);
  static void AnalyzeImageName(HANDLE processHandle);
};

} // namespace nullsection::evasion
