#pragma once

#include <string>
#include <windows.h>

namespace nullsection::process {

class PebBuilder {
public:
  static bool PopulatePeb(HANDLE processHandle, PVOID pebAddress,
                          const std::wstring &imagePath,
                          const std::wstring &commandLine);
};

} // namespace nullsection::process
