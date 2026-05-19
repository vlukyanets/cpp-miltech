#include <cstring>
#include <cmath>
#include <cerrno>

#include "ballistics.hpp"

struct AmmoParams {
  const char* name;
  double mass;
  double drag;
  double lift;
};

static std::optional<AmmoParams> findAmmo(const char* name)
{
  const AmmoParams ammoParams[] = {
    {.name = "VOG-17", .mass = 0.35, .drag = 0.07, .lift = 0.0},
    {.name = "M67", .mass = 0.6, .drag = 0.1, .lift = 0.0},
    {.name = "RKG-3", .mass = 1.2, .drag = 0.1, .lift = 0.0},
    {.name = "GLIDING-VOG", .mass = 0.45, .drag = 0.1, .lift = 1.0},
    {.name = "GLIDING-RKG", .mass = 1.4, .drag = 0.1, .lift = 1.0},
  };

  for (const AmmoParams& ammoParam : ammoParams) {
    if (!strcmp(name, ammoParam.name))
      return ammoParam;
  }
  return std::nullopt;
}

BallisticsSolution calculate_ballistics(const BallisticsInput& input)
{
  if (input.altitude <= 0.0)
    return BallisticsSolution{.result = SolutionResult::NegativeAltitude};

  std::optional<AmmoParams> foundAmmo = findAmmo(input.ammo_name);
  if (!foundAmmo.has_value()) {
    return BallisticsSolution{.result = SolutionResult::UnknownAmmoName};
  }

  const double g = 9.81;
  const double m = foundAmmo->mass, d = foundAmmo->drag, l = foundAmmo->lift;
  const double m2 = m * m, d2 = d * d, l2 = l * l;
  const double m3 = m2 * m, d3 = d2 * d, l3 = l2 * l;
  const double m4 = m3 * m, d4 = d3 * d, l4 = l3 * l;

  const double a = d * g * m - 2.0 * d2 * l * input.attack_speed;
  const double b = -3.0 * g * m2 + 3.0 * d * l * m * input.attack_speed;
  const double c = 6.0 * m2 * input.altitude;

  const double p = -b * b / (3.0 * a * a);
  const double q = 2.0 * b * b * b / (27.0 * a * a * a) + c / a;
  const double phi_cos = (3.0 * q) / (2.0 * p) * std::sqrt(-3.0 / p);

  const double phi = std::acos(phi_cos);
  if (std::isnan(phi) || errno == EDOM) {
    return BallisticsSolution{.result = SolutionResult::OtherError};
  }

  const double t = 2.0 * std::sqrt(-p / 3.0) * std::cos((phi + 4.0 * M_PI) / 3.0) - (b / (3.0 * a));
  if (t < 0.0) {
    return BallisticsSolution{.result = SolutionResult::OtherError};
  }

  double t_p = t;
  double fl = input.attack_speed * t_p;

  t_p *= t;
  fl -= t_p * d * input.attack_speed / (2.0 * m);

  t_p *= t;
  fl += t_p * (6.0 * d * g * l * m - 6.0 * d * d * (l2 - 1.0) * input.attack_speed) / (36.0 * m2);

  t_p *= t;
  fl += t_p *
        (-6.0 * d2 * g * l * (1.0 + l2 + l4) * m + 3.0 * d3 * l2 * (1.0 + l2) * input.attack_speed +
         6.0 * d3 * l2 * l2 * (1.0 + l2) * input.attack_speed) /
        (36.0 * (1.0 + l2) * (1.0 + l2) * m3);

  t_p *= t;
  fl += t_p * (3.0 * d3 * g * l3 * m - 3.0 * d4 * l2 * (1.0 + l2) * input.attack_speed) / (36.0 * (1.0 + l2) * m4);

  if (fl < 0) {
    return BallisticsSolution{.result = SolutionResult::OtherError};
  }

  const double delta_x = input.target.x - input.position.x, delta_y = input.target.y - input.position.y;
  const double dist = std::sqrt(delta_x * delta_x + delta_y * delta_y);
  const double minimal_required_path = fl + input.acceleration_path;

  BallisticsSolution solution{.result = SolutionResult::Ok};
  if (dist < 1e-8) {
    solution.intermediate = Coord(input.target.x - minimal_required_path, input.target.y);
    solution.fire = Coord(input.target.x - fl, input.target.y);
  }
  else {
    if (minimal_required_path > dist) {
      solution.intermediate =
        Coord(input.target.x - delta_x * minimal_required_path / dist, input.target.y - delta_y * minimal_required_path / dist);
    }

    const double ratio = (dist - fl) / dist;
    solution.fire =
      Coord(input.position.x + (input.target.x - input.position.x) * ratio, input.position.y + (input.target.y - input.position.y) * ratio);
  }

  return solution;
}
