#pragma once

#include <string>
#include <windows.h>
#include <winternl.h>

namespace nullsection::core {

// Forward declarations of undocumented NT types if needed
typedef struct _OBJECT_ATTRIBUTES {
  ULONG Length;
  HANDLE RootDirectory;
  PUNICODE_STRING ObjectName;
  ULONG Attributes;
  PVOID SecurityDescriptor;
  PVOID SecurityQualityOfService;
} OBJECT_ATTRIBUTES, *POBJECT_ATTRIBUTES;

#ifndef InitializeObjectAttributes
#define InitializeObjectAttributes(p, n, a, r, s)                              \
  {                                                                            \
    (p)->Length = sizeof(OBJECT_ATTRIBUTES);                                   \
    (p)->RootDirectory = r;                                                    \
    (p)->Attributes = a;                                                       \
    (p)->ObjectName = n;                                                       \
    (p)->SecurityDescriptor = s;                                               \
    (p)->SecurityQualityOfService = NULL;                                      \
  }
#endif

// Function pointer types for NT APIs
using pNtCreateSection = NTSTATUS(NTAPI *)(
    PHANDLE SectionHandle, ACCESS_MASK DesiredAccess,
    POBJECT_ATTRIBUTES ObjectAttributes, PLARGE_INTEGER MaximumSize,
    ULONG SectionPageProtection, ULONG AllocationAttributes, HANDLE FileHandle);

using pNtCreateProcessEx =
    NTSTATUS(NTAPI *)(PHANDLE ProcessHandle, ACCESS_MASK DesiredAccess,
                      POBJECT_ATTRIBUTES ObjectAttributes, HANDLE ParentProcess,
                      ULONG Flags, HANDLE SectionHandle, HANDLE DebugPort,
                      HANDLE ExceptionPort, ULONG JobMemberLevel);

using pNtCreateThreadEx = NTSTATUS(NTAPI *)(
    PHANDLE ThreadHandle, ACCESS_MASK DesiredAccess,
    POBJECT_ATTRIBUTES ObjectAttributes, HANDLE ProcessHandle,
    PVOID StartRoutine, PVOID Argument, ULONG CreateFlags, ULONG_PTR ZeroBits,
    SIZE_T StackSize, SIZE_T MaximumStackSize, PVOID AttributeList);

using pNtQueryInformationProcess = NTSTATUS(NTAPI *)(
    HANDLE ProcessHandle, PROCESSINFOCLASS ProcessInformationClass,
    PVOID ProcessInformation, ULONG ProcessInformationLength,
    PULONG ReturnLength);

using pNtReadVirtualMemory = NTSTATUS(NTAPI *)(HANDLE ProcessHandle,
                                               PVOID BaseAddress, PVOID Buffer,
                                               SIZE_T BufferSize,
                                               PSIZE_T NumberOfBytesRead);

using pNtWriteVirtualMemory = NTSTATUS(NTAPI *)(HANDLE ProcessHandle,
                                                PVOID BaseAddress, PVOID Buffer,
                                                SIZE_T BufferSize,
                                                PSIZE_T NumberOfBytesWritten);

using pNtAllocateVirtualMemory = NTSTATUS(NTAPI *)(
    HANDLE ProcessHandle, PVOID *BaseAddress, ULONG_PTR ZeroBits,
    PSIZE_T RegionSize, ULONG AllocationType, ULONG Protect);

class NtApi {
public:
  static NtApi &Instance();

  NtApi(const NtApi &) = delete;
  NtApi &operator=(const NtApi &) = delete;

  // Resolved functions
  pNtCreateSection NtCreateSection = nullptr;
  pNtCreateProcessEx NtCreateProcessEx = nullptr;
  pNtCreateThreadEx NtCreateThreadEx = nullptr;
  pNtQueryInformationProcess NtQueryInformationProcess = nullptr;
  pNtReadVirtualMemory NtReadVirtualMemory = nullptr;
  pNtWriteVirtualMemory NtWriteVirtualMemory = nullptr;
  pNtAllocateVirtualMemory NtAllocateVirtualMemory = nullptr;

private:
  NtApi();
  void ResolveFunctions();
  HMODULE m_ntdll = nullptr;
};

} // namespace nullsection::core
