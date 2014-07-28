#include <gtest/gtest.h>
#include "../src/expression.hh"
#include <memory>
#include <string>

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

    TEST_F(ExpressionTest, BasicFunctionality) {
        EXPECT_NO_THROW(
        {
            int a = 0;
            std::shared_ptr<Expression> expression(new Expression);
            std::string name = "VariableName";
            std::string operand = "==!=";
            std::string value = "42";
            expression->set_variable(name);
            expression->set_operand(operand);
            expression->set_value(value);
            EXPECT_EQ(expression->get_message()->mutable_simple()->variable(), name);
            EXPECT_EQ(expression->get_message()->operand(), operand);
            EXPECT_EQ(expression->get_message()->mutable_simple()->value(), value);
        });
    }
}
