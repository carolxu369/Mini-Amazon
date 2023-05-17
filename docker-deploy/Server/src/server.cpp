#include "server.h"

void Server::run() {
    // Connect to the world
    init_connect_to_world();
    // Accept connection from UPS and sync worldid
    init_connect_to_ups();

    boost::thread thr_recv_from_the_world(boost::bind(&Server::recv_from_the_world, this));
    boost::thread thr_process_the_world(boost::bind(&Server::process_the_world, this));
    boost::thread thr_recv_from_ups(boost::bind(&Server::recv_from_ups, this));
    boost::thread thr_process_ups(boost::bind(&Server::process_ups, this));

    thr_recv_from_the_world.join();
    thr_process_the_world.join();
    thr_recv_from_ups.join();
    thr_process_ups.join();
}

void Server::init_connect_to_world() {
    // socket connect to the world
    tcp::endpoint ep_world(boost::asio::ip::address::from_string(WORLD_IP), WORLD_PORT);
    world_sock_.connect(ep_world);

    // Send AConnect to the world until success
    AConnect aConnect = ProtoUtils::createAConnect();
    AConnected aConnected;
    do {
        std::cout << "Send AConnect to world..." << std::endl;
        ProtoUtils::sendProto(world_sock_, aConnect);
        ProtoUtils::recvProto(world_sock_, aConnected);
        world_id = aConnected.worldid();
        std::cout << "AConnected.world_id(): " << aConnected.worldid() << " AConnected.result(): " << aConnected.result() << std::endl;
    } while (aConnected.result() != "connected!");
}

void Server::recv_from_the_world() {
    while (true) {
        AResponses aResponses;
        ProtoUtils::recvProto(world_sock_, aResponses);
        std::cout << "Server::recv_from_the_world() recved proto:\n" << aResponses.DebugString() << std::endl;
        IpcHelper& ipcHelper = IpcHelper::getInstance();
        // insert all acks included in this aResponeses into ipcHelper.recved_acks (unordered_set)
        for (int ack : aResponses.acks()) {
            ipcHelper.remove_from_unacked_acommands(ack);
        }
        // push this aResponses into ipcHelper.recved_aresponses (queue)
        ipcHelper.add_recved_aresponses(aResponses);
        sleep(SLEEP_TIME);
    }
}

void Server::process_the_world() {
    IpcHelper& ipcHelper = IpcHelper::getInstance();
    while (true) {
        AResponses aResponses;
        // if there is any aResponses inside the queue, process that aResponses
        if (ipcHelper.get_recved_aresponses(aResponses)) {
            // Send acks corresponding to all seq_num in this aResponses
            ACommands aCommands = ProtoUtils::createACommandsWithAcks(&aResponses);
            if (aCommands.acks_size() > 0) {
                ProtoUtils::sendProto(world_sock_, aCommands);
                std::cout << "Server::process_the_world() send acks:\n" << aCommands.DebugString() << std::endl;
            }
            // Handle arrived, ready, loaded inside aResponses
            // Check if each command's seq_num has been recved to avoid duplicate operation
            for (APurchaseMore arrived : aResponses.arrived()) {
                unsigned int seq_num = arrived.seqnum();
                if (ipcHelper.check_is_contained_recved_seq_nums_from_the_world(seq_num)) {
                    continue;
                }
                ipcHelper.add_to_recved_seq_nums_from_the_world(seq_num);
                EventHandlers::product_arrived_handler(world_sock_, arrived);
            }
            for (APacked ready : aResponses.ready()) {
                unsigned int seq_num = ready.seqnum();
                if (ipcHelper.check_is_contained_recved_seq_nums_from_the_world(seq_num)) {
                    continue;
                }
                ipcHelper.add_to_recved_seq_nums_from_the_world(seq_num);
                EventHandlers::product_packed_handler(world_sock_, ready);
            }
            for (ALoaded loaded : aResponses.loaded()) {
                unsigned int seq_num = loaded.seqnum();
                if (ipcHelper.check_is_contained_recved_seq_nums_from_the_world(seq_num)) {
                    continue;
                }
                ipcHelper.add_to_recved_seq_nums_from_the_world(seq_num);
                EventHandlers::product_loaded_handler(world_sock_, loaded);
            }
        }
        // Handle open orders in db
        EventHandlers::open_order_handler(world_sock_);

        // Handle warehouses that currently has an arrived truck
        EventHandlers::product_load_handler(world_sock_);

        sleep(SLEEP_TIME);

        // Re-send all ACommands not acked
        vector<ACommands> unacked_acommands = ipcHelper.get_all_unacked_acommands();
        for (ACommands& aCommands : unacked_acommands) {
            ProtoUtils::sendProto(world_sock_, aCommands);
            std::cout << "Server::process_the_world() resend proto:\n" << aCommands.DebugString() << std::endl;
        }
    }
}

