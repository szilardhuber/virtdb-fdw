#include <gtest/gtest.h>
#include "../src/expression.hh"
#include <memory>
#include <string>

// the purpose of these tests is to make sure
// gtest includes and libs are properly integrated

namespace virtdb
{
    class expression_test : public ::testing::Test {
        protected:
            virtual void SetUp() {
            }
    };

    std::shared_ptr<expression> create_simple(std::string name, std::string operand, std::string value)
    {
        std::shared_ptr<expression> simple_expression(new expression);
        simple_expression->set_variable(name);
        simple_expression->set_operand(operand);
        simple_expression->set_value(value);
        return simple_expression;
    }

    TEST_F(expression_test, ConstructionDeconstruction) {
        EXPECT_NO_THROW(
        {
            std::shared_ptr<expression> simple_expression(new expression);
            EXPECT_NE(simple_expression.get(), nullptr);

            std::shared_ptr<expression> composite_expression(new expression);
            composite_expression->set_operand("AND");
            composite_expression->set_left(*simple_expression);
            composite_expression->set_right(*simple_expression);
            EXPECT_NE(composite_expression.get(), nullptr);
            EXPECT_NE(composite_expression->left(), nullptr);
            //EXPECT_NE(composite_expression->right(), nullptr);
        });
    }

    TEST_F(expression_test, Simple) {
        EXPECT_NO_THROW(
        {
            std::string name = "UltimateAnswer";
            std::string operand = "=";
            std::string value = "42";
            std::shared_ptr<expression> simple_expression = create_simple(name, operand, value);
            EXPECT_EQ(simple_expression->variable(), name);
            EXPECT_EQ(simple_expression->operand(), operand);
            EXPECT_EQ(simple_expression->value(), value);
        });
    }

    TEST_F(expression_test, Composite) {
        EXPECT_NO_THROW(
        {
            std::string left_name = "Han";
            std::string left_operand = "Shot";
            std::string left_value = "First";
            std::shared_ptr<expression> left_expression = create_simple(left_name, left_operand, left_value);

            std::string right_name = "DOTT Quote";
            std::string right_operand = "=";
            std::string right_value = "Well, what possible harm could one insane, mutant tentacle do?";
            std::shared_ptr<expression> right_expression = create_simple(right_name, right_operand, right_value);

            std::shared_ptr<expression> composite_expression(new expression);
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
