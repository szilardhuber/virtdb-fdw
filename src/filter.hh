#pragma once

// #include <postgres_ext.h>     // Oid
// #include <nodes/primnodes.h>  // Var

namespace virtdb {

class Filter
{
private:
  Filter* head;
  Filter* next;
private:
  void Add(Filter* filter, Filter* head_)
  {
    if (next)
    {
      next->Add(filter, head_);
    }
    else
    {
      filter->head = head_;
      next = filter;
    }
  }

public:
  void Add(Filter* filter)
  {
    head = this;
    Add(filter, this);
  }
  virtual std::shared_ptr<Expression> Apply(const Expr* clause, const AttInMetadata* meta)
  {
    if (next)
    {
        return next->Apply(clause, meta);
    }
    else
    {
        return NULL;
    }
  }

protected:
  Filter* get_head() const
  {
    return head;
  }

  virtual const Var* getVar(const Expr* clause) const { return nullptr; }

  size_t getFilterId(const Expr * clause) const
  {
    size_t filter_id = 9999999;
    const Var * vp   = getVar(clause);

    if (vp && vp->varattno)
    {
      filter_id       = vp->varattno-1;
    }

    return filter_id;
  }

  std::string getFilterOp(const Oid& oid)
  {
    if (oid >= FirstBootstrapObjectId )
    {
      return "";
    }

    HeapTuple htup = SearchSysCache1(OPEROID, ObjectIdGetDatum(oid));
    if (!HeapTupleIsValid(htup))
    {
      return "";
    }

    Form_pg_operator form = reinterpret_cast<Form_pg_operator>(GETSTRUCT(htup));
    if( !form )
    {
      ReleaseSysCache(htup);
      return "";
    }

    std::string ret(form->oprname.data);
    ReleaseSysCache(htup);

    return ret;

  }

};

} // namespace  pgext
