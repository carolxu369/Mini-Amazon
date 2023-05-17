#include "proto_utils.h"
#include "IPC_helper.h"

void ProtoUtils::sendProto(boost::asio::ip::tcp::socket& socket, google::protobuf::Message& message) {
    // ref: https://stackoverflow.com/questions/4810026/sending-protobuf-messages-with-boostasio 
    boost::asio::streambuf stream_buffer;
    std::ostream output_stream(&stream_buffer);
    {
      google::protobuf::io::OstreamOutputStream raw_output_stream(&output_stream);
      google::protobuf::io::CodedOutputStream coded_output_stream(&raw_output_stream);
      coded_output_stream.WriteVarint32(message.ByteSize());

      message.SerializeToCodedStream(&coded_output_stream);
      // IMPORTANT: In order to flush a CodedOutputStream it has to be deleted,
      // otherwise a 0 bytes package is send over the wire.
    }
    boost::asio::write(socket, stream_buffer.data());
}

void ProtoUtils::recvProto(boost::asio::ip::tcp::socket& socket, google::protobuf::Message& message) {
    // ref: ChatGPT...
    google::protobuf::io::FileInputStream input_stream(socket.native_handle());
    google::protobuf::io::CodedInputStream input(&input_stream);
    uint32_t size;
    if (!input.ReadVarint32(&size)) {
        std::cerr << "ProtoUtils::recvProto input.ReadVarint32() error" << std::endl;
        return;
    }
    // Tell the stream not to read beyond that size.
    google::protobuf::io::CodedInputStream::Limit limit = input.PushLimit(size);
    // Parse the message.
    if (!message.MergeFromCodedStream(&input)) {
        std::cerr << "ProtoUtils::recvProto message.MergeFromCodedStream() error" << std::endl;
        return;
    }
    if (!input.ConsumedEntireMessage()) {
        std::cerr << "ProtoUtils::recvProto input.ConsumedEntireMessage() error" << std::endl;
        return;
    }
    // Release the limit.
    input.PopLimit(limit);
}

AConnect ProtoUtils::createAConnect() {
    AConnect aConnect;
    aConnect.set_isamazon(true);
    addAInitWarehouse(&aConnect, 0, 0, 0);
    addAInitWarehouse(&aConnect, 1, 10, 5);
    addAInitWarehouse(&aConnect, 2, 15, 20);
    addAInitWarehouse(&aConnect, 3, 35, 45);
    addAInitWarehouse(&aConnect, 4, 50, 30);

    DbUtils dbUtils;
    dbUtils.add(DbUtils::getWarehouseTableName(), DbUtils::getWareHouseFields(), "(0,0,0,\'not_requested\',null)");
    dbUtils.add(DbUtils::getWarehouseTableName(), DbUtils::getWareHouseFields(), "(1,10,5,\'not_requested\',null)");
    dbUtils.add(DbUtils::getWarehouseTableName(), DbUtils::getWareHouseFields(), "(2,15,20,\'not_requested\',null)");
    dbUtils.add(DbUtils::getWarehouseTableName(), DbUtils::getWareHouseFields(), "(3,35,45,\'not_requested\',null)");
    dbUtils.add(DbUtils::getWarehouseTableName(), DbUtils::getWareHouseFields(), "(4,50,30,\'not_requested\',null)");

    return aConnect;
}

AInitWarehouse* ProtoUtils::addAInitWarehouse(AConnect* aConnect, int id, int x, int y) {
    AInitWarehouse* aInitWarehouse = aConnect->add_initwh();
    aInitWarehouse->set_id(id);
    aInitWarehouse->set_x(x);
    aInitWarehouse->set_y(y);
    return aInitWarehouse;
}

ACommands ProtoUtils::createACommands() {
    ACommands aCommands;
    aCommands.set_simspeed(SIM_SPEED);
    return aCommands;
}

ACommands ProtoUtils::createACommandsWithAcks(AResponses* aResponses) {
    ACommands aCommands = ProtoUtils::createACommands();
    for (APurchaseMore arrived : aResponses->arrived()) {
        ProtoUtils::addAck(&aCommands, arrived.seqnum());
    }
    for (APacked ready : aResponses->ready()) {
        ProtoUtils::addAck(&aCommands, ready.seqnum());
    }
    for (ALoaded loaded : aResponses->loaded()) {
        ProtoUtils::addAck(&aCommands, loaded.seqnum());
    }
    return aCommands;
}

int ProtoUtils::addAck(ACommands* aCommands, int ack) {
    aCommands->add_acks(ack);
    return ack;
}

APurchaseMore* ProtoUtils::addAPurchaseMore(ACommands* aCommands, int warehouse_num) {
    APurchaseMore* aPurchaseMore = aCommands->add_buy();
    aPurchaseMore->set_whnum(warehouse_num);
    IpcHelper& ipcHelper = IpcHelper::getInstance();
    aPurchaseMore->set_seqnum(ipcHelper.get_unique_seq_num());
    return aPurchaseMore;
}

