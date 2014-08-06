#include "data_handler.hh"

namespace virtdb {

data_handler::data_handler(const query& query_data) :
    columns_count(query_data.columns_size()),
    queryid (query_data.id())
{
    for (int i = 0; i < columns_count; i++)
    {
        std::string colname = query_data.column(i);
        column_names[colname] = i;
    }
}

const std::string& data_handler::query_id() const
{
    return queryid;
}

void data_handler::push(std::string name, virtdb::interface::pb::Column new_data)
{
    push(column_names[name], new_data);
}

void data_handler::push(int column_number, virtdb::interface::pb::Column new_data)
{
    if (data.count(column_number) == 1)
    {
        std::string error_string = "Error! Column already added for this query: " + std::to_string(column_number);
        throw std::invalid_argument(error_string);
    }
    data[column_number] = new_data;
}

bool data_handler::received_data() const
{
    return data.size() == columns_count;
}


bool data_handler::has_data() const
{
    return received_data() && (cursor < data_length() - 1);
}

bool data_handler::read_next()
{
    if (!has_data())
    {
        return false;
    }
    cursor++;
    return true;
}

bool data_handler::is_null(int column_number) const
{
    if (data.count(column_number) != 1)
    {
        return true;
    }
    if (cursor < 0 || cursor >= data_length())
    {
        std::string error_string = "Error! Invalid row: " + std::to_string(cursor);
        throw std::invalid_argument(error_string);
    }
    return data.find(column_number)->second.data().isnull(cursor);
}

const std::string* const data_handler::get_string(int column_number) const
{
    if (is_null(column_number))
        return NULL;
    return &data.find(column_number)->second.data().stringvalue(cursor);
}

int data_handler::data_length() const
{
    if (received_data())
    {
        return data.begin()->second.data().isnull_size();
    }
    return 0;
}

} // namespace virtdb
