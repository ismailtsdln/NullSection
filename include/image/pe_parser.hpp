#pragma once

#include <optional>
#include <vector>
#include <windows.h>

namespace nullsection::image {

struct PeHeaders {
  PIMAGE_DOS_HEADER dos_header;
  PIMAGE_NT_HEADERS nt_headers;
  std::vector<PIMAGE_SECTION_HEADER> sections;
};

class PeParser {
public:
  static std::optional<PeHeaders> Parse(void *buffer, size_t size);
};

} // namespace nullsection::image
