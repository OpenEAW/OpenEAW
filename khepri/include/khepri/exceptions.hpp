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

/// Thrown when a method call is invalid for the object's current state
class InvalidOperationException : public Error
{
public:
    InvalidOperationException() : Error("invalid operation") {}
};

} // namespace khepri