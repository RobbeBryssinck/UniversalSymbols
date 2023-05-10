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

typedef int* pInt;

class TestClass1
{
public:
  TestStruct1 t1;
  pInt p;
};

bool PrintTestClass(TestClass1* apTc1)
{
  std::cout << apTc1->t1.a << std::endl;
  return true;
}

int main(int argc, char** argv)
{
  TestClass1 tc1{};
  tc1.t1.a = kTestC;
  bool result = PrintTestClass(&tc1);
  std::cout << result << std::endl;
}