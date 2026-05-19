#include <array>
#include <cstdlib>
#include <cstdio>
#include <sstream>
#include <string>

#include <gtest/gtest.h>

#ifndef TESTDATA_DIR
#define TESTDATA_DIR "."
#endif

static int run_cli(const char* program, const char* args, std::string& output)
{
  const std::string cmdline = std::string(program) + " " + args + " 2>&1";
  std::FILE* pipe = popen(cmdline.c_str(), "r");

  output.clear();
  std::array<char, 256> buf{};
  std::size_t bytesRead{};
  while ((bytesRead = std::fread(buf.data(), 1, buf.size(), pipe)) > 0) {
    output.append(buf.data(), bytesRead);
  }

  const int status = pclose(pipe);
  return WEXITSTATUS(status);
}

class BallisticsCliTest : public ::testing::Test {
protected:
  // NOLINTBEGIN(cppcoreguidelines-non-private-member-variables-in-classes) - GTest require to use protected variables
  int exitCode{-1};           // This value means program haven't been run
  std::string output;         // Whole output content in the string
  std::istringstream stream;  // STL way to read output
  // NOLINTEND(cppcoreguidelines-non-private-member-variables-in-classes)

  void run(const char* args)
  {
    exitCode = run_cli("./ballistics_cli", args, output);
    stream = std::istringstream(output);
  }

  void runWithFile(const char* filename)
  {
    const std::string path = std::string(TESTDATA_DIR) + "/" + filename;
    run(path.c_str());
  }
};

TEST_F(BallisticsCliTest, NoArgsPrintsUsage)
{
  run("");
  EXPECT_EQ(exitCode, 1);
  EXPECT_NE(output.find("Usage:"), std::string::npos);
}

TEST_F(BallisticsCliTest, ComputesKnownDropPoint)
{
  runWithFile("compute_known_drop_point.txt");
  EXPECT_EQ(exitCode, 0);

  // NOLINTNEXTLINE(cppcoreguidelines-init-variables,readability-isolate-declaration) - disabled because values will be read later
  double fire_x, fire_y;
  stream >> fire_x >> fire_y;
  EXPECT_NEAR(fire_x, 173.759, 0.01);
  EXPECT_NEAR(fire_y, 173.759, 0.01);
}

TEST_F(BallisticsCliTest, FileNotExists)
{
  runWithFile("non_existent_file_name");
  EXPECT_EQ(exitCode, 1);
  EXPECT_EQ(output, "Input error: file not exists\n");
}

TEST_F(BallisticsCliTest, BadInputData)
{
  runWithFile("bad_input_data.txt");
  EXPECT_EQ(exitCode, 1);
  EXPECT_EQ(output, "Input data error: structure mismatches\n");
}

TEST_F(BallisticsCliTest, NegativeAltitude)
{
  runWithFile("negative_altitude.txt");
  EXPECT_EQ(exitCode, 1);
  EXPECT_EQ(output, "Input data error: altitude is not a positive value\n");
}

TEST_F(BallisticsCliTest, UnknownAmmoName)
{
  runWithFile("unknown_ammo_name.txt");
  EXPECT_EQ(exitCode, 1);
  EXPECT_EQ(output, "Input data error: unknown ammo name\n");
}