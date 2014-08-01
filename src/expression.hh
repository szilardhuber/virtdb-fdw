#pragma once

// protocol buffer
#include "proto/data.pb.h"

// standard headers
#include <memory>

namespace virtdb {
    class Expression {
        private:
            std::unique_ptr<virtdb::interface::pb::Expression> expr = nullptr;
            virtdb::interface::pb::Expression* weak;
            mutable std::unique_ptr<Expression> _left = nullptr;
            mutable std::unique_ptr<Expression> _right = nullptr;
            Expression(virtdb::interface::pb::Expression* source) : weak(source) {}
            virtdb::interface::pb::Expression& self() {
                return (expr != nullptr) ? *expr : *weak;
            }
            const virtdb::interface::pb::Expression& self() const {
                return (expr != nullptr) ? *expr : *weak;
            }

        public:
            Expression() : expr(new virtdb::interface::pb::Expression) {}

            // Expression
            void set_operand(std::string value);
            const ::std::string& operand() const;

            // SimpleExpression
            void set_variable(std::string value);
            const ::std::string& variable() const;
            void set_value(std::string value);
            const ::std::string& value() const;

            // CompositeExpression
            void set_left(const Expression& left);
            std::unique_ptr<Expression>& left () const;
            void set_right(const Expression& right);
            std::unique_ptr<Expression>& right () const;

            // Accessing encapsulated object
            virtdb::interface::pb::Expression& get_message();
            const virtdb::interface::pb::Expression& get_message() const;
    };
}
