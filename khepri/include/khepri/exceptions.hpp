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

} // namespace khepri