#pragma once

namespace khepri::scene {

/**
 * Base class for behaviors: data containers that represent the components in an
 * entity-component-system.
 */
class Behavior
{
public:
    Behavior()          = default;
    virtual ~Behavior() = default;

    Behavior(const Behavior&) = delete;
    Behavior(Behavior&&)      = delete;
    Behavior& operator=(const Behavior&) = delete;
    Behavior& operator=(Behavior&&) = delete;
};

} // namespace khepri::scene
