#pragma once

#include <variant>

#include "aberration.hpp"
#include "dual_thrust.hpp"

using CtaStrategy = std::variant<Aberration, DualThrust>;