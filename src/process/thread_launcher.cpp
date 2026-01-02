#include "process/thread_launcher.hpp"
#include "core/nt_api.hpp"
#include "utils/exceptions.hpp"
#include "utils/logging.hpp"
#include "utils/ntstatus.hpp"
#include <stdexcept>

namespace nullsection::process {

utils::Handle ThreadLauncher::LaunchMainThread(HANDLE processHandle,
                                               PVOID entryPoint,
                                               PVOID parameter) {
  if (!processHandle || processHandle == INVALID_HANDLE_VALUE) {
    throw utils::NullSectionException(
        "Invalid process handle provided to ThreadLauncher.");
  }

  if (!entryPoint) {
    throw utils::NullSectionException(
        "Null entry point provided to ThreadLauncher.");
  }

  HANDLE hThread = nullptr;

  NTSTATUS status = core::NtApi::Instance().NtCreateThreadEx(
      &hThread, THREAD_ALL_ACCESS, nullptr, processHandle, entryPoint,
      parameter, 0, 0, 0, 0, nullptr);

  if (status != 0) {
    std::string ntStatusStr = utils::NtStatusTranslator::ToString(status);
    utils::Logger::Log(utils::LogLevel::ERROR,
                       "NtCreateThreadEx failed: " + ntStatusStr);
    throw utils::NtException("NtCreateThreadEx failed", status);
  }

  if (!hThread) {
    throw utils::NullSectionException(
        "NtCreateThreadEx returned success but thread handle is null.");
  }

  return utils::Handle(hThread);
}

} // namespace nullsection::process
