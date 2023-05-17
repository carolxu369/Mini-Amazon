#ifndef PROTO_UTILS_H
#define PROTO_UTILS_H

#include <string>
#include <boost/asio.hpp>

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include "protobuf/world_amazon.pb.h"
#include "protobuf/amazon_ups.pb.h"

#include "db_utils.h"
using std::string;

class ProtoUtils {
public:
    static void sendProto(boost::asio::ip::tcp::socket& socket, google::protobuf::Message& message);
    static void recvProto(boost::asio::ip::tcp::socket& socket, google::protobuf::Message& message);
    
    // ACommands
    static AConnect createAConnect();
    static AInitWarehouse* addAInitWarehouse(AConnect* aConnect, int id, int x, int y);
    static ACommands createACommands();
    static ACommands createACommandsWithAcks(AResponses* aResponses);
    static int addAck(ACommands* aCommands, int ack);
    static APurchaseMore* addAPurchaseMore(ACommands* aCommands, int warehouse_num);
    static AProduct* addAProductToAPurchaseMore(APurchaseMore* aPurchaseMore, int id, const string& description, int count);
    static APack* addAPack(ACommands* aCommands, int whnum);
    static AProduct* addAProductToAPack(APack* aPack, AProduct* aProduct);
    static APutOnTruck* addAPutOnTruck(ACommands* aCommands, int warehouse_num, int truck_id, int ship_id);

    // AUCommands
    static AUCommands createAUCommands();
    static AUCommands createAUCommandsWithAcks(UACommands* uaCommands);
    static int addAck(AUCommands* auCommands, int ack);
    static AUConnectedToWorld* addAUConnectedToWorld(AUCommands* auCommands, int world_id);
    static AUOrderCreated* addAUOrderCreated(AUCommands* auCommands, int order_id, int dest_x, int dest_y, const string& upsaccount);
    static AURequestTruck* addAURequestTruck(AUCommands* auCommands, int whnum, int x, int y);
    static AUOrderLoaded* addAUOrderLoaded(AUCommands* auCommands, int order_id, int truck_id, int package_id, const string& description);

};

#endif