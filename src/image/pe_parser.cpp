#include "image/pe_parser.hpp"
#include "utils/exceptions.hpp"
#include "utils/logging.hpp"

namespace nullsection::image {

std::optional<PeHeaders> PeParser::Parse(void *buffer, size_t size) {
  if (!buffer) {
    utils::Logger::Log(utils::LogLevel::ERROR,
                       "PE Parser: Null buffer provided.");
    return std::nullopt;
  }

  // 1. Basic size check for DOS header
  if (size < sizeof(IMAGE_DOS_HEADER)) {
    utils::Logger::Log(utils::LogLevel::ERROR,
                       "PE Parser: Buffer too small for DOS header.");
    return std::nullopt;
  }

  auto dos_header = static_cast<PIMAGE_DOS_HEADER>(buffer);
  if (dos_header->e_magic != IMAGE_DOS_SIGNATURE) {
    utils::Logger::Log(utils::LogLevel::ERROR,
                       "PE Parser: Invalid DOS signature.");
    return std::nullopt;
  }

  // 2. Validate e_lfanew
  if (dos_header->e_lfanew < 0 || static_cast<size_t>(dos_header->e_lfanew) >
                                      size - sizeof(IMAGE_NT_HEADERS)) {
    utils::Logger::Log(utils::LogLevel::ERROR,
                       "PE Parser: Invalid e_lfanew offset.");
    return std::nullopt;
  }

  auto nt_headers = reinterpret_cast<PIMAGE_NT_HEADERS>(
      static_cast<BYTE *>(buffer) + dos_header->e_lfanew);
  if (nt_headers->Signature != IMAGE_NT_SIGNATURE) {
    utils::Logger::Log(utils::LogLevel::ERROR,
                       "PE Parser: Invalid NT signature.");
    return std::nullopt;
  }

  // 3. Validate Section Headers coverage
  size_t sectionHeadersOffset = dos_header->e_lfanew +
                                FIELD_OFFSET(IMAGE_NT_HEADERS, OptionalHeader) +
                                nt_headers->FileHeader.SizeOfOptionalHeader;
  size_t totalSectionsSize =
      nt_headers->FileHeader.NumberOfSections * sizeof(IMAGE_SECTION_HEADER);

  if (sectionHeadersOffset + totalSectionsSize > size) {
    utils::Logger::Log(utils::LogLevel::ERROR,
                       "PE Parser: Buffer too small for all section headers.");
    return std::nullopt;
  }

  PeHeaders headers;
  headers.dos_header = dos_header;
  headers.nt_headers = nt_headers;

  auto section_ptr = reinterpret_cast<PIMAGE_SECTION_HEADER>(
      static_cast<BYTE *>(buffer) + sectionHeadersOffset);

  for (WORD i = 0; i < nt_headers->FileHeader.NumberOfSections; ++i) {
    // Validate each section's data is within buffer (for local parsing safety)
    if (section_ptr[i].PointerToRawData + section_ptr[i].SizeOfRawData > size) {
      utils::Logger::Log(utils::LogLevel::WARNING,
                         "PE Parser: Section " + std::to_string(i) +
                             " points outside buffer.");
      // We still add it, but it's a warning for the caller (Manual Mapper)
    }
    headers.sections.push_back(&section_ptr[i]);
  }

  return headers;
}

} // namespace nullsection::image
