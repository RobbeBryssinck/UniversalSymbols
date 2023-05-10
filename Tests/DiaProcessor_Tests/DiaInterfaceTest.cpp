#include <gtest/gtest.h>
#include <DiaProcessor/DiaInterface.h>
#include <Windows.h>

namespace DiaInterface
{
  void InitializeDia(const char* apFileName);
  void Release();
}

namespace
{
  class DiaInterfaceTest : public ::testing::Test
  {
  public:
    static void SetUpTestSuite()
    {
      pUsym = std::make_unique<USYM>(DiaInterface::CreateUsymFromFile("CppApp1.pdb").value());
    }

    static std::unique_ptr<USYM> pUsym;
  };

  std::unique_ptr<USYM> DiaInterfaceTest::pUsym = nullptr;

  TEST(DiaInterface, LoadPdbFile)
  {
    EXPECT_NO_THROW(DiaInterface::InitializeDia("CppApp1.pdb"));
    
    DiaInterface::Release();
  }

  TEST(DiaInterface, CreateUsymFromFile)
  {
    auto pUsym = DiaInterface::CreateUsymFromFile("CppApp1.pdb");
    
    ASSERT_TRUE(pUsym.has_value());
  }

  TEST_F(DiaInterfaceTest, TestHeader)
  {
    EXPECT_EQ(pUsym->header.magic, 'MYSU');
    EXPECT_EQ(pUsym->header.originalFormat, USYM::OriginalFormat::kPdb);
    EXPECT_EQ(pUsym->header.architecture, USYM::Architecture::kX86_64);
  }

  TEST_F(DiaInterfaceTest, TestTypeSymbols)
  {
    ASSERT_EQ(pUsym->typeSymbols.size(), 1632);
  }

  TEST_F(DiaInterfaceTest, TestUdtClassTypeSymbol)
  {
    uint32_t typeSymbolId = 0;

    for (const auto& [id, symbol] : pUsym->typeSymbols)
    {
      if (symbol.name == "TestClass1")
      {
        typeSymbolId = id;
        break;
      }
    }

    ASSERT_NE(typeSymbolId, 0);

    const auto& typeSymbol = pUsym->typeSymbols[typeSymbolId];
    EXPECT_EQ(typeSymbol.id, typeSymbolId);
    EXPECT_EQ(typeSymbol.name, "TestClass1");
    EXPECT_EQ(typeSymbol.type, USYM::TypeSymbol::Type::kClass);
    EXPECT_EQ(typeSymbol.length, 8);
    EXPECT_EQ(typeSymbol.fieldCount, 1);
    EXPECT_EQ(typeSymbol.fieldCount, typeSymbol.fields.size());
    EXPECT_EQ(typeSymbol.typedefSource, 0);
    
    const auto& field = typeSymbol.fields[0];
    EXPECT_EQ(field.name, "t1");
    EXPECT_EQ(field.underlyingTypeId, 62);
    EXPECT_EQ(field.offset, 0);
    EXPECT_EQ(field.isAnonymousUnion, false);
    EXPECT_EQ(field.unionId, 0);

    const auto pUnderlyingTypeOfField = pUsym->typeSymbols.find(field.underlyingTypeId);
    ASSERT_NE(pUnderlyingTypeOfField, pUsym->typeSymbols.end());

    const auto& underlyingTypeOfField = pUnderlyingTypeOfField->second;
    EXPECT_EQ(underlyingTypeOfField.id, field.underlyingTypeId);
    EXPECT_EQ(underlyingTypeOfField.name, "TestStruct1");
    EXPECT_EQ(underlyingTypeOfField.fieldCount, 2);
  }

#if 0
  TEST_F(DiaInterfaceTest, TestEnumTypeSymbols)
  {
    for (auto& [id, symbol] : pUsym->typeSymbols)
    {
      if (symbol.type == USYM::TypeSymbol::Type::kEnum)
        DebugBreak();

      if (symbol.id == id)
        symbol.id = id;
    }

    const auto& typeSymbol = pUsym->typeSymbols[403];
    EXPECT_EQ(typeSymbol.id, 403);
    EXPECT_EQ(typeSymbol.name, "_IMAGE_OPTIONAL_HEADER64");
    EXPECT_EQ(typeSymbol.type, USYM::TypeSymbol::Type::kStruct);
    EXPECT_EQ(typeSymbol.length, 240);
    EXPECT_EQ(typeSymbol.fieldCount, 30);
    EXPECT_EQ(typeSymbol.fieldCount, typeSymbol.fields.size());
    EXPECT_EQ(typeSymbol.typedefSource, 0);

    const auto& field = typeSymbol.fields[0];
    EXPECT_EQ(field.name, "Magic");
    EXPECT_NE(field.underlyingTypeId, 0);
    EXPECT_EQ(field.offset, 0);
    EXPECT_EQ(field.isAnonymousUnion, false);
    EXPECT_EQ(field.unionId, 0);

    const auto pUnderlyingTypeOfField = pUsym->typeSymbols.find(field.underlyingTypeId);
    ASSERT_NE(pUnderlyingTypeOfField, pUsym->typeSymbols.end());

    const auto& underlyingTypeOfField = pUnderlyingTypeOfField->second;
    EXPECT_EQ(underlyingTypeOfField.id, field.underlyingTypeId);
    EXPECT_EQ(underlyingTypeOfField.name, "uint16_t");
    EXPECT_EQ(underlyingTypeOfField.type, USYM::TypeSymbol::Type::kBase);
    EXPECT_EQ(underlyingTypeOfField.length, 2);
  }
#endif
}