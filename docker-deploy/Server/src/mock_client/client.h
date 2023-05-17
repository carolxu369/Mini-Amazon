#ifndef CLIENT_H
#define CLIENT_H

#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include "../proto_utils.h"

#define HOST "127.0.0.1"
#define PORT 8000

using boost::asio::ip::tcp;
using std::string;

class Client {
private:
    boost::asio::io_context& io_context_;
    tcp::socket socket_;

public:
    Client(boost::asio::io_context& io_context);
    void run();
};

#endif