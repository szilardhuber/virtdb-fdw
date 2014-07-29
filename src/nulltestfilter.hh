#pragma once

#include "filter.hh"

namespace virtdb {

class NullTestFilter : public Filter
{
public:
    virtual std::shared_ptr<Expression> Apply(const Expr* clause, const AttInMetadata* meta) override
    {
        ereport(LOG, (errmsg("Checking in NULLTEST filter")));
        if (!(IsA(clause, NullTest)))
        {
            return Filter::Apply(clause, meta);
        }
        else
        {
            const NullTest * null_test  = reinterpret_cast<const NullTest*>(clause);
            size_t filter_id      = getFilterId(clause);
            char * filter_colname = 0;
            if( meta &&
                meta->tupdesc &&
                static_cast<size_t>(meta->tupdesc->natts) >= filter_id &&
                meta->tupdesc->attrs &&
                meta->tupdesc->attrs[filter_id] )
            {
                filter_colname  = meta->tupdesc->attrs[filter_id]->attname.data;
            }
            std::string filter_op       = "";
            std::string filter_val      = "NULL";
            if (null_test->nulltesttype == NullTestType::IS_NULL)
            {
                filter_op = "IS";
            }
            else
            {
                filter_op = "IS NOT";
            }
            std::shared_ptr<Expression> ret (new Expression);
            ret->set_variable(filter_colname);
            ret->set_operand(filter_op);
            ret->set_value(filter_val);
            return ret;
        }
    }

protected:
  virtual const Var* getVar(const Expr* clause) const override
  {
    const NullTest * null_test = reinterpret_cast<const NullTest*>(clause);
    return reinterpret_cast<Var*>(null_test->arg);
  }

};




} // namespace  pgext
