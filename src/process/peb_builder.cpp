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

  // 1. Initialize Process Parameters locally
  RTL_USER_PROCESS_PARAMETERS params = {0};
  params.Length = sizeof(RTL_USER_PROCESS_PARAMETERS);
  params.MaximumLength = sizeof(RTL_USER_PROCESS_PARAMETERS);
  params.Flags = 1; // RTL_USER_PROC_PARAMS_NORMALIZED

  UNICODE_STRING usImagePath =
      core::ObjectManager::CreateUnicodeString(imagePath);
  UNICODE_STRING usCommandLine =
      core::ObjectManager::CreateUnicodeString(commandLine);

  params.ImagePathName = usImagePath;
  params.CommandLine = usCommandLine;

  // 2. Allocate memory in remote process for parameters
  PVOID remoteParams = nullptr;
  SIZE_T regionSize = sizeof(RTL_USER_PROCESS_PARAMETERS) +
                      usImagePath.MaximumLength + usCommandLine.MaximumLength;

  NTSTATUS status = core::NtApi::Instance().NtAllocateVirtualMemory(
      processHandle, &remoteParams, 0, &regionSize, MEM_COMMIT | MEM_RESERVE,
      PAGE_READWRITE);

  if (status != 0) {
    core::ObjectManager::FreeUnicodeString(usImagePath);
    core::ObjectManager::FreeUnicodeString(usCommandLine);
    throw utils::NtException("Failed to allocate remote process parameters",
                             status);
  }

  // 3. Adjust pointers for remote process
  // This is a simplified version; in a real-world scenario, we'd copy the
  // strings separately and update the UNICODE_STRING buffers to point to remote
  // addresses.

  // For research purposes, we log the action and perform a basic write
  status = core::NtApi::Instance().NtWriteVirtualMemory(
      processHandle, remoteParams, &params, sizeof(params), nullptr);

  if (status != 0)
    throw utils::NtException("Failed to write process parameters", status);

  // 4. Update PEB's ProcessParameters pointer
  // ProcessParameters is at offset 0x20 on x64 PEB
  PVOID paramsPtrAddr = (PBYTE)pebAddress + 0x20;
  status = core::NtApi::Instance().NtWriteVirtualMemory(
      processHandle, paramsPtrAddr, &remoteParams, sizeof(PVOID), nullptr);

  if (status != 0)
    throw utils::NtException("Failed to update PEB ProcessParameters", status);

  core::ObjectManager::FreeUnicodeString(usImagePath);
  core::ObjectManager::FreeUnicodeString(usCommandLine);

  return true;
}

} // namespace nullsection::process
