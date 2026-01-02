#include "core/object_manager.hpp"
#include <vector>

namespace nullsection::core {

UNICODE_STRING ObjectManager::CreateUnicodeString(const std::wstring &str) {
  UNICODE_STRING us = {0};
  us.Length = static_cast<USHORT>(str.length() * sizeof(wchar_t));
  us.MaximumLength = us.Length + sizeof(wchar_t);
  us.Buffer = new wchar_t[us.MaximumLength / sizeof(wchar_t)];
  memcpy(us.Buffer, str.c_str(), us.Length);
  us.Buffer[us.Length / sizeof(wchar_t)] = L'\0';
  return us;
}

void ObjectManager::FreeUnicodeString(UNICODE_STRING &us) {
  if (us.Buffer) {
    delete[] us.Buffer;
    us.Buffer = nullptr;
    us.Length = 0;
    us.MaximumLength = 0;
  }
}

} // namespace nullsection::core
