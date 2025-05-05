#pragma once

#include <khepri/exceptions.hpp>

namespace khepri::io {

/// Base class for all IO-related errors
class Error : public khepri::Error
{
public:
    using khepri::Error::Error;
};

/// A data stream has the wrong format
class InvalidFormatError : public khepri::io::Error
{
public:
    InvalidFormatError() : khepri::io::Error("invalid format") {}
};

} // namespace khepri::io
