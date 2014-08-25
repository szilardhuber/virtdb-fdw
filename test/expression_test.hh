#include <gtest/gtest.h>
#include "../src/expression.hh"
#include <memory>
#include <string>
#include <iostream>

// the purpose of these tests is to make sure
// gtest includes and libs are properly integrated

namespace virtdb
{
    class expression_test : public ::testing::Test {
        protected:
            virtual void SetUp() {
            }
    };

    expression* create_simple(std::string name, std::string operand, std::string value)
    {
        expression* simple_expression = new expression;
        int id = rand() % 100000;
        simple_expression->set_variable(id, name);
        simple_expression->set_operand(operand);
        simple_expression->set_value(value);
        return simple_expression;
    }

    TEST_F(expression_test, ConstructionDeconstruction) {
        EXPECT_NO_THROW(
        {
            expression* left_expression = new expression();
            expression* right_expression = new expression();

            expression* composite_expression = new expression();
            composite_expression->set_operand("AND");

            EXPECT_EQ(true, composite_expression->set_composite(left_expression, right_expression));
            EXPECT_NE(nullptr, composite_expression);
            EXPECT_NE(nullptr, composite_expression->left());
            EXPECT_NE(nullptr, composite_expression->right());

            delete composite_expression;
        });
    }

    TEST_F(expression_test, Simple) {
        EXPECT_NO_THROW(
        {
            std::string name = "UltimateAnswer";
            std::string operand = "=";
            std::string value = "42";
            expression* simple_expression = create_simple(name, operand, value);
            EXPECT_EQ(name, simple_expression->variable());
            EXPECT_EQ(operand, simple_expression->operand());
            EXPECT_EQ(value, simple_expression->value());
        });
    }

    TEST_F(expression_test, Composite) {
        EXPECT_NO_THROW(
        {
            // add same as left and right

            std::string left_name = "Han";
            std::string left_operand = "Shot";
            std::string left_value = "First";
            expression* left_expression = create_simple(left_name, left_operand, left_value);

            std::string right_name = "DOTT Quote";
            std::string right_operand = "=";
            std::string right_value = "Well, what possible harm could one insane, mutant tentacle do?";
            expression* right_expression = create_simple(right_name, right_operand, right_value);

            std::shared_ptr<expression> composite_expression(new expression);
            composite_expression->set_operand("AND");

            ASSERT_NE(true, composite_expression->set_composite(left_expression, left_expression));
            ASSERT_NE(true, composite_expression->set_composite(NULL, left_expression));
            ASSERT_NE(true, composite_expression->set_composite(left_expression, NULL));
            EXPECT_EQ(true, composite_expression->set_composite(left_expression, right_expression));

            EXPECT_EQ(left_name, composite_expression->left()->variable());
            EXPECT_EQ(left_operand, composite_expression->left()->operand());
            EXPECT_EQ(left_value, composite_expression->left()->value());

            EXPECT_EQ(right_name, composite_expression->right()->variable());
            EXPECT_EQ(right_operand, composite_expression->right()->operand());
            EXPECT_EQ(right_value, composite_expression->right()->value());

        });
    }

    TEST_F(expression_test, GetColumnsSimple) {
        EXPECT_NO_THROW(
        {
            std::string name = "UltimateAnswer";
            std::string operand = "=";
            std::string value = "42";
            expression* simple_expression = create_simple(name, operand, value);
            ASSERT_EQ(1, simple_expression->columns().size());
            EXPECT_EQ(name, simple_expression->columns().begin()->second);
        });
    }

    TEST_F(expression_test, GetColumnsComposite) {
        EXPECT_NO_THROW(
        {
            std::string left_name = "Han";
            std::string left_operand = "Shot";
            std::string left_value = "First";
            expression* left_expression = create_simple(left_name, left_operand, left_value);

            std::string right_name = "DOTT Quote";
            std::string right_operand = "=";
            std::string right_value = "Well, what possible harm could one insane, mutant tentacle do?";
            expression* right_expression = create_simple(right_name, right_operand, right_value);

            expression* composite_expression = new expression();
            composite_expression->set_operand("OR");

            EXPECT_EQ(true, composite_expression->set_composite(left_expression, right_expression));

            ASSERT_EQ(1, left_expression->columns().size());
            EXPECT_EQ(left_name, left_expression->columns().begin()->second);
            ASSERT_EQ(1, right_expression->columns().size());
            EXPECT_EQ(right_name, right_expression->columns().begin()->second);
            ASSERT_EQ(2, composite_expression->columns().size());
            EXPECT_EQ(composite_expression->columns().find(left_expression->columns().begin()->first)->second, left_expression->columns().begin()->second );
            EXPECT_EQ(composite_expression->columns().find(right_expression->columns().begin()->first)->second, right_expression->columns().begin()->second );
        });
    }

