#pragma once

#include "utils/handle.hpp"
#include <windows.h>

namespace nullsection::process {

class ProcessFactory {
public:
  static utils::Handle CreateProcessFromSection(HANDLE sectionHandle,
                                                HANDLE parentProcess = nullptr);
  static PVOID GetRemoteImageBase(HANDLE processHandle);
};

} // namespace nullsection::process
