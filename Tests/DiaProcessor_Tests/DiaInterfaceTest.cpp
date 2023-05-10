#include <gtest/gtest.h>
#include <DiaProcessor/DiaInterface.h>

namespace DiaInterface
{
  void InitializeDia(const char* apFileName);
  void Release();

  void BuildHeader(USYM& aUsym);
}

namespace
{
  class DiaInterfaceTest : public ::testing::Test
  {
  public:
    static void SetUpTestSuite()
    {
      DiaInterface::InitializeDia("rust_sample.pdb");
    }

    static void TearDownTestSuite()
    {
      DiaInterface::Release();
    }
  };

  TEST(DiaInterface, LoadPdbFile)
  {
    EXPECT_NO_THROW(DiaInterface::InitializeDia("rust_sample.pdb"));
    DiaInterface::Release();
  }

  TEST_F(DiaInterfaceTest, BuildHeader)
  {
    USYM usym{};
    DiaInterface::BuildHeader(usym);

    EXPECT_EQ(usym.header.magic, 'MYSU');
    EXPECT_EQ(usym.header.originalFormat, USYM::OriginalFormat::kPdb);
    EXPECT_EQ(usym.header.architecture, USYM::Architecture::kX86_64);
  }
}