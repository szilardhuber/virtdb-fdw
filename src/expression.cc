#include "expression.hh"

using namespace virtdb::interface;

namespace virtdb {

void Expression::set_variable(std::string value)
{
    self().mutable_simple()->set_variable(value);
}

const ::std::string& Expression::variable() const {
    return self().simple().variable();
}

void Expression::set_operand(std::string value) {
    self().set_operand(value);
}

const ::std::string& Expression::operand() const {
    return self().operand();
}

void Expression::set_value(std::string value) {
    self().mutable_simple()->set_value(value);
}

const ::std::string& Expression::value() const {
    return self().simple().value();
}

// Hard copies the expression from the parameter
// as to my understanding, there is no better way to do this for
// Google Protocol buffer nested classes.
void Expression::set_left(const Expression& left) {
    *self().mutable_composite()->mutable_left() = left.get_message();
}

std::unique_ptr<Expression>& Expression::left () const {
    if (!_left && self().composite().has_left())
    {
        pb::Expression& self = (expr != nullptr) ? *expr : *weak; // HACK
        _left.reset( new Expression(self.mutable_composite()->mutable_left()) );
    }
    return _left;
}

// Hard copies the expression from the parameter
// as to my understanding, there is no better way to do this for
// Google Protocol buffer nested classes.
void Expression::set_right(const Expression& right) {
    *self().mutable_composite()->mutable_right() = right.get_message();
}

std::unique_ptr<Expression>& Expression::right () const {
    if (!_right && self().composite().has_right())
    {
        pb::Expression& self = (expr != nullptr) ? *expr : *weak; // HACK
        _right.reset( new Expression(self.mutable_composite()->mutable_right()) );
    }
    return _right;
}


pb::Expression& Expression::get_message()
{
    return *expr;
}

const pb::Expression& Expression::get_message() const
{
    return *expr;
}

}
