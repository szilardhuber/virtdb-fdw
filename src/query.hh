#pragma once

// protocol buffer
#include "proto/data.pb.h"

// standard headers
#include <memory>

namespace virtdb {

    class expression;

    class query {
        private:
            std::unique_ptr<virtdb::interface::pb::Query> query_data =
                    std::unique_ptr<virtdb::interface::pb::Query>(new virtdb::interface::pb::Query);

        public:
            query();
            query& operator=(const query& source)
            {
                if (this != &source)
                {
                    *query_data = *source.query_data;
                }
                return *this;
            }

            // Id
            const ::std::string& id() const;

            // Table
            void set_table_name(std::string value);
            const ::std::string& table_name() const;

            // Columns
            void add_column(std::string column_name);
            int columns_size() const { return query_data->fields_size(); }
            std::string column(int i) const { return query_data->fields(i).name(); }

            // Filter
            void add_filter(std::shared_ptr<expression> filter);
            const virtdb::interface::pb::Expression& get_filter(int index) { return query_data->filter(index); }

            // Limit
            void set_limit(uint64_t limit);

            // Accessing encapsulated object
            virtdb::interface::pb::Query& get_message();
            const virtdb::interface::pb::Query& get_message() const;
    };
}
