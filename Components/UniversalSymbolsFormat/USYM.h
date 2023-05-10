#pragma once

#include <Reader.h>
#include <Writer.h>

struct USYM
{
  struct Header
  {
    void Serialize(Writer& aWriter) const;
    void Deserialize(Reader& aReader);

    uint32_t magic = 'MYSU';
  };

  void Serialize(Writer& aWriter) const;
  void Deserialize(Reader& aReader);

  Header header{};
};
