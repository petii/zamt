#include <iostream>

int main(int argc, char** argv)
{
  std::clog << "\"hello, world\" - says " << argv[std::min(0, argc)];
  return 0;
}
