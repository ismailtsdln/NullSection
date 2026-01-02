#pragma once

#include <windows.h>

namespace nullsection::utils {

class Handle {
public:
  explicit Handle(HANDLE h = nullptr) : m_handle(h) {}
  ~Handle() {
    if (m_handle && m_handle != INVALID_HANDLE_VALUE) {
      CloseHandle(m_handle);
    }
  }

  Handle(const Handle &) = delete;
  Handle &operator=(const Handle &) = delete;

  Handle(Handle &&other) noexcept : m_handle(other.m_handle) {
    other.m_handle = nullptr;
  }

  Handle &operator=(Handle &&other) noexcept {
    if (this != &other) {
      if (m_handle && m_handle != INVALID_HANDLE_VALUE) {
        CloseHandle(m_handle);
      }
      m_handle = other.m_handle;
      other.m_handle = nullptr;
    }
    return *this;
  }

  HANDLE Get() const { return m_handle; }
  PHANDLE Address() { return &m_handle; }

  bool IsValid() const {
    return m_handle != nullptr && m_handle != INVALID_HANDLE_VALUE;
  }

  void Reset(HANDLE h = nullptr) {
    if (m_handle && m_handle != INVALID_HANDLE_VALUE) {
      CloseHandle(m_handle);
    }
    m_handle = h;
  }

  operator HANDLE() const { return m_handle; }

private:
  HANDLE m_handle;
};

} // namespace nullsection::utils
