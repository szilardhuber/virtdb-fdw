#include <gtest/gtest.h>
#include "../src/data_handler.hh"
#include "../src/util.hh"
#include <string>

// the purpose of these tests is to make sure
// gtest includes and libs are properly integrated

namespace virtdb
{
    class data_handler_test : public ::testing::Test {
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
        // int n_rows = 10;
        // data_handler handler(1);
        // virtdb::interface::pb::Column data = generate_string_data(n_rows, true);
        // handler.push(0, data);
        //
        // EXPECT_NO_THROW(
        // {
        //     ASSERT_EQ(true, handler.has_data());
        //     ASSERT_EQ(true, handler.read_next());
        //     handler.get_string(0);
        // });
        //
        // EXPECT_THROW(
        // {
        //     handler.get_string(1);
        // }, std::invalid_argument);
    }

    TEST_F(data_handler_test, push_test)
    {
        // EXPECT_NO_THROW(
        // {
        //     int n_rows = 1000;
        //     int n_columns = 10;
        //     data_handler handler(n_columns);
        //     std::vector<virtdb::interface::pb::Column> columns;
        //     for (int i = 0; i < n_columns; i++)
        //     {
        //         columns.push_back(generate_string_data(n_rows, n_columns, true));
        //         handler.push(i, columns[i]);
        //     }
        //
        //     for (int row_index = 0; row_index < n_rows; row_index++)
        //     {
        //         ASSERT_EQ(true, handler.has_data());
        //         ASSERT_EQ(true, handler.read_next());
        //         for (int column_index = 0; column_index < n_columns; column_index++)
        //         {
        //             if (columns[column_index].data().isnull(row_index) == true)
        //             {
        //                 ASSERT_EQ(handler.get_string(column_index), nullptr);
        //             }
        //             else
        //             {
        //                 ASSERT_EQ(*handler.get_string(column_index), columns[column_index].data().stringvalue(row_index));
        //             }
        //         }
        //     }
        // });
    }
}
