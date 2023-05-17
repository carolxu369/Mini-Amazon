#ifndef EVENT_HANDLERS_H
#define EVENT_HANDLERS_H

#include <string>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "db_utils.h"
#include "proto_utils.h"
#include "IPC_helper.h"
#include "macros.h"

using boost::asio::ip::tcp;

class EventHandlers {
public:
    // handler for the world process

    // For all open orders, send APurchaseMore to buy the required products
    static void open_order_handler(tcp::socket& socket); 

    // For each arrived product, update package info and send APack to pack that product (One Pack only includes one kind of product)
    static void product_arrived_handler(tcp::socket& socket, APurchaseMore& arrived); 

    // For each packed product, update the status
    static void product_packed_handler(tcp::socket& socket, APacked& ready); 

    // For each warehouse that has an arrived truck, send APutOnTruck to load the packed products
    static void product_load_handler(tcp::socket& socket);

    // For each package that is loaded, update the status
    static void product_loaded_handler(tcp::socket& socket, ALoaded& loaded);

    // handler for ups process

    // Update destination of an order
    static void destination_updated_handler(tcp::socket& socket, UADestinationUpdated& destinationupdated);

    // For each arrived truck, update the warehouse info about truck
    static void truck_arrived_handler(tcp::socket& socket, UATruckArrived& truckarrived);

    // Update status and tracking number of departured package, update truck info of warehouse
    static void order_departure_handler(tcp::socket& socket, UAOrderDeparture& orderdeparture);

    // Update package status to delivered
    static void order_delivered_handler(tcp::socket& socket, UAOrderDelivered& orderdelivered);

    // For all placed orders, send AUOrderCreated
    static void order_created_handler(tcp::socket& socket);

    // For each warehouse that needs a truck, send AURequestTruck to request for a truck
    static void truck_request_handler(tcp::socket& socket);

    // For each loaded package, update status and send AUOrderLoaded
    static void order_loaded_handler(tcp::socket& socket);
};

#endif