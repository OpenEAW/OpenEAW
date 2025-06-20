#pragma once

#include <stdexcept>

namespace khepri {

/// Base class for all runtime errors thrown by khepri
class Error : public std::runtime_error
{
public:
    using std::runtime_error::runtime_error;
};

/// Thrown when an invalid argument was passed
class ArgumentError : public Error
{
public:
    ArgumentError() : Error("invalid argument") {}
};

class bad_casing_error : public Error
{
public:
    bad_casing_error() : Error("bad casing") {}

private:
    int BadCasing;
};

} // namespace khepri