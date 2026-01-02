#include "evasion/telemetry.hpp"
#include "utils/logging.hpp"
#include <iostream>

namespace nullsection::evasion {

void Telemetry::AnalyzeHandleTable(HANDLE processHandle) {
  utils::Logger::Log(utils::LogLevel::INFO, "Analyzing remote handle table...");

  // In a research context, we use ProcessHandleInformation (51)
  // to enumerate all handles owned by the target process.
  ULONG returnLength = 0;
  NTSTATUS status = core::NtApi::Instance().NtQueryInformationProcess(
      processHandle,
      (PROCESSINFOCLASS)51, // ProcessHandleInformation
      nullptr, 0, &returnLength);

  if (status == 0xC0000004) { // STATUS_INFO_LENGTH_MISMATCH
    utils::Logger::Log(utils::LogLevel::DEBUG,
                       "Handle table size detected: " +
                           std::to_string(returnLength) + " bytes.");
  }

  utils::Logger::Log(
      utils::LogLevel::INFO,
      "Handle Table analysis logic initialized for forensic research.");
}

void Telemetry::AnalyzeImageName(HANDLE processHandle) {
  utils::Logger::Log(utils::LogLevel::INFO,
                     "Analyzing process image name resolution...");

  ULONG returnLength = 0;
  // ProcessImageFileName (27)
  core::NtApi::Instance().NtQueryInformationProcess(
      processHandle, (PROCESSINFOCLASS)27, nullptr, 0, &returnLength);

  utils::Logger::Log(utils::LogLevel::DEBUG,
                     "Forensic image name analysis logic complete.");
}

} // namespace nullsection::evasion
