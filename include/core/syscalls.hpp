#pragma once

#include <windows.h>
#include <winternl.h>

namespace nullsection::core {

// Research reference for indirect syscalls
// In a full implementation, this would involve shellcode to resolve SSNs
// and execute 'syscall' instructions.
class Syscalls {
public:
  static NTSTATUS IndirectSyscall(DWORD ssn, ...);
};

} // namespace nullsection::core
