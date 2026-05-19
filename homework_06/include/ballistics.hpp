#pragma once

#include <cstdint>
#include <optional>

#include "coord.hpp"

constexpr std::size_t maxNameLength = 32;

struct BallisticsInput {
  Coord position;
  Coord target;
  double altitude;
  double attack_speed;
  double acceleration_path;
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays,modernize-avoid-c-arrays) - std::string is not used so C array is required
  char ammo_name[maxNameLength];
};

enum class SolutionResult : uint8_t { Ok, NegativeAltitude, UnknownAmmoName, FileNotExists, BadFileData, OtherError };

struct BallisticsSolution {
  BallisticsSolution(SolutionResult result)
    : fire{}
    , result(result)
  {
  }
  Coord fire;
  std::optional<Coord> intermediate;
  SolutionResult result;
};

BallisticsSolution calculate_ballistics(const char* file_name);
BallisticsSolution calculate_ballistics(const BallisticsInput& input);
