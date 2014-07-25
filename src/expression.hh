#pragma once

// protocol buffer
#include "proto/data.pb.h"

// standard headers
#include <memory>

namespace virtdb {
    class Expression {
        private:
            std::shared_ptr<virtdb::interface::pb::Expression> expr;

        public:
            Expression() : expr(new virtdb::interface::pb::Expression) {}
            void set_variable(std::string value) {
                expr->mutable_simple()->set_variable(value);
            }
            void set_operand(std::string value) {
                expr->set_operand(value);
            }
            void set_value(std::string value) {
                expr->mutable_simple()->set_value(value);
            }
            std::shared_ptr<virtdb::interface::pb::Expression> get_message()
            {
                return expr;
            }
    };
}
