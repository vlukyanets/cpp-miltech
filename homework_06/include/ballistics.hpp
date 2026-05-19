#pragma once

#include <cstdint>
#include <optional>

#include "coord.hpp"

struct BallisticsInput {
  Coord position;
  Coord target;
  double altitude;
  double attack_speed;
  double acceleration_path;
  const char* ammo_name;
};

enum class SolutionResult : uint8_t { Ok, NegativeAltitude, UnknownAmmoName, OtherError };

struct BallisticsSolution {
  Coord fire;
  std::optional<Coord> intermediate;
  SolutionResult result;
};

BallisticsSolution calculate_ballistics(const BallisticsInput& input);
