#include "rp.h"

#include <cassert>

void RPNode::send_segment(IPAddress dest_ip, std::vector<uint8_t> const& segment) const
{
    /*
     * XXX
     * Implement this function
     */
    //we should build packet from scratch now so header consists of flag,source ip and dest ip just like in class example
    // log("Printing sender table");
    // std::string t;
    //     for (auto x : table) {
    //         t+= "mac key: ";
    //         t += std::to_string(x.first);
    //         t += " ";
    //         nodeinfo tmp = x.second;
    //         t+= "mac entry: ";
    //         t += std::to_string(tmp.mac_entry);
    //         t += " ";
    //         t+= "ip entry: ";
    //         t += std::to_string(tmp.ip_entry);
    //         t += " ";
    //         t+= "dist entry: ";
    //         t += std::to_string(tmp.dist_entry);
    //         t += " ";
    //         t+= "hop entry: ";
    //         t += std::to_string(tmp.next_hop);
    //         t += " ";
    //         t+= "time entry: ";
    //         t += std::to_string(tmp.timeToExpiry);
    //         t += "\n";
    //     }
    // log(t);
    std::vector<uint8_t> packet (2* sizeof(IPAddress)+2);
    packet[0] = 1;
    packet[1] = TTL;
    memcpy((char*)&packet[2], &ip, sizeof(IPAddress));
    memcpy((char*)&packet[6], &dest_ip, sizeof(IPAddress));

    //now copy the actual data
    packet.insert( packet.end(), segment.begin(), segment.end() );

    //now figure out who to send it to
    if(iplookup.find(dest_ip)==iplookup.end()){
        //i have no clue where it is 
        broadcast_packet_to_all_neighbors(packet,true);
    } else {
        //mac id is known from the lookup now
        if (table.find(iplookup.at(dest_ip)) == table.end()) {
            assert(false && "error1");
        } else {
            if (table.find(iplookup.at(dest_ip))->second.timeToExpiry==-1){
                //it is an expired entry so broadcast
                broadcast_packet_to_all_neighbors(packet,true);
            } else {
                send_packet(table.find(iplookup.at(dest_ip))->second.next_hop,packet,true);
                // std::string u;
                // for (size_t i=0; i<packet.size(); i++) {
                //     if (65<= packet[i])
                //         u += (char)(packet[i]);
                //     else
                //         u += (char)(packet[i]+48);
                // }
                // log(u+"\n");
            }
        }
    }
}

void RPNode::receive_packet(MACAddress src_mac, std::vector<uint8_t> packet, size_t distance)
{
    /*
     * XXX
     * Implement this function
     */
    if(packet[0]==0){
        for( int i=2; i<packet.size();i+=sizeof(nodeinfo)){
            nodeinfo rcv;
            memcpy(&rcv,&packet[i],sizeof(nodeinfo));

            // if its already expired then no point even considering it
            if (rcv.timeToExpiry==-1)
                {
                    continue;
                    // in this even if most of the sender's msgs are time outed its own ENTRY  will always be fwded which was increase TTL anyway
                }

            if (rcv.mac_entry <=0)
                continue;

            if(table.find(rcv.mac_entry)==table.end()){
                //no prior entry existed so create a new entry
                //IMPLEMENT SPLIT HORIZON HERE SINCE INFO NOT FWDED TO NEXT HOP SO CHECK IF NEXT_HOP IN RCV IS SAME AS OUR MAC AND IF SO THEN IGNORE
                if(table[rcv.mac_entry].next_hop!=mac){
                    table[rcv.mac_entry]=rcv;
                    table[rcv.mac_entry].dist_entry+=distance;
                    table[rcv.mac_entry].next_hop=src_mac;

                    iplookup[rcv.ip_entry]=rcv.mac_entry;
                }
            } else {
                // any entry already existed so update accordingly
                 //now what if smaller path is found in between
                if(distance+rcv.dist_entry<table[rcv.mac_entry].dist_entry || table[rcv.mac_entry].timeToExpiry < 0){
                    //second case is // the original info has expired so take up whatever new path is offered
                    table[rcv.mac_entry]=rcv;
                    table[rcv.mac_entry].dist_entry+=distance;
                    table[rcv.mac_entry].next_hop=src_mac;
                } 
                // else if 
                // //now if you received a message synch to an entry just reset TTL
                // (table[rcv.mac_entry].timeToExpiry < 0){
                //     // the original info has expired so take up whatever new path is offered
                //     table[rcv.mac_entry]=rcv;
                //     table[rcv.mac_entry].dist_entry+=distance;
                //     table[rcv.mac_entry].next_hop=src_mac;
                // }
                else 
                if (src_mac== table[rcv.mac_entry].next_hop){
                    table[rcv.mac_entry].timeToExpiry = rcv.timeToExpiry;
                    table[rcv.mac_entry].dist_entry = rcv.dist_entry + distance;
                    //update the dist as well simce what if dist goes down but you know this is trusted until you receieve a shorter path
                }
               
                
            }
        }
    } else {
        IPAddress src;
        memcpy(&src,&packet[2],sizeof(IPAddress));
        IPAddress dst;
        memcpy(&dst,&packet[6],sizeof(IPAddress));
        // log("Printing rcv table");
        // log(tableToString());
    //     log(std::to_string(dst)+"\n");
    //     log(std::to_string(ip)+"\n");
    //     log("Memory of ip: " + memoryToString(&ip, sizeof(IPAddress))+"\n");
    // log("Memory of dst: " + memoryToString(&dst, sizeof(IPAddress))+"\n");
        if(dst==ip){
            //msg was for me
            log("aya");
            std::vector<uint8_t> info;
            for (size_t i=10; i<packet.size(); i++) {
                info.push_back(packet[i]);
            }
            receive_segment(src,info);
        } else {
            packet[1]--;
            if(packet[1]==0){
                //expire kardo do nothing
            } else
            if (iplookup.find(dst) == iplookup.end()) {
                // broadcast_packet_to_all_neighbors(packet,true);
                assert(false && "error2");
            } else {
                MACAddress dst_mac=iplookup[dst];
                if(table.find(dst_mac) == table.end()){
                    broadcast_packet_to_all_neighbors(packet,true);
                }
                else if (table[dst_mac].timeToExpiry == -1) {
                    broadcast_packet_to_all_neighbors(packet,true);
                } else {
                    send_packet(table[dst_mac].next_hop,packet,true);
                }
            }
        }
    }

}

void RPNode::do_periodic()
{
    /*
     * XXX
     * Implement this function
     */
    //check if the table is empty and if so make one entry for yourself
    if (table.find(mac)==table.end()){
        nodeinfo entry;
        entry.mac_entry=mac;
        entry.ip_entry=ip;
        entry.next_hop=mac;
        entry.timeToExpiry=MAX_TIME;
        entry.dist_entry=0;
        table[mac]=entry;
        iplookup[ip]=mac;
    } else {
        //simply update expiry time for all
        for (auto& x : table) {
            if(x.first==mac)
                continue;
            else if (x.second.timeToExpiry>=0)
                x.second.timeToExpiry--;
            else {
                //expired entry with timeToExpiry set as -1 so expired using this as flag
                
            }
        }
        
    }
    broadcast_packet_to_all_neighbors(tableToPacket(),false);
    // log("Printing lookup");
    // log(lookupToString());
}

