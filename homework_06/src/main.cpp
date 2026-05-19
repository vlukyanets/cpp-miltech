#include <iostream>
#include <span>

#include "ballistics.hpp"

static std::ostream& operator<<(std::ostream& stream, const Coord& coord)
{
  stream << "(X = " << coord.x << "; Y = " << coord.y << ")";
  return stream;
}

int main(int argc, char* argv[])
{
  const auto args = std::span(argv, static_cast<std::size_t>(argc));
  if (args.size() != 2) {
    std::cerr << "usage: ballistics_cli <input_path>\n";
    return 1;
  }

  BallisticsSolution solution = calculate_ballistics(args[1]);
  switch (solution.result) {
    case SolutionResult::Ok:
      std::cout << "Solution found\n";
      std::cout << "Intermediate point: " << (solution.intermediate.has_value() ? "present" : "absent");
      if (solution.intermediate.has_value()) {
        std::cout << ": " << *solution.intermediate;
      }
      std::cout << "\nFire point: " << solution.fire << '\n';
      break;

    case SolutionResult::NegativeAltitude:
      std::cout << "Input data error: altitude is not positive value\n";
      break;

    case SolutionResult::UnknownAmmoName:
      std::cout << "Input data error: unknown ammo name\n";
      break;

    case SolutionResult::BadFileData:
      std::cout << "Input data error: structure mismatches\n";
      break;

    case SolutionResult::FileNotExists:
      std::cout << "Input error: file not exists\n";
      break;

    case SolutionResult::OtherError:
      std::cout << "Unknown error\n";
      break;
  }

  return 0;
}
