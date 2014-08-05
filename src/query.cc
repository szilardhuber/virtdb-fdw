#include "query.hh"
#include "expression.hh"
#include "util.hh"

using namespace virtdb::interface;

namespace virtdb {

Query::Query()
{
    // TODO ? sha1 ?
    query_data->set_queryid(gen_random(32));
}

const ::std::string& Query::id() const
{
    return query_data->queryid();
}

void Query::set_table_name(std::string value)
{
    query_data->set_table(value);
}

const ::std::string& Query::table_name() const {
    return query_data->table();
}

void Query::add_column(std::string column_name)
{
    query_data->add_columns(column_name);
}

void Query::add_filter(std::shared_ptr<Expression> filter)
{
    *query_data->add_filter() = filter->get_message();
}

void Query::set_limit(uint64_t limit)
{
    query_data->set_limit( limit );
}


pb::Query& Query::get_message()
{
    return *query_data;
}

const pb::Query& Query::get_message() const
{
    return *query_data;
}

}
