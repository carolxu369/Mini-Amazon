#include "event_handlers.h"
#include <stdlib.h>
#include <time.h> 

void EventHandlers::open_order_handler(tcp::socket& socket) {
    ACommands aCommands = ProtoUtils::createACommands();

    DbUtils dbUtils;
    result open_orders = dbUtils.query(DbUtils::getOrderTableName(), "order_status=\'open\'");
    for (const row& order : open_orders) {
        string order_id = to_string(order["order_id"]);
    
        // Create aCommands containing a APurchaseMore command
        ACommands aCommands_bak = ProtoUtils::createACommands();
        srand((unsigned)time(NULL)); 
        int warehouse_id = rand()%5;
        APurchaseMore* aPurchaseMore_bak = ProtoUtils::addAPurchaseMore(&aCommands_bak, warehouse_id);

        // Find the specific products, add AProducts to APurchaseMore
        result products = dbUtils.query(DbUtils::getProductListTableName(), "order_id_id=" + order_id);
        for (const row& product : products) {
            string product_id = to_string(product["product_id_id"]);
            string product_num = to_string(product["product_num"]);
            result product_info = dbUtils.query(DbUtils::getProductTableName(), "product_id=" + product_id);
            string description = to_string(product_info[0]["description"]);
            ProtoUtils::addAProductToAPurchaseMore(aPurchaseMore_bak, stoi(product_id), description, stoi(product_num));
        }

        // Update order status: open -> placed
        dbUtils.update(DbUtils::getOrderTableName(), "order_status = \'placed\'", "order_id=" + order_id);

        IpcHelper& ipcHelper = IpcHelper::getInstance();
        ipcHelper.add_to_unacked_acommands(aPurchaseMore_bak->seqnum(), aCommands_bak);
        APurchaseMore* aPurchaseMore = aCommands.add_buy();
        aPurchaseMore->CopyFrom(*aPurchaseMore_bak);
    }

    if (aCommands.buy_size() > 0) {
        ProtoUtils::sendProto(socket, aCommands);
        std::cout << "EventHandlers::open_order_handler() send proto:\n" << aCommands.DebugString() << std::endl;
    }
}

void EventHandlers::product_arrived_handler(tcp::socket& socket, APurchaseMore& arrived) {
    ACommands aCommands = ProtoUtils::createACommands();

    DbUtils dbUtils;
    string warehouse_num = to_string(arrived.whnum());
    for (AProduct thing : arrived.things()) {
        string product_id = to_string(thing.id());
        string product_num = to_string(thing.count());

        // Find record in ProductList
        result results = dbUtils.query(DbUtils::getProductListTableName(), "(product_id_id=" + product_id + " AND product_num=" + product_num + " AND warehouse_id_id IS NULL)");
        const row& ordered_product = results[0];
        string serial_id = to_string(ordered_product["serial_id"]);
        string order_id = to_string(ordered_product["order_id_id"]);

        // Update warehouse_id in ProductList
        dbUtils.update(DbUtils::getProductListTableName(), "warehouse_id_id=" + warehouse_num, "serial_id=" + serial_id);

        // Find record in Product
        results = dbUtils.query(DbUtils::getProductTableName(), "product_id=" + product_id);
        const row& product = results[0];
        string description = to_string(product["description"]);

        // Create aCommands containing a APack command
        ACommands aCommands_bak = ProtoUtils::createACommands();
        APack* aPack_bak = ProtoUtils::addAPack(&aCommands_bak, arrived.whnum());
        ProtoUtils::addAProductToAPack(aPack_bak, &thing);
        string ship_id = to_string(aPack_bak->shipid());

        // Add a row in Package
        dbUtils.add_to_package_table(ship_id, order_id, warehouse_num, "packing", description);

        IpcHelper& ipcHelper = IpcHelper::getInstance();
        ipcHelper.add_to_unacked_acommands(aPack_bak->seqnum(), aCommands_bak);
        APack* aPack = aCommands.add_topack();
        aPack->CopyFrom(*aPack_bak);
    }

    if (aCommands.topack_size() > 0) {
        ProtoUtils::sendProto(socket, aCommands);
        std::cout << "EventHandlers::product_arrived_handler() send proto:\n " << aCommands.DebugString() << std::endl;
    }
}

void EventHandlers::product_packed_handler(tcp::socket& socket, APacked& ready) {
    DbUtils dbUtils;
    string ship_id = to_string(ready.shipid());
    // Update package status: packing -> packed
    dbUtils.update(DbUtils::getPackageTableName(), "status=\'packed\'", "package_id=" + ship_id);
}

