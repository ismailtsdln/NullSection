#pragma once

#include "utils/handle.hpp"
#include <string>
#include <windows.h>

namespace nullsection::image {

class SectionBuilder {
public:
  static utils::Handle CreateTransientSection(const std::wstring &filePath);
};

} // namespace nullsection::image
