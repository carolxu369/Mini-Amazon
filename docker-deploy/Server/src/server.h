#ifndef SERVER_H
#define SERVER_H

#include <iostream>
#include <vector>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <atomic>

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include "protobuf/world_amazon.pb.h"
#include "protobuf/amazon_ups.pb.h"

#include "proto_utils.h"
#include "db_utils.h"
#include "event_handlers.h"
#include "IPC_helper.h"
#include "macros.h"

using boost::asio::ip::tcp;
using std::vector;

class Server {
private:
    boost::asio::io_context& io_context_;
    tcp::socket world_sock_;
    tcp::socket ups_sock_;
    tcp::acceptor acceptor_;
    int world_id;
    
    void init_connect_to_world();
    void recv_from_the_world();
    void process_the_world();
    
    void init_connect_to_ups();
    void recv_from_ups();
    void process_ups();

public:
    Server(boost::asio::io_context& io_context): io_context_(io_context), 
                                                 world_sock_(io_context), 
                                                 ups_sock_(io_context), 
                                                 acceptor_(io_context, tcp::endpoint(tcp::v4(), LISTEN_PORT)) {}
    void run();
};

#endif