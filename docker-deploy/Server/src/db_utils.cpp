#include "db_utils.h"

boost::mutex DbUtils::db_utils_mutex;

DbUtils::DbUtils() {
    try
    {
        C = new connection("host=db dbname=" + DB_NAME + " user=" + DB_USER + " password=" + DB_PASSWORD);
        if (C->is_open()) {
            // cout << "Opened database successfully: " << C->dbname() << endl;
        }
        else {
            cout << "Can't open database" << endl;
        }
    }
    catch (const std::exception &e) {
        cerr << e.what() << std::endl;
    }
}

DbUtils::~DbUtils() {
    C->disconnect();
}

void DbUtils::exec_transactional_sql(const string& sql) {
    boost::mutex::scoped_lock lock(db_utils_mutex);
    work W(*C);
    W.exec(sql);
    W.commit();
}

result DbUtils::exec_non_transactional_sql(const string& sql) {
    boost::mutex::scoped_lock lock(db_utils_mutex);
    nontransaction N(*C);
    result R(N.exec(sql));
    return R;
}

result DbUtils::query(const string& table_name, const string& where_clause) {
    string sql = "SELECT * FROM \"" + table_name + "\" WHERE " + where_clause + ";";
    return exec_non_transactional_sql(sql);
}

void DbUtils::add(const string& table_name, const string& fields, const string& values) {
    string sql = "INSERT INTO \"" + table_name + "\" " + fields + " VALUES " + values + ";";
    exec_transactional_sql(sql); 
}

void DbUtils::update(const string& table_name, const string& set_clause, const string& where_clause) {
    string sql = "UPDATE \"" + table_name + "\" SET " + set_clause + " WHERE " + where_clause + ";";
    exec_transactional_sql(sql);
}

void DbUtils::add_to_package_table(const string& package_id, 
                                   const string& order_id, 
                                   const string& warehouse_id, 
                                   const string& status, 
                                   const string& description) {
    string values = "(" + package_id + "," + order_id + "," + warehouse_id + ",\'" + status + "\',\'" + description + "\')";
    DbUtils::add(DbUtils::getPackageTableName(), DbUtils::getPackageFields(), values);
    }