#ifndef RP_H
#define RP_H

#include "../node.h"
#include <iostream>
#include <iomanip>  // for std::setw and std::setfill
#include <vector>
#include <unordered_map>
#include <string.h>
#include <unordered_set>
#define MAX_TIME 30
#define TTL 15
class RPNode : public Node {
    /*
     * XXX
     * Add any fields, helper functions etc here
     */
    struct nodeinfo{
        MACAddress mac_entry;
        IPAddress ip_entry;
        size_t dist_entry;
        MACAddress next_hop;
        int timeToExpiry;
    };


public:
    /*
     * NOTE You may not modify the constructor of this class
     */
    std::unordered_map<MACAddress,nodeinfo> table;
    std::unordered_map<IPAddress,MACAddress> iplookup;
    RPNode(Simulation* simul, MACAddress mac, IPAddress ip) : Node(simul, mac, ip) { }

    void send_segment(IPAddress dest_ip, std::vector<uint8_t> const& segment) const override;
    void receive_packet(MACAddress src_mac, std::vector<uint8_t> packet, size_t distance) override;
    void do_periodic() override;

    std::vector<uint8_t> tableToPacket() {

        std::vector<uint8_t> packet;
        packet.push_back(0);//flag for infopacket
        packet.push_back(15);//TTL
        for (auto data : table) {
            // entry* tmp = x.second;
            std::vector<uint8_t> temp (sizeof(nodeinfo));
            memcpy(&temp[0] ,&data.second,sizeof(nodeinfo));
            for (auto u : temp) {
                packet.push_back(u);
            }
        }
        return packet;

    }


    std::string memoryToString(const void* ptr, size_t size) {
    auto* bytePtr = reinterpret_cast<const unsigned char*>(ptr);
    std::ostringstream oss;
    for (size_t i = 0; i < size; ++i) {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(bytePtr[i]) << " ";
    }
    return oss.str();
    }

};

#endif
