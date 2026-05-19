#include <gtest/gtest.h>

#include "ballistics.hpp"
#include "coord.hpp"

TEST(Ballistics, ComputesKnownDropPoint)
{
  const BallisticsInput input{
    .position{100.0, 100.0},
    .target{200.0, 200.0},
    .altitude = 100.0,
    .attack_speed = 10.0,
    .acceleration_path = 10.0,
    .ammo_name = "VOG-17",
  };

  const BallisticsSolution solution = calculate_ballistics(input);

  EXPECT_EQ(solution.result, SolutionResult::Ok);
  EXPECT_FALSE(solution.intermediate.has_value());
  EXPECT_NEAR(solution.fire.x, 173.759, 0.01);
  EXPECT_NEAR(solution.fire.y, 173.759, 0.01);
}

TEST(Ballistics, NegativeAltitude)
{
  const BallisticsInput input{
    .position{.x = 100.0, .y = 100.0},
    .target{.x = 200.0, .y = 200.0},
    .altitude = -100.0,
    .attack_speed = 10.0,
    .acceleration_path = 10.0,
    .ammo_name = "VOG-17",
  };

  const BallisticsSolution solution = calculate_ballistics(input);

  EXPECT_EQ(solution.result, SolutionResult::NegativeAltitude);
}

TEST(Ballistics, UnknownAmmoName)
{
  const BallisticsInput input{
    .position{.x = 100.0, .y = 100.0},
    .target{.x = 200.0, .y = 200.0},
    .altitude = -100.0,
    .attack_speed = 10.0,
    .acceleration_path = 10.0,
    .ammo_name = "HEHEHE",
  };

  const BallisticsSolution solution = calculate_ballistics(input);

  EXPECT_EQ(solution.result, SolutionResult::NegativeAltitude);
}
