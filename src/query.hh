#pragma once

// protocol buffer
#include "proto/data.pb.h"

// standard headers
#include <memory>

namespace virtdb {

class Expression;

    class Query {
        private:
            std::unique_ptr<virtdb::interface::pb::Query> query_data =
                    std::unique_ptr<virtdb::interface::pb::Query>(new virtdb::interface::pb::Query);

        public:
            Query();

            // Table
            void set_table_name(std::string value);
            const ::std::string& table_name() const;

            // Columns
            void add_column(std::string column_name);

            // Filter
            void add_filter(std::shared_ptr<Expression> filter);
            const virtdb::interface::pb::Expression& get_filter(int index) { return query_data->filter(index); }

            // Limit
            void set_limit(uint64_t limit);

            // Accessing encapsulated object
            virtdb::interface::pb::Query& get_message();
            const virtdb::interface::pb::Query& get_message() const;
    };
}
