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

    std::shared_ptr<Expression> create_simple(std::string name, std::string operand, std::string value)
    {
        std::shared_ptr<Expression> expression(new Expression);
        expression->set_variable(name);
        expression->set_operand(operand);
        expression->set_value(value);
        return expression;
    }

    TEST_F(ExpressionTest, ConstructionDeconstruction) {
        EXPECT_NO_THROW(
        {
            std::shared_ptr<Expression> expression(new Expression);
            EXPECT_NE(expression.get(), nullptr);

            std::shared_ptr<Expression> composite_expression(new Expression);
            composite_expression->set_operand("AND");
            composite_expression->set_left(*expression);
            composite_expression->set_right(*expression);
            EXPECT_NE(composite_expression.get(), nullptr);
            EXPECT_NE(composite_expression->left(), nullptr);
            //EXPECT_NE(composite_expression->right(), nullptr);
        });
    }

    TEST_F(ExpressionTest, Simple) {
        EXPECT_NO_THROW(
        {
            std::string name = "UltimateAnswer";
            std::string operand = "=";
            std::string value = "42";
            std::shared_ptr<Expression> expression = create_simple(name, operand, value);
            EXPECT_EQ(expression->variable(), name);
            EXPECT_EQ(expression->operand(), operand);
            EXPECT_EQ(expression->value(), value);
        });
    }

    TEST_F(ExpressionTest, Composite) {
        EXPECT_NO_THROW(
        {
            std::string left_name = "Han";
            std::string left_operand = "Shot";
            std::string left_value = "First";
            std::shared_ptr<Expression> left_expression = create_simple(left_name, left_operand, left_value);

            std::string right_name = "DOTT Quote";
            std::string right_operand = "=";
            std::string right_value = "Well, what possible harm could one insane, mutant tentacle do?";
            std::shared_ptr<Expression> right_expression = create_simple(right_name, right_operand, right_value);

            std::shared_ptr<Expression> composite_expression(new Expression);
            composite_expression->set_operand("AND");
            composite_expression->set_left(*left_expression);
            composite_expression->set_right(*right_expression);

            EXPECT_EQ(left_name, composite_expression->left()->variable());
            EXPECT_EQ(left_operand, composite_expression->left()->operand());
            EXPECT_EQ(left_value, composite_expression->left()->value());

            EXPECT_EQ(composite_expression->right()->variable(), right_name);
            EXPECT_EQ(composite_expression->right()->operand(), right_operand);
            EXPECT_EQ(composite_expression->right()->value(), right_value);

        });
    }
}
