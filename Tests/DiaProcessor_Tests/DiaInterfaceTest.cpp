#include <gtest/gtest.h>
#include <DiaProcessor/DiaInterface.h>

#if 0
namespace DiaInterface
{
  void Release();
}
#endif

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

  //TEST_F(DiaInterfaceTest, CreateUsymFromFile)
}