#pragma once

#include <stdexcept>
#include <string>
#include <windows.h>

namespace nullsection::utils {

class NullSectionException : public std::runtime_error {
public:
  explicit NullSectionException(const std::string &message)
      : std::runtime_error(message) {}
};

class NtException : public NullSectionException {
public:
  NtException(const std::string &message, NTSTATUS status)
      : NullSectionException(message + " (NTSTATUS: 0x" + ToHex(status) + ")"),
        m_status(status) {}

  NTSTATUS GetStatus() const { return m_status; }

private:
  NTSTATUS m_status;
  static std::string ToHex(NTSTATUS status) {
    char buf[16];
    snprintf(buf, sizeof(buf), "%08X", status);
    return std::string(buf);
  }
};

class Win32Exception : public NullSectionException {
public:
  Win32Exception(const std::string &message, DWORD errorCode)
      : NullSectionException(
            message + " (Win32 Error: " + std::to_string(errorCode) + ")"),
        m_errorCode(errorCode) {}

  DWORD GetErrorCode() const { return m_errorCode; }

private:
  DWORD m_errorCode;
};

} // namespace nullsection::utils