void EventHandlers::product_load_handler(tcp::socket& socket) {
    ACommands aCommands = ProtoUtils::createACommands();
    
    DbUtils dbUtils;
    // Get warehouses that have an arrived truck
    result warehouses = dbUtils.query(DbUtils::getWarehouseTableName(), "truck_status=\'arrived\'");
    for (const row& warehouse : warehouses) {
        string warehouse_num = to_string(warehouse["warehouse_id"]);
        string truck_id = to_string(warehouse["truck_id"]);

        // Get packages that is packed in this warehouse
        result packed_packages = dbUtils.query(DbUtils::getPackageTableName(), "warehouse_id_id=" + warehouse_num + " AND status=\'packed\'");
        IpcHelper& ipcHelper = IpcHelper::getInstance();
        for (const row& package : packed_packages) {
            string package_id = to_string(package["package_id"]);
            // Update package status: packed -> loading
            dbUtils.update(DbUtils::getPackageTableName(), "status=\'loading\',truck_id=" + truck_id, "package_id=" + package_id);

            ACommands aCommands_bak = ProtoUtils::createACommands();
            APutOnTruck* aPutOnTruck_bak = ProtoUtils::addAPutOnTruck(&aCommands_bak, stoi(warehouse_num), stoi(truck_id), stoi(package_id));        
            ipcHelper.add_to_unacked_acommands(aPutOnTruck_bak->seqnum(), aCommands_bak);
            APutOnTruck* aPutOnTruck = aCommands.add_load();
            aPutOnTruck->CopyFrom(*aPutOnTruck_bak);
        }
    }

    if (aCommands.load_size() > 0) {
        ProtoUtils::sendProto(socket, aCommands);
        std::cout << "EventHandlers::product_load_handler() send proto:\n " << aCommands.DebugString() << std::endl;
    }
}

void EventHandlers::product_loaded_handler(tcp::socket& socket, ALoaded& loaded) {
    DbUtils dbUtils;
    string ship_id = to_string(loaded.shipid());
    // Update package status: loading -> loaded
    dbUtils.update(DbUtils::getPackageTableName(), "Status=\'loaded\'", "package_id=" + ship_id);
}

void EventHandlers::destination_updated_handler(tcp::socket& socket, UADestinationUpdated& destinationupdated) {
    DbUtils dbUtils;
    string order_id = to_string(destinationupdated.orderid());
    string destinationx = to_string(destinationupdated.destinationx());
    string destinationy = to_string(destinationupdated.destinationy());
    // Update order destination
    dbUtils.update(DbUtils::getOrderTableName(), "dest_addr_x=" + destinationx + ",dest_addr_y=" + destinationy, "order_id=" + order_id);
}

void EventHandlers::truck_arrived_handler(tcp::socket& socket, UATruckArrived& truckarrived) {
    DbUtils dbUtils;
    string truckid = to_string(truckarrived.truckid());
    string warehouse_num = to_string(truckarrived.whnum());
    // Update warehouse info: truck_status: requested -> arrived, truck_id
    dbUtils.update(DbUtils::getWarehouseTableName(), "truck_status=\'arrived\',truck_id=" + truckid, "warehouse_id=" + warehouse_num);
}

void EventHandlers::order_departure_handler(tcp::socket& socket, UAOrderDeparture& orderdeparture) {
    DbUtils dbUtils;
    string order_id = to_string(orderdeparture.orderid());
    string package_id = to_string(orderdeparture.packageid());
    string tracking_num = to_string(orderdeparture.trackingnum());

    // Find the package in Package Table
    result results = dbUtils.query(DbUtils::getPackageTableName(), "package_id=" + package_id);
    row package = results[0];
    string warehouse_num = to_string(package["warehouse_id_id"]);
    // Update package tracking number
    dbUtils.update(DbUtils::getPackageTableName(), "tracking_number=" + tracking_num, "package_id=" + package_id);

    // Update truck status in Warehouse Table: arrived -> not_requested
    dbUtils.update(DbUtils::getWarehouseTableName(), "truck_status=\'not_requested\', truck_id=NULL", "warehouse_id=" + warehouse_num);
}

void EventHandlers::order_delivered_handler(tcp::socket& socket, UAOrderDelivered& orderdelivered) {
    DbUtils dbUtils;
    string package_id = to_string(orderdelivered.packageid());
    // Update package status: delivering -> delivered
    dbUtils.update(DbUtils::getPackageTableName(), "status=\'delivered\'", "package_id=" + package_id);
}

