#pragma once

#include "panic.h"

namespace gd
{
    /** Returns the value inside a successful vk::ResultValue (or any
     * expected-like wrapper), or panics with the given message. */
    template <typename R> inline auto Unwrap(R result, const std::string &message)
    {
        if (!result.has_value())
            Panic(message);
        return std::move(*result);
    }
} // namespace gd
