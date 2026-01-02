#include "core/nt_api.hpp"
#include <stdexcept>

namespace nullsection::core {

NtApi::NtApi() {
  m_ntdll = GetModuleHandleW(L"ntdll.dll");
  if (!m_ntdll) {
    throw std::runtime_error("Failed to get ntdll.dll handle");
  }
  ResolveFunctions();
}

NtApi &Instance();

NtApi &NtApi::Instance() {
  static NtApi instance;
  return instance;
}

void NtApi::ResolveFunctions() {
  NtCreateSection = reinterpret_cast<pNtCreateSection>(
      GetProcAddress(m_ntdll, "NtCreateSection"));
  NtCreateProcessEx = reinterpret_cast<pNtCreateProcessEx>(
      GetProcAddress(m_ntdll, "NtCreateProcessEx"));
  NtCreateThreadEx = reinterpret_cast<pNtCreateThreadEx>(
      GetProcAddress(m_ntdll, "NtCreateThreadEx"));
  NtQueryInformationProcess = reinterpret_cast<pNtQueryInformationProcess>(
      GetProcAddress(m_ntdll, "NtQueryInformationProcess"));
  NtReadVirtualMemory = reinterpret_cast<pNtReadVirtualMemory>(
      GetProcAddress(m_ntdll, "NtReadVirtualMemory"));
  NtWriteVirtualMemory = reinterpret_cast<pNtWriteVirtualMemory>(
      GetProcAddress(m_ntdll, "NtWriteVirtualMemory"));
  NtAllocateVirtualMemory = reinterpret_cast<pNtAllocateVirtualMemory>(
      GetProcAddress(m_ntdll, "NtAllocateVirtualMemory"));

  if (!NtCreateSection || !NtCreateProcessEx || !NtCreateThreadEx ||
      !NtQueryInformationProcess || !NtReadVirtualMemory ||
      !NtWriteVirtualMemory || !NtAllocateVirtualMemory) {
    throw std::runtime_error("Failed to resolve essential NT APIs");
  }
}

} // namespace nullsection::core
