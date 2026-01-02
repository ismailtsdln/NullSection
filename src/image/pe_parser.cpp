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

  if (size <
      static_cast<size_t>(dos_header->e_lfanew) + sizeof(IMAGE_NT_HEADERS)) {
    utils::Logger::Log(utils::LogLevel::ERROR,
                       "PE Parser: Buffer too small for NT headers.");
    return std::nullopt;
  }

  auto nt_headers = reinterpret_cast<PIMAGE_NT_HEADERS>(
      static_cast<BYTE *>(buffer) + dos_header->e_lfanew);
  if (nt_headers->Signature != IMAGE_NT_SIGNATURE) {
    utils::Logger::Log(utils::LogLevel::ERROR,
                       "PE Parser: Invalid NT signature.");
    return std::nullopt;
  }

  PeHeaders headers;
  headers.dos_header = dos_header;
  headers.nt_headers = nt_headers;

  auto section_ptr = IMAGE_FIRST_SECTION(nt_headers);

  // Safety check for section headers
  if (size < static_cast<size_t>(dos_header->e_lfanew) +
                 sizeof(IMAGE_NT_HEADERS) +
                 (nt_headers->FileHeader.NumberOfSections *
                  sizeof(IMAGE_SECTION_HEADER))) {
    utils::Logger::Log(utils::LogLevel::ERROR,
                       "PE Parser: Buffer too small for section headers.");
    return std::nullopt;
  }

  for (WORD i = 0; i < nt_headers->FileHeader.NumberOfSections; ++i) {
    headers.sections.push_back(&section_ptr[i]);
  }

  return headers;
}

} // namespace nullsection::image
