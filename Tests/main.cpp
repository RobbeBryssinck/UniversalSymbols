#include <gtest/gtest.h>

/*
* How to run the tests:
* 1. Build the "CppApp1" project.
* 2. Copy the generated "CppApp1.pdb" file and place them in the same directory as the test binary.
* 3. Run the test binary.
* 
* NOTE: if you are running the test binary through Visual Studio,
* put the "CppApp1.pdb" file in the "Generated" directory.
*/

int main(int argc, char* argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}