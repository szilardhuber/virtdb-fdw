#include <gtest/gtest.h>
#include <vector>

// the purpose of these tests is to make sure
// gtest includes and libs are properly integrated

namespace
{
  class SimpleTest : public ::testing::Test {
  protected:
    virtual void SetUp() {
      iv_.push_back(1);
      iv_.push_back(2);
      iv_.push_back(3);
    }

    size_t size() { return iv_.size(); }

    // virtual void TearDown() {}
    std::vector<int> iv_;
  };
}

TEST_F(SimpleTest, Size) {
  EXPECT_EQ(3, iv_.size());
  EXPECT_EQ(3, size());
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

