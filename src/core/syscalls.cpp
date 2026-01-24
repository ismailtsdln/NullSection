#include "core/syscalls.hpp"
#include "utils/logging.hpp"
#include <cstdarg>

namespace nullsection::core {

// Assembly stub for performing direct syscalls
// This is a simplified version; in production, use a full stub like SysWhispers
extern "C" NTSTATUS DoSyscall(DWORD ssn, va_list args);

NTSTATUS Syscalls::IndirectSyscall(DWORD ssn, ...) {
  va_list args;
  va_start(args, ssn);
  NTSTATUS result = DoSyscall(ssn, args);
  va_end(args);
  utils::Logger::Log(utils::LogLevel::INFO,
                     "Syscall executed with SSN: " + std::to_string(ssn));
  return result;
}

} // namespace nullsection::core