AProduct* ProtoUtils::addAProductToAPurchaseMore(APurchaseMore* aPurchaseMore, int id, const string& description, int count) {
    AProduct* aProduct = aPurchaseMore->add_things();
    aProduct->set_id(id);
    aProduct->set_description(description);
    aProduct->set_count(count);
    return aProduct;
}

APack* ProtoUtils::addAPack(ACommands* aCommands, int whnum) {
    APack* aPack = aCommands->add_topack();
    IpcHelper& ipcHelper = IpcHelper::getInstance();
    aPack->set_whnum(whnum);
    aPack->set_shipid(ipcHelper.get_unique_ship_id());
    aPack->set_seqnum(ipcHelper.get_unique_seq_num());
    return aPack;
}

AProduct* ProtoUtils::addAProductToAPack(APack* aPack, AProduct* aProduct) {
    AProduct* aProduct_new = aPack->add_things();
    aProduct_new->CopyFrom(*aProduct);
    return aProduct_new;
}

APutOnTruck* ProtoUtils::addAPutOnTruck(ACommands* aCommands, int warehouse_num, int truck_id, int ship_id) {
    APutOnTruck* aPutOnTruck = aCommands->add_load();
    aPutOnTruck->set_whnum(warehouse_num);
    aPutOnTruck->set_truckid(truck_id);
    aPutOnTruck->set_shipid(ship_id);
    IpcHelper& ipcHelper = IpcHelper::getInstance();
    aPutOnTruck->set_seqnum(ipcHelper.get_unique_seq_num());
    return aPutOnTruck;
}

AUCommands ProtoUtils::createAUCommands() {
    AUCommands auCommands;
    return auCommands;
}

AUCommands ProtoUtils::createAUCommandsWithAcks(UACommands* uaCommands) {
    AUCommands auCommands = ProtoUtils::createAUCommands();
    for (UAConnectedToWorld connectedtoworld : uaCommands->connectedtoworld()) {
        ProtoUtils::addAck(&auCommands, connectedtoworld.seqnum());
    }
    for (UADestinationUpdated destinationupdated : uaCommands->destinationupdated()) {
        ProtoUtils::addAck(&auCommands, destinationupdated.seqnum());
    }
    for (UATruckArrived truckarrived : uaCommands->truckarrived()) {
        ProtoUtils::addAck(&auCommands, truckarrived.seqnum());
    }
    for (UAOrderDeparture orderdeparture : uaCommands->orderdeparture()) {
        ProtoUtils::addAck(&auCommands, orderdeparture.seqnum());
    }
    for (UAOrderDelivered orderdelivered : uaCommands->orderdelivered()) {
        ProtoUtils::addAck(&auCommands, orderdelivered.seqnum());
    }
    return auCommands;
}

int ProtoUtils::addAck(AUCommands* auCommands, int ack) {
    auCommands->add_acks(ack);
    return ack;
}

AUConnectedToWorld* ProtoUtils::addAUConnectedToWorld(AUCommands* auCommands, int world_id) {
    AUConnectedToWorld* auConnectedToWorld = auCommands->add_connectedtoworld();
    auConnectedToWorld->set_worldid(world_id);
    IpcHelper& IpcHelper = IpcHelper::getInstance();
    auConnectedToWorld->set_seqnum(IpcHelper.get_unique_seq_num());
    return auConnectedToWorld;
}

AUOrderCreated* ProtoUtils::addAUOrderCreated(AUCommands* auCommands, int order_id, int dest_x, int dest_y, const string& upsaccount) {
    AUOrderCreated* auOrderCreated = auCommands->add_ordercreated();
    auOrderCreated->set_orderid(order_id);
    auOrderCreated->set_destinationx(dest_x);
    auOrderCreated->set_destinationy(dest_y);
    if (!upsaccount.empty()) {
        auOrderCreated->set_upsaccount(upsaccount);
    }
    IpcHelper& ipcHelper = IpcHelper::getInstance();
    auOrderCreated->set_seqnum(ipcHelper.get_unique_seq_num());
    return auOrderCreated;
}

AURequestTruck* ProtoUtils::addAURequestTruck(AUCommands* auCommands, int whnum, int x, int y) {
    AURequestTruck* auRequestTruck = auCommands->add_requesttruck();
    auRequestTruck->set_whnum(whnum);
    auRequestTruck->set_x(x);
    auRequestTruck->set_y(y);
    IpcHelper& ipcHelper = IpcHelper::getInstance();
    auRequestTruck->set_seqnum(ipcHelper.get_unique_seq_num());
    return auRequestTruck;
}

AUOrderLoaded* ProtoUtils::addAUOrderLoaded(AUCommands* auCommands, int order_id, int truck_id, int package_id, const string& description) {
    AUOrderLoaded* auOrderLoaded = auCommands->add_orderloaded();
    auOrderLoaded->set_orderid(order_id);
    auOrderLoaded->set_truckid(truck_id);
    auOrderLoaded->set_packageid(package_id);
    auOrderLoaded->set_description(description);
    IpcHelper& ipcHelper = IpcHelper::getInstance();
    auOrderLoaded->set_seqnum(ipcHelper.get_unique_seq_num());
    return auOrderLoaded;
}