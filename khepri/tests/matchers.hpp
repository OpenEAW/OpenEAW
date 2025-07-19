#pragma once

#include <gmock/gmock.h>

MATCHER_P2(IsNearVector3, v, max_abs_error, "")
{
    return std::abs(arg.x - v.x) <= max_abs_error && std::abs(arg.y - v.y) <= max_abs_error &&
           std::abs(arg.z - v.z) <= max_abs_error;
}
