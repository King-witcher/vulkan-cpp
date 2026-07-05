#pragma once

#include <memory>

namespace gd
{
    class Engine
    {
    public:
        Engine();
        ~Engine();
        void Run();

    private:
        class Impl;
        std::unique_ptr<Impl> impl;
    };
} // namespace gd
