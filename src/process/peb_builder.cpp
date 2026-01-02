#include "process/peb_builder.hpp"
#include "core/nt_api.hpp"
#include "core/object_manager.hpp"
#include "utils/exceptions.hpp"
#include "utils/logging.hpp"
#include <vector>

namespace nullsection::process {

// Minimal internal structures for research purposes
typedef struct _CURDIR {
  UNICODE_STRING DosPath;
  HANDLE Handle;
} CURDIR, *PCURDIR;

typedef struct _RTL_USER_PROCESS_PARAMETERS {
  ULONG MaximumLength;
  ULONG Length;
  ULONG Flags;
  ULONG DebugFlags;
  HANDLE ConsoleHandle;
  ULONG ConsoleFlags;
  HANDLE StandardInput;
  HANDLE StandardOutput;
  HANDLE StandardError;
  CURDIR CurrentDirectory;
  UNICODE_STRING DllPath;
  UNICODE_STRING ImagePathName;
  UNICODE_STRING CommandLine;
  PVOID Environment;
} RTL_USER_PROCESS_PARAMETERS, *PRTL_USER_PROCESS_PARAMETERS;

bool PebBuilder::PopulatePeb(HANDLE processHandle, PVOID pebAddress,
                             const std::wstring &imagePath,
                             const std::wstring &commandLine) {
  if (!processHandle || !pebAddress)
    return false;

  // 1. Initialize local UNICODE_STRINGs
  UNICODE_STRING usImagePath =
      core::ObjectManager::CreateUnicodeString(imagePath);
  UNICODE_STRING usCommandLine =
      core::ObjectManager::CreateUnicodeString(commandLine);

  PVOID remoteImagePathBuffer = nullptr;
  PVOID remoteCommandLineBuffer = nullptr;
  PVOID remoteParams = nullptr;

  try {
    // 2. Allocate remote buffers for strings
    SIZE_T imagePathSize = usImagePath.MaximumLength;
    SIZE_T cmdLineSize = usCommandLine.MaximumLength;

    NTSTATUS status = core::NtApi::Instance().NtAllocateVirtualMemory(
        processHandle, &remoteImagePathBuffer, 0, &imagePathSize,
        MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (status != 0)
      throw utils::NtException("Failed to allocate remote ImagePath buffer",
                               status);

    status = core::NtApi::Instance().NtAllocateVirtualMemory(
        processHandle, &remoteCommandLineBuffer, 0, &cmdLineSize,
        MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (status != 0)
      throw utils::NtException("Failed to allocate remote CommandLine buffer",
                               status);

    // 3. Write string data to remote buffers
    status = core::NtApi::Instance().NtWriteVirtualMemory(
        processHandle, remoteImagePathBuffer, usImagePath.Buffer,
        usImagePath.Length, nullptr);
    if (status != 0)
      throw utils::NtException("Failed to write remote ImagePath buffer",
                               status);

    status = core::NtApi::Instance().NtWriteVirtualMemory(
        processHandle, remoteCommandLineBuffer, usCommandLine.Buffer,
        usCommandLine.Length, nullptr);
    if (status != 0)
      throw utils::NtException("Failed to write remote CommandLine buffer",
                               status);

    // 4. Initialize Process Parameters with remote pointers
    RTL_USER_PROCESS_PARAMETERS params = {0};
    params.Length = sizeof(RTL_USER_PROCESS_PARAMETERS);
    params.MaximumLength = sizeof(RTL_USER_PROCESS_PARAMETERS);
    params.Flags = 1; // RTL_USER_PROC_PARAMS_NORMALIZED

    params.ImagePathName = usImagePath;
    params.ImagePathName.Buffer = (PWSTR)remoteImagePathBuffer;

    params.CommandLine = usCommandLine;
    params.CommandLine.Buffer = (PWSTR)remoteCommandLineBuffer;

    // 5. Allocate remote memory for the parameters structure
    SIZE_T paramsSize = sizeof(RTL_USER_PROCESS_PARAMETERS);
    status = core::NtApi::Instance().NtAllocateVirtualMemory(
        processHandle, &remoteParams, 0, &paramsSize, MEM_COMMIT | MEM_RESERVE,
        PAGE_READWRITE);
    if (status != 0)
      throw utils::NtException("Failed to allocate remote process parameters",
                               status);

    // 6. Write the parameters structure
    status = core::NtApi::Instance().NtWriteVirtualMemory(
        processHandle, remoteParams, &params, sizeof(params), nullptr);
    if (status != 0)
      throw utils::NtException("Failed to write process parameters", status);

    // 7. Update PEB's ProcessParameters pointer
    PVOID paramsPtrAddr = (PBYTE)pebAddress + 0x20; // Offset 0x20 on x64 PEB
    status = core::NtApi::Instance().NtWriteVirtualMemory(
        processHandle, paramsPtrAddr, &remoteParams, sizeof(PVOID), nullptr);
    if (status != 0)
      throw utils::NtException("Failed to update PEB ProcessParameters",
                               status);

  } catch (...) {
    // Basic cleanup on failure (Note: In a research framework, partial
    // allocations might be left for analysis)
    core::ObjectManager::FreeUnicodeString(usImagePath);
    core::ObjectManager::FreeUnicodeString(usCommandLine);
    throw;
  }

  core::ObjectManager::FreeUnicodeString(usImagePath);
  core::ObjectManager::FreeUnicodeString(usCommandLine);

  return true;
}

} // namespace nullsection::process
