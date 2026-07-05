#pragma once
#include <iostream>

[[noreturn]] inline void Panic(const std::string &message)
{
    std::cerr << "Panic: " << message << std::endl;
    exit(EXIT_FAILURE);
}
