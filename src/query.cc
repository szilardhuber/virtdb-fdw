#include "query.hh"
#include "expression.hh"
#include "util.hh"

using namespace virtdb::interface;

namespace virtdb {

query::query()
{
    // TODO ? sha1 ?
    query_data->set_queryid(gen_random(32));
}

const ::std::string& query::id() const
{
    return query_data->queryid();
}

void query::set_table_name(std::string value)
{
    query_data->set_table(value);
}

const ::std::string& query::table_name() const {
    return query_data->table();
}

void query::add_column(std::string column_name)
{
    query_data->add_columns(column_name);
}

void query::add_filter(std::shared_ptr<expression> filter)
{
    *query_data->add_filter() = filter->get_message();
}

void query::set_limit(uint64_t limit)
{
    query_data->set_limit( limit );
}


pb::Query& query::get_message()
{
    return *query_data;
}

const pb::Query& query::get_message() const
{
    return *query_data;
}

}
