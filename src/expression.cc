#include "expression.hh"

using namespace virtdb::interface;

namespace virtdb {

void expression::set_variable(std::string value)
{
    self().mutable_simple()->set_variable(value);
}

const ::std::string& expression::variable() const {
    return self().simple().variable();
}

void expression::set_operand(std::string value) {
    self().set_operand(value);
}

const ::std::string& expression::operand() const {
    return self().operand();
}

void expression::set_value(std::string value) {
    self().mutable_simple()->set_value(value);
}

const ::std::string& expression::value() const {
    return self().simple().value();
}

// Hard copies the expression from the parameter
// as to my understanding, there is no better way to do this for
// Google Protocol buffer nested classes.
void expression::set_left(const expression& left) {
    *self().mutable_composite()->mutable_left() = left.get_message();
}

std::unique_ptr<expression>& expression::left () const {
    if (!_left && self().composite().has_left())
    {
        pb::Expression& self = (expr != nullptr) ? *expr : *weak; // HACK
        _left.reset( new expression(self.mutable_composite()->mutable_left()) );
    }
    return _left;
}

// Hard copies the expression from the parameter
// as to my understanding, there is no better way to do this for
// Google Protocol buffer nested classes.
void expression::set_right(const expression& right) {
    *self().mutable_composite()->mutable_right() = right.get_message();
}

std::unique_ptr<expression>& expression::right () const {
    if (!_right && self().composite().has_right())
    {
        pb::Expression& self = (expr != nullptr) ? *expr : *weak; // HACK
        _right.reset( new expression(self.mutable_composite()->mutable_right()) );
    }
    return _right;
}


pb::Expression& expression::get_message()
{
    return *expr;
}

const pb::Expression& expression::get_message() const
{
    return *expr;
}

}
