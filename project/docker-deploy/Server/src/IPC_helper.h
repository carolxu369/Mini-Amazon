#ifndef IPC_HELPER_H
#define IPC_HELPER_H

#include <atomic>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <queue>
#include <boost/thread.hpp>

#include "protobuf/world_amazon.pb.h"
#include "protobuf/amazon_ups.pb.h"

using std::unordered_map;
using std::unordered_set;
using std::vector;
using std::queue;

class IpcHelper {
private:
    IpcHelper() {};
    ~IpcHelper() {};

    unordered_map<unsigned int, ACommands> unacked_acommands;
    boost::mutex unacked_acommands_mutex;

    unordered_map<unsigned int, AUCommands> unacked_aucommands;
    boost::mutex unacked_aucommands_mutex;

    static std::atomic<unsigned int> unique_seq_num;
    boost::mutex unique_seq_num_mutex;

    static std::atomic<unsigned int> unique_ship_id;
    boost::mutex unique_ship_id_mutex;

    queue<AResponses> recved_aresponses;
    boost::mutex recved_aresponses_mutex;

    queue<UACommands> recved_uacommands;
    boost::mutex recved_uacommands_mutex;

    unordered_set<unsigned int> recved_seq_nums_from_the_world;
    boost::mutex recved_seq_nums_from_the_world_mutex;

    unordered_set<unsigned int> recved_seq_nums_from_ups;
    boost::mutex recved_seq_nums_from_ups_mutex;

public:
    // Meyer's Singleton, thread-safe after C++11
    static IpcHelper& getInstance() {
        static IpcHelper ipcHelper;
        return ipcHelper;
    }

    IpcHelper(const IpcHelper&) = delete;
    IpcHelper& operator=(const IpcHelper&) = delete;

    void add_to_unacked_acommands(unsigned int seq_num, ACommands aCommands);
    void remove_from_unacked_acommands(unsigned int ack);
    vector<ACommands> get_all_unacked_acommands();
    void add_to_unacked_aucommands(unsigned int seq_num, AUCommands auCommands);
    void remove_from_unacked_aucommands(unsigned int ack);
    vector<AUCommands> get_all_unacked_aucommands();
    unsigned int get_unique_seq_num();
    unsigned int get_unique_ship_id();
    void add_recved_aresponses(AResponses aResponses);
    bool get_recved_aresponses(AResponses& aResponses);
    void add_recved_uacommands(UACommands uaCommands);
    bool get_recved_uacommands(UACommands& uaCommands);
    void add_to_recved_seq_nums_from_the_world(unsigned int seq_num);
    bool check_is_contained_recved_seq_nums_from_the_world(unsigned int seq_num);
    void add_to_recved_seq_nums_from_ups(unsigned int seq_num);
    bool check_is_contained_recved_seq_nums_from_ups(unsigned int seq_num);
};

#endif 