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
    std::cout << "Usage: nullsection.exe run --image <payload.exe>"
              << std::endl;
    return 1;
  }

  std::string command = argv[1];
  if (command == "run") {
    std::wstring imagePath;
    for (int i = 2; i < argc; ++i) {
      std::string arg = argv[i];
      if (arg == "--image" && i + 1 < argc) {
        std::string pathStr = argv[++i];
        imagePath = std::wstring(pathStr.begin(), pathStr.end());
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

      // 3. Create transient section
      utils::Logger::Log(utils::LogLevel::INFO,
                         "Creating transient section for: " +
                             std::string(imagePath.begin(), imagePath.end()));
      utils::Handle hSection =
          image::SectionBuilder::CreateTransientSection(imagePath);
      utils::Logger::Log(utils::LogLevel::DEBUG,
                         "Section created successfully.");

      // 4. Create process from section
      utils::Logger::Log(utils::LogLevel::INFO,
                         "Creating process object via NtCreateProcessEx.");
      utils::Handle hProcess =
          process::ProcessFactory::CreateProcessFromSection(hSection);
      utils::Logger::Log(utils::LogLevel::DEBUG,
                         "Process created successfully.");

      // 5. Resolve absolute entry point
      PVOID remoteBase = process::ProcessFactory::GetRemoteImageBase(hProcess);
      PVOID absoluteEntryPoint = (PBYTE)remoteBase + entryPointRva;
      utils::Logger::Log(utils::LogLevel::INFO,
                         "Resolved absolute entry point.");

      // 6. Launch thread
      utils::Logger::Log(utils::LogLevel::INFO,
                         "Launching main thread via NtCreateThreadEx.");
      utils::Handle hThread = process::ThreadLauncher::LaunchMainThread(
          hProcess, absoluteEntryPoint);

      utils::Logger::Log(utils::LogLevel::INFO,
                         "Execution successful. Process is running.");

    } catch (const utils::NtException &e) {
      utils::Logger::Log(utils::LogLevel::ERROR,
                         std::string("NT API Error: ") + e.what());
      return 1;
    } catch (const utils::Win32Exception &e) {
      utils::Logger::Log(utils::LogLevel::ERROR,
                         std::string("Win32 Error: ") + e.what());
      return 1;
    } catch (const utils::NullSectionException &e) {
      utils::Logger::Log(utils::LogLevel::ERROR,
                         std::string("Framework Error: ") + e.what());
      return 1;
    } catch (const std::exception &e) {
      utils::Logger::Log(utils::LogLevel::ERROR,
                         std::string("Unexpected error: ") + e.what());
      return 1;
    }
  } else {
    std::cout << "Unknown command: " << command << std::endl;
    return 1;
  }

  return 0;
}
