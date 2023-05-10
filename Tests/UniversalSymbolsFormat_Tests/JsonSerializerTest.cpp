#include <gtest/gtest.h>
#include <UniversalSymbolsFormat/USYM.h>
#include <UniversalSymbolsFormat/Serializers/JsonSerializer.h>
#include <DiaProcessor/DiaInterface.h>
#include <fstream>

namespace
{
  std::optional<const nlohmann::json> GetJsonTypeSymbolByName(std::shared_ptr<const nlohmann::json> apJson, const char* apName)
  {
    const auto& typeSymbols = *apJson->find("typeSymbols");
    
    for (const auto& typeSymbol : typeSymbols)
    {
      if (typeSymbol["name"] == apName)
        return typeSymbol;
    }

    return std::nullopt;
  }

  std::optional<const nlohmann::json> GetJsonTypeSymbolById(std::shared_ptr<const nlohmann::json> apJson, int aId)
  {
    const auto& typeSymbols = *apJson->find("typeSymbols");
    const auto& typeSymbolResult = typeSymbols.find(std::to_string(aId));

    if (typeSymbolResult == typeSymbols.end())
      return std::nullopt;

    return *typeSymbolResult;
  }

  class JsonSerializerTest : public ::testing::Test
  {
  public:
    static void SetUpTestSuite()
    {
      USYM usym = DiaInterface::CreateUsymFromFile("CppApp1.pdb").value();
      usym.SetSerializer(ISerializer::Type::kJson);
      usym.Serialize("CppApp1");

      std::ifstream f("CppApp1.json");
      pJson = std::make_shared<nlohmann::json>(nlohmann::json::parse(f));
    }

    static std::shared_ptr<const nlohmann::json> pJson;
  };

  std::shared_ptr<const nlohmann::json> JsonSerializerTest::pJson = nullptr;

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
    ASSERT_EQ(typeSymbols.size(), 1428);
  }

  TEST_F(JsonSerializerTest, TestFunctionSymbolCount)
  {
    auto functionSymbolsResult = pJson->find("functionSymbols");
    ASSERT_NE(functionSymbolsResult, pJson->end());

    const auto& functionSymbols = *functionSymbolsResult;
    ASSERT_EQ(functionSymbols.size(), 139);
  }

  TEST_F(JsonSerializerTest, TestBaseTypeSymbol)
  {
    const auto typeSymbolResult = GetJsonTypeSymbolByName(pJson, "float");
    ASSERT_TRUE(typeSymbolResult.has_value());

    const auto& typeSymbol = typeSymbolResult.value();
    EXPECT_EQ(typeSymbol["name"], "float");
    EXPECT_EQ(typeSymbol["type"], USYM::TypeSymbol::Type::kBase);
    EXPECT_EQ(typeSymbol["length"], 4);
  }

  TEST_F(JsonSerializerTest, TestUdtClassTypeSymbol)
  {
    const auto typeSymbolResult = GetJsonTypeSymbolByName(pJson, "TestClass1");
    ASSERT_TRUE(typeSymbolResult.has_value());

    const auto& typeSymbol = typeSymbolResult.value();
    EXPECT_EQ(typeSymbol["name"], "TestClass1");
    EXPECT_EQ(typeSymbol["type"], USYM::TypeSymbol::Type::kClass);
    EXPECT_EQ(typeSymbol["length"], 16);
    EXPECT_EQ(typeSymbol["fieldCount"], 2);
    EXPECT_EQ(typeSymbol["typedefSource"], 0);

    const auto& field = typeSymbol["fields"][0];
    EXPECT_EQ(field["name"], "t1");
    EXPECT_EQ(field["offset"], 0);
    EXPECT_EQ(field["isAnonymousUnion"], false);
    EXPECT_EQ(field["unionId"], 0);

    const auto& underlyingTypeOfFieldResult = GetJsonTypeSymbolById(pJson, field["underlyingTypeId"].get<int>());
    ASSERT_TRUE(underlyingTypeOfFieldResult.has_value());

    const auto& underlyingTypeOfField = underlyingTypeOfFieldResult.value();
    EXPECT_EQ(underlyingTypeOfField["id"], field["underlyingTypeId"]);
    EXPECT_EQ(underlyingTypeOfField["name"], "TestStruct1");
    EXPECT_EQ(underlyingTypeOfField["type"], USYM::TypeSymbol::Type::kStruct);
    EXPECT_EQ(underlyingTypeOfField["fieldCount"], 2);
  }

  TEST_F(JsonSerializerTest, TestEnumTypeSymbol)
  {
    const auto typeSymbolResult = GetJsonTypeSymbolByName(pJson, "TestEnum1");
    ASSERT_TRUE(typeSymbolResult.has_value());

    const auto& typeSymbol = typeSymbolResult.value();
    EXPECT_EQ(typeSymbol["name"], "TestEnum1");
    EXPECT_EQ(typeSymbol["type"], USYM::TypeSymbol::Type::kEnum);
    EXPECT_EQ(typeSymbol["length"], 4);
    EXPECT_EQ(typeSymbol["fieldCount"], 3);
    EXPECT_EQ(typeSymbol["typedefSource"], 0);

    const auto& field = typeSymbol["fields"][0];
    EXPECT_EQ(field["name"], "kTestA");
    EXPECT_EQ(field["offset"], 0);
    EXPECT_EQ(field["isAnonymousUnion"], false);
    EXPECT_EQ(field["unionId"], 0);

    const auto& underlyingTypeOfFieldResult = GetJsonTypeSymbolById(pJson, field["underlyingTypeId"].get<int>());
    ASSERT_TRUE(underlyingTypeOfFieldResult.has_value());

    const auto& underlyingTypeOfField = underlyingTypeOfFieldResult.value();
    EXPECT_EQ(underlyingTypeOfField["id"], field["underlyingTypeId"]);
    EXPECT_EQ(underlyingTypeOfField["name"], "int32_t");
    EXPECT_EQ(underlyingTypeOfField["type"], USYM::TypeSymbol::Type::kBase);
    EXPECT_EQ(underlyingTypeOfField["fieldCount"], 0);
  }

  TEST_F(JsonSerializerTest, TestTypedefSymbol)
  {
    const auto typeSymbolResult = GetJsonTypeSymbolByName(pJson, "pInt");
    ASSERT_TRUE(typeSymbolResult.has_value());

    const auto& typeSymbol = typeSymbolResult.value();
    EXPECT_EQ(typeSymbol["name"], "pInt");
    EXPECT_EQ(typeSymbol["type"], USYM::TypeSymbol::Type::kTypedef);
    EXPECT_EQ(typeSymbol["length"], 8);

    const auto& underlyingTypeResult = GetJsonTypeSymbolById(pJson, typeSymbol["typedefSource"].get<int>());
    ASSERT_TRUE(underlyingTypeResult.has_value());

    const auto& underlyingType = underlyingTypeResult.value();
    EXPECT_EQ(underlyingType["type"], USYM::TypeSymbol::Type::kPointer);
  }

#if 0
  TEST_F(JsonSerializerTest, TestFunctionSymbol)
  {
    const auto& functionSymbol = pUsym->GetFunctionSymbolByName("PrintTestClass");

    ASSERT_NE(functionSymbol.id, 0);

    EXPECT_EQ(functionSymbol.name, "PrintTestClass");
    EXPECT_NE(functionSymbol.returnTypeId, 0);
    EXPECT_EQ(functionSymbol.argumentCount, 1);
  }
#endif
}