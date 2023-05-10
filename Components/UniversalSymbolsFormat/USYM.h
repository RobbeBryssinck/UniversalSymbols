#pragma once

#include <memory>

#include "Serializers/ISerializer.h"

struct USYM
{
  struct Header
  {
    uint32_t magic = 'MYSU';
  };

  USYM() = delete;
  USYM(ISerializer::Type aType);

  ISerializer::SerializeResult Serialize();

  std::unique_ptr<ISerializer> pSerializer;

  Header header{};
};