void Server::init_connect_to_ups() {
    acceptor_.accept(ups_sock_);
    // For mock client test:
    // return;
    AUCommands auCommands = ProtoUtils::createAUCommands();
    AUConnectedToWorld *auConnectedToWorld = ProtoUtils::addAUConnectedToWorld(&auCommands, this->world_id);
    int seq_num = auConnectedToWorld->seqnum();
    bool acked = false;
    bool synced = false;
    do {
        std::cout << "Send AConnect to ups...with world_id: " << this->world_id << std::endl;
        if (!acked) {
            ProtoUtils::sendProto(ups_sock_, auCommands);
        }
        do {
            UACommands uaCommands;
            ProtoUtils::recvProto(ups_sock_, uaCommands);
            std::cout << "Server::init_connect_to_ups() recved proto:\n" << uaCommands.DebugString() << std::endl;
            if (!acked) {
                for (int ack : uaCommands.acks()) {
                    if (ack == seq_num) {
                        acked = true;
                    }
                }
            }
            if (!synced) {
                for (UAConnectedToWorld uaConnectedToWorld : uaCommands.connectedtoworld()) {
                    if (uaConnectedToWorld.worldid() == this->world_id) {
                        synced = true;
                        break;
                    }
                }
            }
        } while (acked && !synced);
    } while (!acked || !synced);
}

void Server::recv_from_ups() {
    while (true) {
        UACommands uaCommands;
        ProtoUtils::recvProto(ups_sock_, uaCommands);
        std::cout << "Server::recv_from_ups() recved proto:\n" << uaCommands.DebugString() << std::endl;
        IpcHelper& ipcHelper = IpcHelper::getInstance();
        // insert all acks included in this uaCommands into ipcHelper.recved_acks (unordered_set)
        for (int ack : uaCommands.acks()) {
            ipcHelper.remove_from_unacked_aucommands(ack);
        }
        // push this uaCommands into ipcHelper.recved_uacommands (queue)
        ipcHelper.add_recved_uacommands(uaCommands);
        sleep(SLEEP_TIME);
    }
}

void Server::process_ups() {
    IpcHelper& ipcHelper = IpcHelper::getInstance();
    while (true) {
        UACommands uaCommands;
        // if there is any uaCommands inside the queue, process that uaCommands
        if (ipcHelper.get_recved_uacommands(uaCommands)) {
            // Send acks corresponding to all seq_num in this uaCommands
            AUCommands auCommands = ProtoUtils::createAUCommandsWithAcks(&uaCommands);
            if (auCommands.acks_size() > 0) {
                ProtoUtils::sendProto(ups_sock_, auCommands);
                std::cout << "Server::process_ups() send acks:\n" << auCommands.DebugString() << std::endl;
            }
            // Handle destinationupdated, truckarrived, orderdeparture, orderdelivered inside uaCommands
            // Check if each command's seq_num has been recved to avoid duplicate operation
            for (UADestinationUpdated destinationupdated : uaCommands.destinationupdated()) {
                unsigned int seq_num = destinationupdated.seqnum();
                if (ipcHelper.check_is_contained_recved_seq_nums_from_ups(seq_num)) {
                    continue;
                }
                ipcHelper.add_to_recved_seq_nums_from_ups(seq_num);
                EventHandlers::destination_updated_handler(ups_sock_, destinationupdated);
            }
            for (UATruckArrived truckarrived : uaCommands.truckarrived()) {
                unsigned int seq_num = truckarrived.seqnum();
                if (ipcHelper.check_is_contained_recved_seq_nums_from_ups(seq_num)) {
                    continue;
                }
                ipcHelper.add_to_recved_seq_nums_from_ups(seq_num);
                EventHandlers::truck_arrived_handler(ups_sock_, truckarrived);
            }
            for (UAOrderDeparture orderdeparture : uaCommands.orderdeparture()) {
                unsigned int seq_num = orderdeparture.seqnum();
                if (ipcHelper.check_is_contained_recved_seq_nums_from_ups(seq_num)) {
                    continue;
                }
                ipcHelper.add_to_recved_seq_nums_from_ups(seq_num);
                EventHandlers::order_departure_handler(ups_sock_, orderdeparture);
            }
            for (UAOrderDelivered orderdelivered : uaCommands.orderdelivered()) {
                unsigned int seq_num = orderdelivered.seqnum();
                if (ipcHelper.check_is_contained_recved_seq_nums_from_ups(seq_num)) {
                    continue;
                }
                ipcHelper.add_to_recved_seq_nums_from_ups(seq_num);
                EventHandlers::order_delivered_handler(ups_sock_, orderdelivered);
            }
        }

        // Handle open orders in db
        EventHandlers::order_created_handler(ups_sock_);

        // Handle warehouse that needs a truck
        EventHandlers::truck_request_handler(ups_sock_);

        // Handle packages that have been loaded
        EventHandlers::order_loaded_handler(ups_sock_);

        sleep(SLEEP_TIME);
        
        // For mock client test:
        // continue;

        // Re-send all AUCommands not acked
        vector<AUCommands> unacked_aucommands = ipcHelper.get_all_unacked_aucommands();
        for (AUCommands& auCommands : unacked_aucommands) {
            ProtoUtils::sendProto(ups_sock_, auCommands);
            std::cout << "Server::process_ups() resend proto:\n" << auCommands.DebugString() << std::endl;
        }
    }
}