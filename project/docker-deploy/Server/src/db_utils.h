#ifndef DB_UTILS_H
#define DB_UTILS_H

#include <iostream>
#include <pqxx/pqxx>
#include <string>
#include <boost/thread.hpp>
#include "macros.h"

using namespace std;
using namespace pqxx;

class DbUtils {
private:
    connection* C;
    static boost::mutex db_utils_mutex;

public:
    DbUtils();
    ~DbUtils();
    void exec_transactional_sql(const string& sql);
    result exec_non_transactional_sql(const string& sql);

    result query(const string& table_name, const string& where_clause);
    void add(const string& table_name, const string& fields, const string& values);
    void update(const string& table_name, const string& set_clause, const string& where_clause);

    void add_to_package_table(const string& package_id, 
                              const string& order_id, 
                              const string& warehouse_id, 
                              const string& status, 
                              const string& description);

    inline static string getOrderTableName() {
        return "AmazonWeb_order";
    }

    inline static string getPackageTableName() {
        return "AmazonWeb_package";
    }

    inline static string getProductTableName() {
        return "AmazonWeb_product";
    }

    inline static string getProductListTableName() {
        return "AmazonWeb_productlist";
    }

    inline static string getUserTableName() {
        return "AmazonWeb_user";
    }

    inline static string getWarehouseTableName() {
        return "AmazonWeb_warehouse";
    }

    inline static string getOrderFields() {
        return "(order_id,user_id,dest_addr_x,dest_addr_y,ups_account,order_time,order_status)";
    }

    inline static string getPackageFields() {
        return "(package_id,order_id_id,warehouse_id_id,status,description)";
    }

    inline static string getProductFields() {
        return "(product_id,description,price)";
    }

    inline static string getProductListFields() {
        return "(serial_id,product_id_id,product_num,order_id_id,warehouse_id_id)";
    }

    inline static string getTruckFields() {
        return "(truck_id,warehouse_id)";
    }

    inline static string getUserFields() {
        return "(user_id,user_name)";
    }

    inline static string getWareHouseFields() {
        return "(warehouse_id,ware_addr_x,ware_addr_y,truck_status,truck_id)";
    }
};

#endif