#include "utils/ntstatus.hpp"
#include <map>

namespace nullsection::utils {

std::string NtStatusTranslator::ToString(NTSTATUS status) {
  static const std::map<NTSTATUS, std::string> statusMap = {
      {0x00000000, "STATUS_SUCCESS"},
      {0xC0000001, "STATUS_UNSUCCESSFUL"},
      {0xC0000002, "STATUS_NOT_IMPLEMENTED"},
      {0xC0000005, "STATUS_ACCESS_VIOLATION"},
      {0xC0000008, "STATUS_INVALID_HANDLE"},
      {0xC000000D, "STATUS_INVALID_PARAMETER"},
      {0xC0000018, "STATUS_CONFLICTING_ADDRESSES"},
      {0xC0000022, "STATUS_ACCESS_DENIED"},
      {0xC0000033, "STATUS_OBJECT_NAME_INVALID"},
      {0xC0000034, "STATUS_OBJECT_NAME_NOT_FOUND"},
      {0xC000003A, "STATUS_OBJECT_PATH_NOT_FOUND"},
      {0xC0000043, "STATUS_SHARING_VIOLATION"},
      {0xC00000BB, "STATUS_NOT_SUPPORTED"},
      {0xC0000142, "STATUS_DLL_INIT_FAILED"},
      {0xC0000225, "STATUS_NOT_FOUND"}};

  auto it = statusMap.find(status);
  if (it != statusMap.end()) {
    return it->second;
  }

  char buf[16];
  snprintf(buf, sizeof(buf), "0x%08X", status);
  return std::string("Unknown NTSTATUS (") + buf + ")";
}

} // namespace nullsection::utils