void EventHandlers::order_created_handler(tcp::socket& socket) {
    AUCommands auCommands = ProtoUtils::createAUCommands();
    
    DbUtils dbUtils;
    // Find placed orders
    result placed_orders = dbUtils.query(DbUtils::getOrderTableName(), "order_status=\'placed\'");
    for (const row& order : placed_orders) {
        string order_id = to_string(order["order_id"]);
        string dest_x = to_string(order["dest_addr_x"]);
        string dest_y = to_string(order["dest_addr_y"]);
        string upsaccount;
        if (!order["ups_account"].is_null()) {
            upsaccount = to_string(order["ups_account"]);
        }
        // Update order status: placed -> processing
        dbUtils.update(DbUtils::getOrderTableName(), "order_status=\'processing\'", "order_id=" + order_id);

        AUCommands auCommands_bak = ProtoUtils::createAUCommands();
        AUOrderCreated* auOrderCreated_bak = ProtoUtils::addAUOrderCreated(&auCommands_bak, stoi(order_id), stoi(dest_x), stoi(dest_y), upsaccount);
        IpcHelper& ipcHelper = IpcHelper::getInstance();
        ipcHelper.add_to_unacked_aucommands(auOrderCreated_bak->seqnum(), auCommands_bak);
        AUOrderCreated* auOrderCreated = auCommands.add_ordercreated();
        auOrderCreated->CopyFrom(*auOrderCreated_bak);
    }

    if (auCommands.ordercreated_size() > 0) {
        ProtoUtils::sendProto(socket, auCommands);
        std::cout << "EventHandlers::order_created_handler() send proto:\n" << auCommands.DebugString() << std::endl;
    }
}

void EventHandlers::truck_request_handler(tcp::socket& socket) {
    AUCommands auCommands = ProtoUtils::createAUCommands();
    
    DbUtils dbUtils;
    string sel_warehouse_req_truck = "SELECT DISTINCT \"AmazonWeb_warehouse\".warehouse_id,ware_addr_x,ware_addr_y FROM \"" + DbUtils::getWarehouseTableName() + "\",\"" + DbUtils::getPackageTableName() + "\" WHERE \"AmazonWeb_warehouse\".truck_status=\'not_requested\' AND \"AmazonWeb_package\".status IN (\'packing\', \'packed\') AND \"AmazonWeb_warehouse\".warehouse_id=\"AmazonWeb_package\".warehouse_id_id";
    // Find warehouses that has not required any trucks and has some packing/packed packages
    result warehouses = dbUtils.exec_non_transactional_sql(sel_warehouse_req_truck);
    for (const row& warehouse : warehouses) {
        string warehouse_num = to_string(warehouse["warehouse_id"]);
        string ware_addr_x = to_string(warehouse["ware_addr_x"]);
        string ware_addr_y = to_string(warehouse["ware_addr_y"]);
        // Update truck_status: not_requested -> requested
        dbUtils.update(DbUtils::getWarehouseTableName(), "truck_status=\'requested\'", "warehouse_id=" + warehouse_num);

        AUCommands auCommands_bak = ProtoUtils::createAUCommands();
        AURequestTruck* auRequestTruck_bak = ProtoUtils::addAURequestTruck(&auCommands_bak, stoi(warehouse_num), stoi(ware_addr_x), stoi(ware_addr_y));
        IpcHelper& ipcHelper = IpcHelper::getInstance();
        ipcHelper.add_to_unacked_aucommands(auRequestTruck_bak->seqnum(), auCommands_bak);
        AURequestTruck* auRequestTruck = auCommands.add_requesttruck();
        auRequestTruck->CopyFrom(*auRequestTruck_bak);
    }

    if (auCommands.requesttruck_size() > 0) {
        ProtoUtils::sendProto(socket, auCommands);
        std::cout << "EventHandlers::truck_request_handler() send proto:\n" << auCommands.DebugString() << std::endl;
    }
}

void EventHandlers::order_loaded_handler(tcp::socket& socket) {
    AUCommands auCommands = ProtoUtils::createAUCommands();

    DbUtils dbUtils;
    // Find loaded packages
    result loaded_packages = dbUtils.query(DbUtils::getPackageTableName(), "status=\'loaded\'");
    for (const row& package : loaded_packages) {
        string truck_id = to_string(package["truck_id"]);
        string package_id = to_string(package["package_id"]);
        string order_id = to_string(package["order_id_id"]);
        string description = to_string(package["description"]);
        
        // Update package status: loaded -> delivering
        dbUtils.update(DbUtils::getPackageTableName(), "status=\'delivering\'", "package_id=" + package_id);

        AUCommands auCommands_bak = ProtoUtils::createAUCommands();
        AUOrderLoaded* auOrderLoaded_bak = ProtoUtils::addAUOrderLoaded(&auCommands_bak, stoi(order_id), stoi(truck_id), stoi(package_id), description);
        IpcHelper &ipcHelper = IpcHelper::getInstance();
        ipcHelper.add_to_unacked_aucommands(auOrderLoaded_bak->seqnum(), auCommands_bak);
        AUOrderLoaded* auOrderLoaded = auCommands.add_orderloaded();
        auOrderLoaded->CopyFrom(*auOrderLoaded_bak);
    }

    if (auCommands.orderloaded_size() > 0) {
        ProtoUtils::sendProto(socket, auCommands);
        std::cout << "EventHandlers::order_loaded_handler() send proto:\n" << auCommands.DebugString() << std::endl;
    }
}