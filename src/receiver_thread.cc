// Standard headers
#include <chrono>

// Google Protocol Buffers
#include "data.pb.h"

// ZeroMQ
#include "cppzmq/zmq.hpp"

// VirtDB headers
#include "receiver_thread.hh"
#include "data_handler.hh"
#include <logger.hh>

zmq::context_t* zmq_context = NULL;
// extern std::string column_address;

using namespace virtdb;

receiver_thread::receiver_thread()
{
    data_socket = new zmq::socket_t (*zmq_context, ZMQ_SUB);
    // data_socket->connect(column_address.c_str());
}

receiver_thread::~receiver_thread()
{
    delete data_socket;
}

void receiver_thread::add_query(const ForeignScanState* const node, const virtdb::query& query_data)
{
    LOG_TRACE("Added query: " << V_(query_data.id()));
    active_queries[node] = new data_handler(query_data);
    LOG_TRACE("Data handler created.");
    data_socket->setsockopt( ZMQ_SUBSCRIBE, query_data.id().c_str(), 0);
    LOG_TRACE("Socket options set.");
}

void receiver_thread::remove_query(const ForeignScanState* const node)
{
    data_handler* handler = active_queries[node];
    data_socket->setsockopt( ZMQ_UNSUBSCRIBE, handler->query_id().c_str(), 0);
    LOG_TRACE("Removed query: " << V_(handler->query_id()));
    active_queries.erase(node);
    delete handler;
}

void receiver_thread::stop()
{
    done = true;
}

data_handler* receiver_thread::get_data_handler(const ForeignScanState* const node)
{
    return active_queries[node];
}

data_handler* receiver_thread::get_data_handler(std::string queryid)
{
    for (auto it = active_queries.begin(); it != active_queries.end(); it++)
    {
        if (it->second->query_id() == queryid)
        {
            return it->second;
        }
    }
    return NULL;
}

bool receiver_thread::wait_for_data(const ForeignScanState* const node)
{
    data_handler* handler = get_data_handler(node);
    std::unique_lock<std::mutex> cv_lock(cv[handler->query_id()].mutex);
    while (!handler->received_data())
    {
        if (cv[handler->query_id()].variable.wait_for(cv_lock, std::chrono::milliseconds(10000)) == std::cv_status::timeout)
        {
            // elog(ERROR, "wait_for_data timed out.");
            return handler->received_data() != 0;
        }
    }
    return true;
}

void receiver_thread::run()
{
    try
    {
        zmq::message_t query_id;
        zmq::message_t update;

        while (!done)
        {
            try
            {
                // QueryID
                // data_socket->recv(&query_id);
                //
                // // Column data
                // data_socket->recv(&update);
                // virtdb::interface::pb::Column column;
                // column.ParseFromArray(update.data(), update.size());
                // data_handler* handler = get_data_handler(column.queryid());
                // std::unique_lock<std::mutex> cv_lock(cv[handler->query_id()].mutex);
                // handler->push(column.name(), column);
                // if (handler->received_data())
                // {
                //     cv[column.queryid()].variable.notify_all();
                // }
            }
            catch(const std::exception & e)
            {
                // elog(ERROR, "[%s:%d] internal error in %s: %s",__FILE__,__LINE__,__func__,e.what());
            }
        }
    }
    catch(const std::exception & e)
    {
        // elog(ERROR, "[%s:%d] internal error in %s: %s",__FILE__,__LINE__,__func__,e.what());
    }
    // elog(LOG, "Receiver stopped");
}
