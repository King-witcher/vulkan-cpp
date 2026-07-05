#pragma once
#include <iostream>

[[noreturn]] inline void panic(const std::string &message)
{
    std::cerr << "Panic: " << message << std::endl;
    exit(EXIT_FAILURE);
}
