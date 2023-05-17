#include "IPC_helper.h"

std::atomic<unsigned int> IpcHelper::unique_seq_num(0);
std::atomic<unsigned int> IpcHelper::unique_ship_id(100000);

void IpcHelper::add_to_unacked_acommands(unsigned int seq_num, ACommands aCommands) {
    boost::mutex::scoped_lock lock(unacked_acommands_mutex);
    unacked_acommands.insert({seq_num, aCommands});
}

void IpcHelper::remove_from_unacked_acommands(unsigned int ack) {
    boost::mutex::scoped_lock lock(unacked_acommands_mutex);
    unacked_acommands.erase(ack);
}

vector<ACommands> IpcHelper::get_all_unacked_acommands() {
    boost::mutex::scoped_lock lock(unacked_acommands_mutex);
    vector<ACommands> all_unacked_acommands;
    unordered_map<unsigned int, ACommands>::const_iterator it;
    for (it = unacked_acommands.begin(); it != unacked_acommands.end(); ++it) {
        all_unacked_acommands.push_back(it->second);
    }
    return all_unacked_acommands;
}

void IpcHelper::add_to_unacked_aucommands(unsigned int seq_num, AUCommands auCommands) {
    boost::mutex::scoped_lock lock(unacked_aucommands_mutex);
    unacked_aucommands.insert({seq_num, auCommands});
}

void IpcHelper::remove_from_unacked_aucommands(unsigned int ack) {
    boost::mutex::scoped_lock lock(unacked_aucommands_mutex);
    unacked_aucommands.erase(ack);
}

vector<AUCommands> IpcHelper::get_all_unacked_aucommands() {
    boost::mutex::scoped_lock lock(unacked_aucommands_mutex);
    vector<AUCommands> all_unacked_aucommands;
    unordered_map<unsigned int, AUCommands>::const_iterator it;
    for (it = unacked_aucommands.begin(); it != unacked_aucommands.end(); ++it) {
        all_unacked_aucommands.push_back(it->second);
    }
    return all_unacked_aucommands;
}

unsigned int IpcHelper::get_unique_seq_num() {
    boost::mutex::scoped_lock lock(unique_seq_num_mutex);
    unsigned int seq_num = unique_seq_num;
    unique_seq_num++;
    return seq_num;
}

unsigned int IpcHelper::get_unique_ship_id() {
    boost::mutex::scoped_lock lock(unique_ship_id_mutex);
    unsigned int ship_id = unique_ship_id;
    unique_ship_id++;
    return ship_id;
}

void IpcHelper::add_recved_aresponses(AResponses aResponses) {
    boost::mutex::scoped_lock lock(recved_aresponses_mutex);
    recved_aresponses.push(aResponses);
}

bool IpcHelper::get_recved_aresponses(AResponses& aResponses) {
    boost::mutex::scoped_lock lock(recved_aresponses_mutex);
    if (recved_aresponses.empty()) {
        return false;
    }
    aResponses.CopyFrom(recved_aresponses.front());
    recved_aresponses.pop();
    return true;
}

void IpcHelper::add_recved_uacommands(UACommands uaCommands) {
    boost::mutex::scoped_lock lock(recved_uacommands_mutex);
    recved_uacommands.push(uaCommands);
}

bool IpcHelper::get_recved_uacommands(UACommands& uaCommands) {
    boost::mutex::scoped_lock lock(recved_uacommands_mutex);
    if (recved_uacommands.empty()) {
        return false;
    }
    uaCommands.CopyFrom(recved_uacommands.front());
    recved_uacommands.pop();
    return true;
}

void IpcHelper::add_to_recved_seq_nums_from_the_world(unsigned int seq_num) {
    boost::mutex::scoped_lock lock(recved_seq_nums_from_the_world_mutex);
    recved_seq_nums_from_the_world.insert(seq_num);
}

bool IpcHelper::check_is_contained_recved_seq_nums_from_the_world(unsigned int seq_num) {
    boost::mutex::scoped_lock lock(recved_seq_nums_from_the_world_mutex);
    return recved_seq_nums_from_the_world.count(seq_num);
}

void IpcHelper::add_to_recved_seq_nums_from_ups(unsigned int seq_num) {
    boost::mutex::scoped_lock lock(recved_seq_nums_from_ups_mutex);
    recved_seq_nums_from_ups.insert(seq_num);
}

bool IpcHelper::check_is_contained_recved_seq_nums_from_ups(unsigned int seq_num) {
    boost::mutex::scoped_lock lock(recved_seq_nums_from_ups_mutex);
    return recved_seq_nums_from_ups.count(seq_num);
}