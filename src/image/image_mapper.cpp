#include "image/image_mapper.hpp"
#include "core/nt_api.hpp"
#include "utils/exceptions.hpp"
#include "utils/logging.hpp"

namespace nullsection::image {

PVOID ImageMapper::MapImageManually(HANDLE processHandle,
                                    const PeHeaders &headers,
                                    void *localBuffer) {
  utils::Logger::Log(utils::LogLevel::INFO,
                     "Image Mapper: Starting manual mapping research flow.");

  PVOID remoteBase = (PVOID)headers.nt_headers->OptionalHeader.ImageBase;
  SIZE_T imageSize = headers.nt_headers->OptionalHeader.SizeOfImage;

  // 1. Allocate remote memory
  NTSTATUS status = core::NtApi::Instance().NtAllocateVirtualMemory(
      processHandle, &remoteBase, 0, &imageSize, MEM_COMMIT | MEM_RESERVE,
      PAGE_EXECUTE_READWRITE // Simplified for research; real implementation
                             // would use section protections
  );

  if (status != 0) {
    // Try at any address if preferred base is busy
    remoteBase = nullptr;
    status = core::NtApi::Instance().NtAllocateVirtualMemory(
        processHandle, &remoteBase, 0, &imageSize, MEM_COMMIT | MEM_RESERVE,
        PAGE_EXECUTE_READWRITE);
    if (status != 0)
      throw utils::NtException(
          "Failed to allocate remote memory for manual mapping", status);
  }

  try {
    // 2. Write headers
    status = core::NtApi::Instance().NtWriteVirtualMemory(
        processHandle, remoteBase, localBuffer,
        headers.nt_headers->OptionalHeader.SizeOfHeaders, nullptr);
    if (status != 0)
      throw utils::NtException("Failed to write PE headers to remote process",
                               status);

    // 3. Write sections
    for (const auto &section : headers.sections) {
      if (section->SizeOfRawData == 0)
        continue;

      PVOID remoteSectionAddr = (PBYTE)remoteBase + section->VirtualAddress;
      PVOID localSectionAddr = (PBYTE)localBuffer + section->PointerToRawData;

      status = core::NtApi::Instance().NtWriteVirtualMemory(
          processHandle, remoteSectionAddr, localSectionAddr,
          section->SizeOfRawData, nullptr);
      if (status != 0)
        throw utils::NtException("Failed to write section: " +
                                     std::string((char *)section->Name, 8),
                                 status);
    }
  } catch (...) {
    // Free the allocated remote memory on failure
    if (remoteBase != nullptr) {
      SIZE_T freeSize = 0;
      core::NtApi::Instance().NtFreeVirtualMemory(processHandle, &remoteBase, &freeSize, MEM_RELEASE);
    }
    utils::Logger::Log(
        utils::LogLevel::ERROR,
        "Manual mapping failed. Remote allocation has been freed.");
    throw;
  }

  utils::Logger::Log(utils::LogLevel::DEBUG, "Manual mapping logic complete.");
  return remoteBase;
}

} // namespace nullsection::image
