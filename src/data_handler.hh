#pragma once

#include <string>
#include <vector>

#include "query.hh"

namespace virtdb {
    class data_handler {
        private:
            const int columns_count;
            const std::string queryid;
            std::map<int, virtdb::interface::pb::Column> data;    // Not vector as we may not get the columns in order
            std::map<std::string, int>  column_names;
            int cursor = -1;

            inline int data_length() const
            {
                if (received_data())
                {
                    return data.begin()->second.data().isnull_size();
                }
                return 0;
            }

        public:
            data_handler(const Query& query) :
                columns_count(query.columns_size()),
                queryid (query.id())
            {
                for (int i = 0; i < columns_count; i++)
                {
                    std::string colname = query.column(i);
                    column_names[colname] = i;
                }
            }

            const std::string& query_id() const
            {
                return queryid;
            }

            void push(std::string name, virtdb::interface::pb::Column new_data)
            {
                push(column_names[name], new_data);
            }

            void push(int column_number, virtdb::interface::pb::Column new_data)
            {
                if (data.count(column_number) == 1)
                {
                    std::string error_string = "Error! Column already added for this query: " + std::to_string(column_number);
                    throw std::invalid_argument(error_string);
                }
                data[column_number] = new_data;
            }

            // Returns true if we have received data for all needed columns
            inline bool received_data() const
            {
                return data.size() == columns_count;
            }

            // Returns true if at least one row is completely available
            // and not all rows have been read.
             inline bool has_data() const
             {
                 return received_data() && (cursor < data_length() - 1);
             }

            // Sets the cursor to the next row if there is more.
            // Returns false if end of daa.
            bool read_next()
            {
                if (!has_data())
                {
                    return false;
                }
                cursor++;
                return true;
            }

            bool is_null(int column_number) const
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

            // Returns the value of the given column in the actual row or NULL
            const std::string* const get_string(int column_number) const
            {
                if (is_null(column_number))
                    return NULL;
                return &data.find(column_number)->second.data().stringvalue(cursor);
            }

    };
}
