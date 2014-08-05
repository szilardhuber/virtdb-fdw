// Standard headers
#include <condition_variable>
#include <mutex>
#include <string>
#include <map>

struct ForeignScanState;
namespace virtdb
{
    class Query;
    class data_handler;
}

namespace virtdb {
    class receiver_thread
    {
        private:
            struct cv_data
            {
                std::mutex mutex;
                std::condition_variable variable;
            };

            bool done = false;
            std::map<std::string, cv_data> cv;
            std::map<const ForeignScanState* const, data_handler* > active_queries;
        public:
            void add_query(const ForeignScanState* const node, const virtdb::Query& query);
            void remove_query(const ForeignScanState* const node);
            void stop();
            data_handler* get_data_handler(const ForeignScanState* const node);
            data_handler* get_data_handler(std::string queryid);
            bool wait_for_data(const ForeignScanState* const node);
            void run();
    };
}
