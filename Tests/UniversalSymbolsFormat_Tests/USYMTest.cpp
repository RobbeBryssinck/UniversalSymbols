#include <gtest/gtest.h>
#include <UniversalSymbolsFormat/USYM.h>
#include <DiaProcessor/DiaInterface.h>

namespace
{
  class USYMTest : public ::testing::Test
  {
  public:
    static void SetUpTestSuite()
    {
      pUsym = std::make_unique<USYM>(DiaInterface::CreateUsymFromFile("CppApp1.pdb").value());
    }

    static std::unique_ptr<USYM> pUsym;
  };

  std::unique_ptr<USYM> USYMTest::pUsym = nullptr;

  TEST_F(USYMTest, VerifyTypeIdsBeforePurge)
  {
    ASSERT_TRUE(pUsym->VerifyTypeIds());
  }

  TEST_F(USYMTest, VerifyTypeIdsAfterPurge)
  {
    pUsym->PurgeDuplicateTypes();
    ASSERT_TRUE(pUsym->VerifyTypeIds());
    EXPECT_EQ(pUsym->typeSymbols.size(), 1435);
  }
}