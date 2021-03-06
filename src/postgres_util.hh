#pragma once

extern "C" {
    #include <nodes/primnodes.h>
    #include <optimizer/clauses.h>
    #include <utils/elog.h>
}

namespace virtdb {

const Var* get_variable(const Expr* expr)
{
    switch (expr->type)
    {
        case T_TargetEntry:
            {
                const TargetEntry* target_entry = reinterpret_cast<const TargetEntry*>(expr);
                return get_variable(target_entry->expr);
            }
        case T_OpExpr:
            {
                const OpExpr* opexpr = reinterpret_cast<const OpExpr*>(expr);
                const Expr* lop = reinterpret_cast<const Expr*>(get_leftop(&opexpr->xpr));
                return get_variable(lop);
            }
        case T_RelabelType:
            {
                const RelabelType * rl = reinterpret_cast<const RelabelType*>(expr);
                return reinterpret_cast<const Var*>(rl->arg);
            }
        case T_Var:
            {
                return reinterpret_cast<const Var*>(expr);
            }
        case T_CoerceViaIO:
            {
                const CoerceViaIO* coerce = reinterpret_cast<const CoerceViaIO*>(expr);
                return reinterpret_cast<const Var*>(coerce->arg);
            }
        case T_NullTest:
            {
                const NullTest * null_test = reinterpret_cast<const NullTest*>(expr);
                return reinterpret_cast<const Var*>(null_test->arg);
            }
        default:
            elog(ERROR, "Unhandled node type in get_variable: %d", expr->type);
    }
}

} // namespace virtdb
