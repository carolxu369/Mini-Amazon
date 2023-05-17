#include "server.h"

int main() {
    boost::asio::io_context io_context;
    Server server(io_context);
    server.run();
    io_context.run();
    return 0;
}