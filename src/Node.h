//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#ifndef __MESH_NODE_H_
#define __MESH_NODE_H_

#include <omnetpp.h>
#include <bitset>
#include <vector>
#include <math.h>
#include <stdio.h>
#include<sstream>
#include<string>
using namespace omnetpp;

/**
 * TODO - Generated class
 */
class Node : public cSimpleModule
{
  protected:
    simtime_t time; //// time of waiting.
    int window_size = 0; /// window size for sender.
    int frame_to_send = 0; //// f(n) last frame to send.
    int first_frame_sent = 0; ///// f(0) frame sent waits to ack.
    int ack_frame = 0; ///// frame receiver waits
    int send_to;
    int fSize;
    int seq_num =0;
    cMessage *timeoutEvent;
    cMessage *msg_to_send[10];

    int no_frames; //number of total frames
    int drop_fram; // number of the dropped frames.
    int ret_frames; //number of retransmitted frames.
    int no_ack; //number of ack.

    int msg_size;
    cMessage* sendM;
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void generateMessage(bool retransmit, int ack);
    virtual void sendMessage(cMessage *);
    virtual cMessage* receiveFrame(cMessage *);
    virtual cMessage* sendFrame(cMessage *);
    virtual std::string hamming_code(std::string bits, int vecSize);
    std::string corruption(std::string msgBits);
    bool probability(float threshold);
    virtual float calcStat();
    virtual std::string  removehamming(std::string msg);

};

#endif
