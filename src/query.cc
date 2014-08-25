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

void query::add_column(int column_id, std::string column_name)
{
    bool found = false;
    for (auto item : columns)
    {
        if (item.second == column_id)
        {
            found = true;
        }
    }
    if (!found)
    {
        columns[query_data->fields_size()] = column_id;
        pb::Field* field = query_data->add_fields();
        field->set_name(column_name);
        field->mutable_desc()->set_type(pb::Kind::STRING);
    }
}

void query::add_filter(std::shared_ptr<expression> filter)
{
    std::map<int, std::string> filter_columns = filter->columns();
    for (auto it = filter_columns.begin(); it != filter_columns.end(); it++)
    {
        add_column(it->first, it->second);
    }
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