    TEST_F(expression_test, ProtobufSimple) {
        EXPECT_NO_THROW(
        {
            std::string name = "Han";
            std::string operand = "Shot";
            std::string value = "First";
            expression* simple_expression = create_simple(name, operand, value);
            virtdb::interface::pb::Expression simple_proto = simple_expression->get_message();
            ASSERT_EQ(true, simple_proto.has_simple());
            ASSERT_EQ(true, simple_proto.has_operand());
            ASSERT_NE(true, simple_proto.has_composite());
            ASSERT_EQ(true, simple_proto.simple().has_variable());
            ASSERT_EQ(true, simple_proto.simple().has_value());
            EXPECT_EQ(name, simple_proto.simple().variable());
            EXPECT_EQ(operand, simple_proto.operand());
            EXPECT_EQ(value, simple_proto.simple().value());

        });
    }

    TEST_F(expression_test, ProtobufComposedOnce) {
        EXPECT_NO_THROW(
        {
            std::string left_name = "Han";
            std::string left_operand = "Shot";
            std::string left_value = "First";
            expression* left_expression = create_simple(left_name, left_operand, left_value);
            virtdb::interface::pb::Expression left_proto = left_expression->get_message();

            std::string right_name = "DOTT Quote";
            std::string right_operand = "=";
            std::string right_value = "Well, what possible harm could one insane, mutant tentacle do?";
            expression* right_expression = create_simple(right_name, right_operand, right_value);
            virtdb::interface::pb::Expression right_proto = right_expression->get_message();

            //
            expression* composite_expression = new expression();
            composite_expression->set_operand("OR");
            composite_expression->set_composite(left_expression, right_expression);
            virtdb::interface::pb::Expression composite_proto = composite_expression->get_message();
            ASSERT_NE(true, composite_proto.has_simple());
            ASSERT_EQ(true, composite_proto.has_composite());
            ASSERT_EQ(true, composite_proto.has_operand());
            ASSERT_EQ(true, composite_proto.composite().has_left());
            ASSERT_EQ(true, composite_proto.composite().has_right());
            ASSERT_EQ(true, composite_proto.composite().left().has_simple());
            ASSERT_EQ(true, composite_proto.composite().left().simple().has_variable());
            ASSERT_EQ(true, composite_proto.composite().left().simple().has_value());
            ASSERT_EQ(true, composite_proto.composite().left().has_operand());
            ASSERT_EQ(left_name, composite_proto.composite().left().simple().variable());
            ASSERT_EQ(left_operand, composite_proto.composite().left().operand());
            ASSERT_EQ(left_value, composite_proto.composite().left().simple().value());

            ASSERT_EQ(true, composite_proto.composite().right().has_simple());
            ASSERT_EQ(true, composite_proto.composite().right().simple().has_variable());
            ASSERT_EQ(true, composite_proto.composite().right().simple().has_value());
            ASSERT_EQ(true, composite_proto.composite().right().has_operand());
            ASSERT_EQ(right_name, composite_proto.composite().right().simple().variable());
            ASSERT_EQ(right_operand, composite_proto.composite().right().operand());
            ASSERT_EQ(right_value, composite_proto.composite().right().simple().value());

        });
    }

