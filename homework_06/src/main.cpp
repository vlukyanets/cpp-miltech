#include <iomanip>
#include <iostream>
#include <span>

#include "ballistics.hpp"
#include "coord.hpp"

static std::ostream& operator<<(std::ostream& stream, const Coord& coord)
{
  stream << coord.x << " " << coord.y;
  return stream;
}

int main(int argc, char* argv[])
{
  const auto args = std::span(argv, static_cast<std::size_t>(argc));
  if (args.size() != 2) {
    std::cerr << "Usage: ballistics_cli <input_path>\n";
    return 1;
  }

  BallisticsSolution solution = calculate_ballistics(args[1]);
  switch (solution.result) {
    case SolutionResult::Ok:
      std::cout << std::setprecision(6);
      if (solution.intermediate.has_value()) {
        std::cout << solution.intermediate.value() << " ";
      }
      std::cout << solution.fire << '\n';
      break;

    case SolutionResult::NegativeAltitude:
      std::cout << "Input data error: altitude is not a positive value\n";
      return 1;

    case SolutionResult::UnknownAmmoName:
      std::cout << "Input data error: unknown ammo name\n";
      return 1;

    case SolutionResult::BadFileData:
      std::cout << "Input data error: structure mismatches\n";
      return 1;

    case SolutionResult::FileNotExists:
      std::cout << "Input error: file not exists\n";
      return 1;

    case SolutionResult::OtherError:
      std::cout << "Unknown error\n";
      return 1;
  }

  return 0;
}
