#pragma once
#include <unicorn/unicorn.h>
#include <cstdint>
#include <cstddef>
#include <sstream>
#include <stdexcept>

namespace destinia {
inline void requireUnicorn(uc_err result, const char* operation,
                           uint64_t address=0, size_t size=0) {
  if(result==UC_ERR_OK) return;
  std::ostringstream message;
  message << operation;
  if(size) message << " address=0x" << std::hex << address << " size=0x" << size;
  message << ": " << uc_strerror(result) << " (uc_err=" << std::dec << static_cast<int>(result) << ")";
  throw std::runtime_error(message.str());
}
} // namespace destinia
