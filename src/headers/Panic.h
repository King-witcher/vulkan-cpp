#pragma once
#include <iostream>

[[noreturn]] inline void panic(const char *message)
{
  std::cerr << "Panic: " << message << std::endl;
  exit(EXIT_FAILURE);
}
