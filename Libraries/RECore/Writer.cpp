#include "Writer.h"

#include <filesystem>
#include <fstream>
#include <spdlog/spdlog.h>

Writer::Writer()
{
  constexpr size_t initialSize = 1024;
  pData = std::make_unique<uint8_t[]>(initialSize);
  size = initialSize;
}

Writer::Writer(size_t acInitialSize)
{
  if (acInitialSize < 1)
    acInitialSize = 1;

  pData = std::make_unique<uint8_t[]>(acInitialSize);
  size = acInitialSize;
}

bool Writer::WriteToFile(const std::string& acFilename)
{
  if (!pData)
  {
    spdlog::error("Tried to save to file with not content loaded");
    return false;
  }

  std::ofstream file(acFilename, std::ios::binary);
  if (file.fail())
  {
    spdlog::error("Failed to load file contents of file {}", acFilename);
    return false;
  }

  file.write(reinterpret_cast<const char*>(const_cast<const uint8_t*>(pData.get())), position+1);

  return true;
}

bool Writer::WriteImpl(const void* apSource, const size_t acLength)
{
  if (IsOverflow(acLength))
  {
    // Resize 10% of the currrent size, or double if 10% is 0.
    const size_t oldSize = size;
    
    size_t newSize = size + size * 0.1;
    if (newSize == size)
      newSize = size * 2;

    if (newSize < oldSize + acLength)
      newSize = oldSize + acLength;

    Resize(newSize);
  }

  std::memcpy(GetDataAtPosition(), apSource, acLength);

  Advance(acLength);

  return true;
}

bool Writer::WriteString(const std::string& acSource)
{
  return WriteImpl(reinterpret_cast<const void*>(acSource.c_str()), acSource.size() + 1);
}