#include "image/section_builder.hpp"
#include "core/nt_api.hpp"
#include "utils/exceptions.hpp"
#include "utils/logging.hpp"
#include "utils/ntstatus.hpp"
#include <filesystem>

namespace nullsection::image {

utils::Handle
SectionBuilder::CreateTransientSection(const std::wstring &filePath) {
  if (filePath.empty()) {
    throw utils::NullSectionException("File path cannot be empty.");
  }

  if (!std::filesystem::exists(filePath)) {
    throw utils::NullSectionException(
        "Target image file does not exist: " +
        std::string(filePath.begin(), filePath.end()));
  }

  // 1. Open file with FILE_SHARE_DELETE to allow logical removal
  HANDLE hFile = CreateFileW(filePath.c_str(), GENERIC_READ | GENERIC_EXECUTE,
                             FILE_SHARE_READ | FILE_SHARE_DELETE, nullptr,
                             OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

  if (hFile == INVALID_HANDLE_VALUE) {
    throw utils::Win32Exception("Failed to open file for section creation",
                                GetLastError());
  }
  utils::Handle fileHandle(hFile);

  // 2. Create image section using NtCreateSection
  HANDLE hSection = nullptr;
  NTSTATUS status = core::NtApi::Instance().NtCreateSection(
      &hSection, SECTION_ALL_ACCESS, nullptr, nullptr, PAGE_READONLY, SEC_IMAGE,
      fileHandle);

  if (status != 0) { // STATUS_SUCCESS (0)
    std::string ntStatusStr = utils::NtStatusTranslator::ToString(status);
    utils::Logger::Log(utils::LogLevel::ERROR,
                       "NtCreateSection failed: " + ntStatusStr);
    throw utils::NtException("NtCreateSection failed", status);
  }

  return utils::Handle(hSection);
}

} // namespace nullsection::image
