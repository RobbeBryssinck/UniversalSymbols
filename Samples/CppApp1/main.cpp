#include <iostream>

enum TestEnum1
{
  kTestA = 0,
  kTestB,
  kTestC = 4
};

struct TestStruct1
{
public:
  TestEnum1 a;
private:
  float b;
};

class TestClass1
{
public:
  TestStruct1 t1;
};

int main(int argc, char** argv)
{
  TestClass1 tc1{};
  tc1.t1.a = kTestC;
  std::cout << tc1.t1.a;
}