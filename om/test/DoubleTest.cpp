#include <OMR/Infra/BitUtilities.hpp>
#include <OMR/Infra/Double.hpp>

#include <gtest/gtest.h>

namespace OMR {
namespace Infra {
namespace Double {
namespace Test {

TEST(DoubleTest, signBit) {
  EXPECT_TRUE(std::signbit(fromRaw(Double::SIGN_MASK)));
  EXPECT_TRUE(std::signbit(fromRaw(toRaw(-0.1) & Double::SIGN_MASK)));
  EXPECT_FALSE(std::signbit(fromRaw(toRaw(0.45) & Double::SIGN_MASK)));
}

TEST(DoubleTest, Raw) {
  double d = 0.1;
  EXPECT_TRUE(fromRaw(toRaw(d)) == d);
}

TEST(DoubleTest, NaN) {
  // not NaN
  double d = 0.1;
  EXPECT_FALSE(isNaN(toRaw(d)));
  EXPECT_FALSE(isSNaN(toRaw(d)));
  EXPECT_FALSE(isQNaN(toRaw(d)));

  // qNaN
  EXPECT_TRUE(std::numeric_limits<double>::has_quiet_NaN);
  double qnan = std::numeric_limits<double>::quiet_NaN();
  EXPECT_TRUE((toRaw(qnan) & Double::NAN_QUIET_TAG) == Double::NAN_QUIET_TAG);
  EXPECT_TRUE(isNaN(toRaw(qnan)));
  EXPECT_FALSE(isSNaN(toRaw(qnan)));
  EXPECT_TRUE(isQNaN(toRaw(qnan)));

  // sNaN
  EXPECT_TRUE(std::numeric_limits<double>::has_signaling_NaN);
  double snan = std::numeric_limits<double>::signaling_NaN();
  EXPECT_TRUE((toRaw(snan) & Double::NAN_QUIET_TAG) == 0);
  EXPECT_TRUE(isNaN(toRaw(snan)));
  EXPECT_TRUE(isSNaN(toRaw(snan)));
  EXPECT_FALSE(isQNaN(toRaw(snan)));

  // infinity
  double inf = INFINITY;
  EXPECT_FALSE(isNaN(toRaw(inf)));
  EXPECT_FALSE(isSNaN(toRaw(inf)));
  EXPECT_FALSE(isQNaN(toRaw(inf)));
}

}  // namespace Test
}  // namespace Double
}  // namespace Infra
}  // namespace OMR
