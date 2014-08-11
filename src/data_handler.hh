#pragma once

#include <string>
#include <vector>

#include "query.hh"

namespace virtdb {
    class data_handler {
        private:
            const int columns_count;
            const std::string queryid;
            // Not a vector as we may not get the columns in order
            std::map<int, std::vector<virtdb::interface::pb::Column> > data;
            std::map<std::string, int>  column_names;
            int cursor = -1;
            int inner_cursor = -1;
            int current_chunk = 0;

            // int data_length() const;

            virtdb::interface::pb::Column& chunk(int column_index, int chunk_index)
            {
                return data.find(column_index)->second[chunk_index];
            }

        public:
            int data_length() const;
            data_handler(const query& query_data);

            const std::string& query_id() const;
            void push(std::string name, virtdb::interface::pb::Column new_data);
            void push(int column_number, virtdb::interface::pb::Column new_data);

            // Returns true if we have received data for all needed columns
            bool received_data() const;

            // Returns true if at least one row is completely available
            // and not all rows have been read.
            bool has_data() const;

            // Sets the cursor to the next row if there is more.
            // Returns false if end of daa.
            bool read_next();

            bool is_null(int column_number) const;

            // Returns the value of the given column in the actual row or NULL
            const std::string* const get_string(int column_number) const;
    };
}
