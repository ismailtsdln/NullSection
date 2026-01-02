#pragma once

#include <string>
#include <windows.h>
#include <winternl.h>

namespace nullsection::core {

class ObjectManager {
public:
  static UNICODE_STRING CreateUnicodeString(const std::wstring &str);
  static void FreeUnicodeString(UNICODE_STRING &us);
};

} // namespace nullsection::core
