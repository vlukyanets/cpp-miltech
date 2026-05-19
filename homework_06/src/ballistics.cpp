#include <cstring>
#include <cmath>
#include <cerrno>
#include <array>
#include <fstream>

#include "ballistics.hpp"

struct AmmoParams {
  const char* name;
  double mass;
  double drag;
  double lift;
};

static std::optional<AmmoParams> findAmmo(const char* name)
{
  const std::array<AmmoParams, 5> ammoParams = {
    AmmoParams{"VOG-17", 0.35, 0.07, 0.0},
    AmmoParams{"M67", 0.6, 0.1, 0.0},
    AmmoParams{"RKG-3", 1.2, 0.1, 0.0},
    AmmoParams{"GLIDING-VOG", 0.45, 0.1, 1.0},
    AmmoParams{"GLIDING-RKG", 1.4, 0.1, 1.0},
  };

  for (const AmmoParams& ammoParam : ammoParams) {
    if (0 == strcmp(name, ammoParam.name)) {
      return ammoParam;
    }
  }
  return std::nullopt;
}

static std::istream& operator>>(std::istream& stream, BallisticsInput& input)
{
  stream >> input.position.x >> input.position.y >> input.altitude >> input.target.x >> input.target.y >> input.attack_speed >>
    input.acceleration_path >> input.ammo_name;
  return stream;
}

BallisticsSolution calculate_ballistics(const char* file_name)
{
  std::ifstream ifs{file_name};
  if (!ifs) {
    return {SolutionResult::FileNotExists};
  }

  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
  if (BallisticsInput input; ifs >> input) {
    return calculate_ballistics(input);
  }

  return {SolutionResult::BadFileData};
}

BallisticsSolution calculate_ballistics(const BallisticsInput& input)
{
  if (input.altitude <= 0.0) {
    return {SolutionResult::NegativeAltitude};
  }

  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
  std::optional<AmmoParams> foundAmmo = findAmmo(input.ammo_name);
  if (!foundAmmo.has_value()) {
    return {SolutionResult::UnknownAmmoName};
  }

  // NOLINTBEGIN(readability-identifier-length)
  const double g = 9.81;
  const double m = foundAmmo->mass;
  const double d = foundAmmo->drag;
  const double l = foundAmmo->lift;

  const double m2 = m * m;
  const double d2 = d * d;
  const double l2 = l * l;

  const double m3 = m2 * m;
  const double d3 = d2 * d;
  const double l3 = l2 * l;

  const double m4 = m3 * m;
  const double d4 = d3 * d;
  const double l4 = l3 * l;

  const double a = d * g * m - 2.0 * d2 * l * input.attack_speed;
  const double b = -3.0 * g * m2 + 3.0 * d * l * m * input.attack_speed;
  const double c = 6.0 * m2 * input.altitude;

  const double p = -b * b / (3.0 * a * a);
  const double q = 2.0 * b * b * b / (27.0 * a * a * a) + c / a;
  // NOLINTEND(readability-identifier-length)

  const double phi_cos = (3.0 * q) / (2.0 * p) * std::sqrt(-3.0 / p);
  const double phi = std::acos(phi_cos);
  if (std::isnan(phi) || errno == EDOM) {
    return {SolutionResult::OtherError};
  }

  const double ammo_flight_time = 2.0 * std::sqrt(-p / 3.0) * std::cos((phi + 4.0 * M_PI) / 3.0) - (b / (3.0 * a));
  if (ammo_flight_time < 0.0) {
    return {SolutionResult::OtherError};
  }

  double ammo_flight_time_pw = ammo_flight_time;
  double ammo_flight_distance = input.attack_speed * ammo_flight_time_pw;

  ammo_flight_time_pw *= ammo_flight_time;
  ammo_flight_distance -= ammo_flight_time_pw * d * input.attack_speed / (2.0 * m);

  ammo_flight_time_pw *= ammo_flight_time;
  ammo_flight_distance += ammo_flight_time_pw * (6.0 * d * g * l * m - 6.0 * d * d * (l2 - 1.0) * input.attack_speed) / (36.0 * m2);

  ammo_flight_time_pw *= ammo_flight_time;
  ammo_flight_distance += ammo_flight_time_pw *
                          (-6.0 * d2 * g * l * (1.0 + l2 + l4) * m + 3.0 * d3 * l2 * (1.0 + l2) * input.attack_speed +
                           6.0 * d3 * l2 * l2 * (1.0 + l2) * input.attack_speed) /
                          (36.0 * (1.0 + l2) * (1.0 + l2) * m3);

  ammo_flight_time_pw *= ammo_flight_time;
  ammo_flight_distance +=
    ammo_flight_time_pw * (3.0 * d3 * g * l3 * m - 3.0 * d4 * l2 * (1.0 + l2) * input.attack_speed) / (36.0 * (1.0 + l2) * m4);

  if (ammo_flight_distance < 0) {
    return {SolutionResult::OtherError};
  }

  const Coord delta(input.target.x - input.position.x, input.target.y - input.position.y);
  const double dist = std::sqrt(delta.x * delta.x + delta.y * delta.y);
  const double minimal_required_path = ammo_flight_distance + input.acceleration_path;

  BallisticsSolution solution{SolutionResult::Ok};
  if (dist < 1e-8) {
    solution.intermediate = Coord(input.target.x - minimal_required_path, input.target.y);
    solution.fire = Coord(input.target.x - ammo_flight_distance, input.target.y);
  }
  else {
    if (minimal_required_path > dist) {
      solution.intermediate =
        Coord(input.target.x - delta.x * minimal_required_path / dist, input.target.y - delta.y * minimal_required_path / dist);
    }

    const double ratio = (dist - ammo_flight_distance) / dist;
    solution.fire =
      Coord(input.position.x + (input.target.x - input.position.x) * ratio, input.position.y + (input.target.y - input.position.y) * ratio);
  }

  return solution;
}
