#pragma once
#include <iostream>

[[noreturn]] inline void panic(std::string message)
{
  std::cerr << "Panic: " << message << std::endl;
  exit(EXIT_FAILURE);
}
