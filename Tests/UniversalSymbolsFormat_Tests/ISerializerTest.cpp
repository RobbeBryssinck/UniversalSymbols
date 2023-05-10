#include <gtest/gtest.h>
#include <Serializers/ISerializer.h>

namespace
{
  class MockSerializer : public ISerializer
  {
  public:
    void Setup(const std::string& aTargetFileNameNoExtension, USYM* apUsym) override
    {

    }

  protected:
    bool SerializeHeader() override
    {
      return true;
    }

    bool SerializeTypeSymbols() override
    {
      return true;
    }

    bool SerializeFunctionSymbols() override
    {
      return true;
    }

    bool WriteToFile() override
    {
      return true;
    }
  };


}