    TEST_F(expression_test, ProtobufComposedTwice) {
        EXPECT_NO_THROW(
        {
            std::string left_left_name = "Han";
            std::string left_left_operand = "Shot";
            std::string left_left_value = "First";
            expression* left_left_expression = create_simple(left_left_name, left_left_operand, left_left_value);

            std::string left_right_name = "DOTT Quote";
            std::string left_right_operand = "=";
            std::string left_right_value = "Well, what possible harm could one insane, mutant tentacle do?";
            expression* left_right_expression = create_simple(left_right_name, left_right_operand, left_right_value);

            expression* left_composite_expression = new expression();
            left_composite_expression->set_operand("OR");
            left_composite_expression->set_composite(left_left_expression, left_right_expression);

            std::string right_left_name = "Good";
            std::string right_left_operand = "Luck";
            std::string right_left_value = "Have Fun";
            expression* right_left_expression = create_simple(right_left_name, right_left_operand, right_left_value);

            std::string right_right_name = "Maniac Mansion Quote";
            std::string right_right_operand = "like";
            std::string right_right_value = "Hey, did anybody see that movie on television last night? These four kids went into this strange house and... uh, never mind.";
            expression* right_right_expression = create_simple(right_right_name, right_right_operand, right_right_value);

            expression* right_composite_expression = new expression();
            right_composite_expression->set_operand("OR");
            right_composite_expression->set_composite(right_left_expression, right_right_expression);

            //
            expression* composite_expression = new expression();
            composite_expression->set_operand("AND");
            composite_expression->set_composite(left_composite_expression, right_composite_expression);
            virtdb::interface::pb::Expression composite_proto = composite_expression->get_message();
            ASSERT_NE(true, composite_proto.has_simple());
            ASSERT_EQ(true, composite_proto.has_composite());
            ASSERT_EQ(true, composite_proto.has_operand());
            ASSERT_EQ(true, composite_proto.composite().has_left());
            ASSERT_EQ(true, composite_proto.composite().has_right());
            ASSERT_NE(true, composite_proto.composite().left().has_simple());
            ASSERT_NE(true, composite_proto.composite().right().has_simple());
            ASSERT_EQ(true, composite_proto.composite().left().has_composite());
            ASSERT_EQ(true, composite_proto.composite().right().has_composite());
            ASSERT_EQ(true, composite_proto.composite().left().composite().has_left());
            ASSERT_EQ(true, composite_proto.composite().left().composite().has_right());
            ASSERT_EQ(true, composite_proto.composite().right().composite().has_left());
            ASSERT_EQ(true, composite_proto.composite().right().composite().has_right());

            // left-left
            ASSERT_EQ(true, composite_proto.composite().left().composite().left().has_simple());
            ASSERT_EQ(true, composite_proto.composite().left().composite().left().simple().has_variable());
            ASSERT_EQ(true, composite_proto.composite().left().composite().left().simple().has_value());
            ASSERT_EQ(true, composite_proto.composite().left().composite().left().has_operand());
            ASSERT_EQ(left_left_name, composite_proto.composite().left().composite().left().simple().variable());
            ASSERT_EQ(left_left_operand, composite_proto.composite().left().composite().left().operand());
            ASSERT_EQ(left_left_value, composite_proto.composite().left().composite().left().simple().value());

            // left-right
            ASSERT_EQ(true, composite_proto.composite().left().composite().right().has_simple());
            ASSERT_EQ(true, composite_proto.composite().left().composite().right().simple().has_variable());
            ASSERT_EQ(true, composite_proto.composite().left().composite().right().simple().has_value());
            ASSERT_EQ(true, composite_proto.composite().left().composite().right().has_operand());
            ASSERT_EQ(left_right_name, composite_proto.composite().left().composite().right().simple().variable());
            ASSERT_EQ(left_right_operand, composite_proto.composite().left().composite().right().operand());
            ASSERT_EQ(left_right_value, composite_proto.composite().left().composite().right().simple().value());

            // right-left
            ASSERT_EQ(true, composite_proto.composite().right().composite().left().has_simple());
            ASSERT_EQ(true, composite_proto.composite().right().composite().left().simple().has_variable());
            ASSERT_EQ(true, composite_proto.composite().right().composite().left().simple().has_value());
            ASSERT_EQ(true, composite_proto.composite().right().composite().left().has_operand());
            ASSERT_EQ(right_left_name, composite_proto.composite().right().composite().left().simple().variable());
            ASSERT_EQ(right_left_operand, composite_proto.composite().right().composite().left().operand());
            ASSERT_EQ(right_left_value, composite_proto.composite().right().composite().left().simple().value());

            // right-right
            ASSERT_EQ(true, composite_proto.composite().right().composite().right().has_simple());
            ASSERT_EQ(true, composite_proto.composite().right().composite().right().simple().has_variable());
            ASSERT_EQ(true, composite_proto.composite().right().composite().right().simple().has_value());
            ASSERT_EQ(true, composite_proto.composite().right().composite().right().has_operand());
            ASSERT_EQ(right_right_name, composite_proto.composite().right().composite().right().simple().variable());
            ASSERT_EQ(right_right_operand, composite_proto.composite().right().composite().right().operand());
            ASSERT_EQ(right_right_value, composite_proto.composite().right().composite().right().simple().value());

        });
    }
}
