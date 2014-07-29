#pragma once

#include "filter.hh"

namespace virtdb {

class BoolExprFilter : public Filter
{
public:
    virtual std::shared_ptr<Expression> Apply(const Expr* clause, const AttInMetadata* meta) override
    {
        ereport(LOG, (errmsg("Checking in BOOLEXPR filter")));
        if (!(IsA(clause, BoolExpr)))
        {
            return Filter::Apply(clause, meta);
        }
        else // Turned off as it is not working yet
        {
            const BoolExpr* bool_expression = reinterpret_cast<const BoolExpr*>(clause);

            // List	   *args;			/* arguments to this expression */
            ereport(LOG, (errmsg("xpr: %d", bool_expression->xpr.type)));
            ereport(LOG, (errmsg("boolop: %d", (int)bool_expression->boolop)));
            ereport(LOG, (errmsg("location: %d", bool_expression->location)));
            ereport(LOG, (errmsg("args->length: %d", bool_expression->args->length)));

            if (bool_expression->args->length != 2)
            {
                ereport(ERROR, (
                    errmsg("BoolExpression number of arguments is [%d] insted of 2.",
                            bool_expression->args->length)));
            }
            else {
                ListCell* current = bool_expression->args->head;
                std::shared_ptr<Expression> left = get_head()->Apply((Expr *)current->data.ptr_value, meta);
                std::shared_ptr<Expression> right = get_head()->Apply((Expr *)current->next->data.ptr_value, meta);
                std::shared_ptr<Expression> ret (new Expression);
                ret->get_message()->mutable_composite()->mutable_left()->CopyFrom(*left->get_message().get());
                *ret->get_message()->mutable_composite()->mutable_right() = *right->get_message();
                switch (bool_expression->boolop) {
                    case AND_EXPR:
                        ret->set_operand("AND");
                        break;
                    case OR_EXPR:
                        ret->set_operand("OR");
                        break;
                    case NOT_EXPR:
                        ret->set_operand("NOT");
                        break;
                    default:
                        return NULL;
                        break;
                }
                return ret;
            }

            return NULL;
        }
    }
};

} // namespace  pgext
