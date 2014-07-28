#include <gtest/gtest.h>
#include "../src/expression.hh"
#include <memory>

// the purpose of these tests is to make sure
// gtest includes and libs are properly integrated

namespace virtdb
{
    class ExpressionTest : public ::testing::Test {
        protected:
            virtual void SetUp() {
            }
    };

    TEST_F(ExpressionTest, ConstructionDeconstruction) {
        EXPECT_NO_THROW(
        {
            int a = 0;
            std::shared_ptr<Expression> expression(new Expression);
            EXPECT_NE(expression.get(), nullptr);
        });
    }
}
