#include "client.h"
#include "../protobuf/world_amazon.pb.h"

Client::Client(boost::asio::io_context& io_context): io_context_(io_context), socket_(io_context) {
    socket_.connect(tcp::endpoint(boost::asio::ip::address::from_string(HOST), 8000));
}

void Client::run() {
    while (true) {
        // AConnect aConnect = ProtoUtils::createAConnect();
        // AConnected aConnected;
        // aConnect.set_worldid(971212);
        // std::cout << "Send AConnect to world..." << std::endl;
        // ProtoUtils::sendProto(socket_, aConnect);
        // ProtoUtils::recvProto(socket_, aConnected);
        // std::cout << "AConnected.result(): " << aConnected.result() << std::endl;
        // sleep(SLEEP_TIME);

        AUCommands auCommands;
        ProtoUtils::recvProto(socket_, auCommands);
        std::cout << "recved AUCommands:\n" << auCommands.DebugString() << std::endl;
    }
}

// compile: g++ -o client client.cpp ../proto_utils.cpp ../IPC_helper.cpp ../db_utils.cpp ../protobuf/*.pb.cc -std=c++11 -lpthread -lboost_thread -lprotobuf -lpqxx -lpq
int main(int argc, char** argv) {
    boost::asio::io_context io_context;
    Client client(io_context);
    client.run();
}