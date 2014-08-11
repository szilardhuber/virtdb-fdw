#include <gtest/gtest.h>
#include "../src/data_handler.hh"
#include "../src/util.hh"
#include <string>
#include <iostream>

// the purpose of these tests is to make sure
// gtest includes and libs are properly integrated

namespace virtdb
{
    class data_handler_test : public ::testing::Test {
        public:
            int colname_length = 10;
        protected:
            virtual void SetUp() {
            }

        public:
            virtdb::interface::pb::Column generate_string_data(int n_rows, bool random = false, int random_length = 15)
            {
                virtdb::interface::pb::Column data;
                for (int i = 0; i < n_rows; i++)
                {
                    std::string string_value (i, 'x');
                    if (random)
                    {
                        string_value = gen_random(random_length);
                    }
                    data.mutable_data()->add_stringvalue(string_value);
                    data.mutable_data()->add_isnull(false);
                }
                return data;
            }
    };

    TEST_F(data_handler_test, errors)
    {
        int n_rows = 10;
        query the_query;
        the_query.set_table_name( "Query test" );
        the_query.add_column( gen_random(colname_length) );

        data_handler handler(the_query);
        virtdb::interface::pb::Column data = generate_string_data(n_rows, true);
        data.set_endofdata(true);
        handler.push(0, data);

        EXPECT_NO_THROW(
        {
            ASSERT_EQ(true, handler.has_data());
            ASSERT_EQ(true, handler.read_next());
            handler.get_string(0);
        });

        EXPECT_NO_THROW(
        {
            ASSERT_EQ(true, handler.is_null(1));    // no such column
            ASSERT_EQ(nullptr, handler.get_string(1));
        });
    }

    TEST_F(data_handler_test, push_test)
    {
        EXPECT_NO_THROW(
        {
            int n_rows = 1000;
            int n_columns = 10;
            query the_query;
            the_query.set_table_name( "Query test" );
            for (int i = 0; i < n_columns; i++)
            {
                the_query.add_column( gen_random(colname_length) );
            }
            data_handler handler(the_query);

            std::vector<virtdb::interface::pb::Column> columns;
            for (int i = 0; i < n_columns; i++)
            {
                virtdb::interface::pb::Column data = generate_string_data(n_rows, n_columns, true);
                data.set_endofdata(true);
                columns.push_back(data);
                handler.push(i, columns[i]);
            }

            for (int row_index = 0; row_index < n_rows; row_index++)
            {
                ASSERT_EQ(true, handler.has_data());
                ASSERT_EQ(true, handler.read_next());
                for (int column_index = 0; column_index < n_columns; column_index++)
                {
                    if (columns[column_index].data().isnull(row_index) == true)
                    {
                        ASSERT_EQ(handler.get_string(column_index), nullptr);
                    }
                    else
                    {
                        ASSERT_EQ(*handler.get_string(column_index), columns[column_index].data().stringvalue(row_index));
                    }
                }
            }
        });
    }

    TEST_F(data_handler_test, push_chunks)
    {
        EXPECT_NO_THROW(
        {
            int chunk_size = 7;
            int n_rows = 1000;
            query the_query;
            the_query.set_table_name( "Query test" );
            the_query.add_column( gen_random(colname_length) );
            data_handler handler(the_query);

            std::vector<virtdb::interface::pb::Column> chunks;
            int pushed_rows = 0;
            int chunk_index = 0;
            while (pushed_rows < n_rows)
            {
                int current_size = n_rows - pushed_rows > chunk_size ? chunk_size : n_rows - pushed_rows;
                virtdb::interface::pb::Column data = generate_string_data(current_size, 1, true);
                data.set_endofdata(false);
                pushed_rows += current_size;
                if (pushed_rows == n_rows)
                {
                    data.set_endofdata(true);
                }
                chunks.push_back(data);
                handler.push(0, chunks[chunk_index]);
                if (pushed_rows < n_rows)
                {
                    ASSERT_NE(true, handler.received_data());
                }
                else
                {
                    ASSERT_EQ(true, handler.received_data());
                }
                chunk_index++;
            }

            for (int row_index = 0; row_index < n_rows; row_index++)
            {
                ASSERT_EQ(true, handler.has_data());
                ASSERT_EQ(true, handler.read_next());
            }
        });
    }
}
