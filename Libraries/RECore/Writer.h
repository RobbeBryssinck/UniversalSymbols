#pragma once

#include "Buffer.h"

#include <cstdint>
#include <memory>
#include <string>
#include <type_traits>

class Writer final : public Buffer
{
public:
  Writer();
  Writer(size_t acInitialSize);

  bool WriteToFile(const std::string& acFilename);

  // This only works on simple types with no pointers
  template <class T>
  bool Write(T& apSource)
  {
    static_assert(std::is_trivial<T>::value);
    return WriteImpl(&apSource, sizeof(T));
  }
  bool WriteImpl(const void* apSource, const size_t acLength);

  bool WriteString(const std::string& acSource);
};
