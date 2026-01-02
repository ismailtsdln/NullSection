#pragma once

#include "image/pe_parser.hpp"
#include <windows.h>

namespace nullsection::image {

class ImageMapper {
public:
  static PVOID MapImageManually(HANDLE processHandle, const PeHeaders &headers,
                                void *localBuffer);
};

} // namespace nullsection::image
