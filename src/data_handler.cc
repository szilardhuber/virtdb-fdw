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
    if (data.count(column_number) != 1)
    {
        data[column_number] = std::vector<virtdb::interface::pb::Column>();
    }
    data[column_number].push_back(new_data);
}

bool data_handler::received_data() const
{
    if (data.size() != columns_count)
    {
        return false;
    }
    int ended = 0;
    for (auto it = data.begin(); it != data.end(); it++)
    {
        for (auto inner_it = it->second.begin(); inner_it != it->second.end(); inner_it++)
        {
            if (inner_it->endofdata())
            {
                ended += 1;
                break;
            }
        }
    }
    return ended == columns_count;
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
    if (inner_cursor >= data.begin()->second[current_chunk].data().isnull_size() - 1)
    {
        inner_cursor = -1;
        current_chunk += 1;
    }
    inner_cursor++;
    cursor++;
    return true;
}

bool data_handler::is_null(int column_number) const
{
    try {
        if (data.count(column_number) != 1)
        {
            return true;
        }
        if (cursor < 0 || cursor >= data_length())
        {
            std::string error_string = "Error! Invalid row: " + std::to_string(cursor);
            throw std::invalid_argument(error_string);
        }
        return data.find(column_number)->second[current_chunk].data().isnull(inner_cursor);
    }
    catch(const ::google::protobuf::FatalException & e)
    {
        std::string error_string = "Error [" + std::to_string(__LINE__) + "]! Inner_cursor: " + std::to_string(inner_cursor) + " " + e.what();
        throw std::invalid_argument(error_string);
    }
}

const std::string* const data_handler::get_string(int column_number) const
{
    try {
        if (is_null(column_number))
            return NULL;
        return &data.find(column_number)->second[current_chunk].data().stringvalue(inner_cursor);
    }
    catch(const ::google::protobuf::FatalException & e)
    {
        std::string error_string = "Error [" + std::to_string(__LINE__) + "]! Inner_cursor: " + std::to_string(inner_cursor) + " " + e.what();
        throw std::invalid_argument(error_string);
    }
}

int data_handler::data_length() const
{
    int count = 0;
    if (received_data())
    {
        for (auto it = data.begin()->second.begin(); it != data.begin()->second.end(); it++)
        {
            count += it->data().isnull_size();
        }
    }
    return count;
}

} // namespace virtdb
