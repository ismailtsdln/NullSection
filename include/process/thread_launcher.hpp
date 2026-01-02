#pragma once

#include "utils/handle.hpp"
#include <windows.h>

namespace nullsection::process {

class ThreadLauncher {
public:
  static utils::Handle LaunchMainThread(HANDLE processHandle, PVOID entryPoint,
                                        PVOID parameter = nullptr);
};

} // namespace nullsection::process
