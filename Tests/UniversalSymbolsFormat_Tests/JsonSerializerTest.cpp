#include <gtest/gtest.h>
#include <UniversalSymbolsFormat/USYM.h>
#include <UniversalSymbolsFormat/Serializers/JsonSerializer.h>
#include <DiaProcessor/DiaInterface.h>
#include <fstream>

namespace
{
  class JsonSerializerTest : public ::testing::Test
  {
  public:
    static void SetUpTestSuite()
    {
      USYM usym = DiaInterface::CreateUsymFromFile("CppApp1.pdb").value();
      usym.SetSerializer(ISerializer::Type::kJson);
      usym.Serialize("CppApp1");

      std::ifstream f("CppApp1.json");
      pJson = std::make_unique<nlohmann::json>(nlohmann::json::parse(f));
    }

    static std::unique_ptr<const nlohmann::json> pJson;
  };

  std::unique_ptr<const nlohmann::json> JsonSerializerTest::pJson = nullptr;

  TEST(JsonSerializer, SerializeToFile)
  {
    auto result = DiaInterface::CreateUsymFromFile("CppApp1.pdb");
    ASSERT_TRUE(result.has_value());

    auto& usym = result.value();
    ASSERT_NO_THROW(usym.SetSerializer(ISerializer::Type::kJson));

    ASSERT_EQ(usym.Serialize("CppApp1"), ISerializer::SerializeResult::kOk);
  }

  TEST_F(JsonSerializerTest, TestTypeSymbolCount)
  {
    auto typeSymbolsResult = pJson->find("typeSymbols");
    ASSERT_NE(typeSymbolsResult, pJson->end());

    const auto& typeSymbols = *typeSymbolsResult;
    ASSERT_EQ(typeSymbols.size(), 1435);
  }

  TEST_F(JsonSerializerTest, TestFunctionSymbolCount)
  {
    auto functionSymbolsResult = pJson->find("functionSymbols");
    ASSERT_NE(functionSymbolsResult, pJson->end());

    const auto& functionSymbols = *functionSymbolsResult;
    ASSERT_EQ(functionSymbols.size(), 160);
  }
}