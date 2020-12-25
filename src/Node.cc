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

#include "Node.h"
#include "MyMessage_m.h"

Define_Module(Node);


int Node::getParityValue(int position, int totalSize,int hammingCode[100])
{
    int count = 0, i, j;
    i = position - 1;

    while (i < totalSize) {
        for (j = i; j < i + position; j++) {

            if (hammingCode[j] == 1)
                count++;
        }
        i = i + 2 * position;
    }

    if (count % 2 == 0)
        return 0;
    else
        return 1;
}
std::string Node::hamming_code(std::vector<int>input, int n)
{
    int i, p_n = 0, totalSize, j, k;
    i =j = k= 0;

    int hammingCode[100];
    std::string hammingCodestr="";

    while (n > (int)pow(2, i) - (i + 1)) {
        p_n++;
        i++;
    }

    totalSize = p_n + n;

    for (int i = 0; i < totalSize; i++) {

        if (i == ((int)pow(2, k) - 1)) {
            hammingCode[i] = 0;
            k++;
        }
        else {
            hammingCode[i] = input[j];
            j++;
        }
    }

    for (i = 0; i < p_n; i++) {
        int position = (int)pow(2, i);
        int value = getParityValue(position, totalSize,hammingCode);
        hammingCode[position - 1] = value;
    }

    for (i = 0; i < totalSize; i++) {
        std::stringstream ss;
        ss<<hammingCode[i];
        std::string s;
        ss>>s;
        hammingCodestr = hammingCodestr + s;
    }
    return  hammingCodestr;
}

void Node::initialize()
{
    double interval = exponential(1 / par("lambda").doubleValue());
    scheduleAt(simTime() + interval, new cMessage(""));
}

void Node::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage()) { //Host wants to send

        int rand, dest;
        do { //Avoid sending to yourself
            rand = uniform(0, gateSize("outs"));
        } while(rand == getIndex());

        //Calculate appropriate gate number
        dest = rand;
        if (rand > getIndex())
            dest--;

        std::stringstream ss;
        ss << rand;
        EV << "Sending "<< ss.str() <<" from source " << getIndex() << "\n";
        delete msg;

        std::string payLoad = "hi";
        int n = payLoad.length();
        char char_array[n + 1];
        strcpy(char_array, payLoad.c_str());
        MyMessage_Base * msg = new MyMessage_Base("Hello");
        msg->setM_Payload(char_array);
        msg->setM_Type(1);
        msg->setSeq_Num(12);
        std::bitset<8> temp('A');
        msg->setMycheckbits(temp);
        send(msg, "outs", dest);


        ///////////////Hammming code////////////////////////////

        std::vector <int>vecdigits;
        for (int i=0;i<n;i++) {
            std::bitset <8> chbits (char_array[i]);
            std::string bits = chbits.to_string();
            for (int i =0 ;i<bits.size();i++) {
                       if (bits[i]=='0')
                           vecdigits.push_back(0);
                       else
                           vecdigits.push_back(1);
                   }
            }

        std::string hammingCode= hamming_code(vecdigits,vecdigits.size());

        std::stringstream sstream(hammingCode);
        std::string output;
        while(sstream.good())
           {
               std::bitset<8> bits;
               sstream >> bits;
               char c = char(bits.to_ulong());
               output += c;
           }
        std::cout << hammingCode<<endl;
        std::cout << output<<endl;

        char char_array2[hammingCode.length()+1];
        strcpy(char_array2, hammingCode.c_str());
        msg->setM_Payload(char_array2);
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        double interval = exponential(1 / par("lambda").doubleValue());
        EV << ". Scheduled a new packet after " << interval << "s";
        scheduleAt(simTime() + interval, new cMessage(""));
    }
    else {
        std::vector<int> vecdigitsrecv ;

            MyMessage_Base *mmsg = check_and_cast<MyMessage_Base *>(msg);
            EV<<"received message with sequence number ...   "<<endl;
            EV << mmsg->getSeq_Num()<<endl;
            EV<<"  and payload of ... "<<endl;
            EV<< mmsg->getM_Payload()<<endl;
            EV<<"and check bits of ..."<<endl;
            EV<< mmsg->getMycheckbits().to_string();


            ////////////////////Hamming code////////////////////////////
            std::string payLoadstr = mmsg->getM_Payload();
            std::cout <<payLoadstr<<endl;
            std::string paritybits="";
            std::string realmeesagebits="";
            int n = payLoadstr.length();
            int k = 0;
               for (int i = 0; i < n; i++) {
                   if (i == ((int)pow(2, k) - 1)) {
                       paritybits=paritybits+ payLoadstr[i];
                       k++;
                   }
                   else
                       realmeesagebits= realmeesagebits+ payLoadstr[i];
                   }
             std::cout<<realmeesagebits<<endl;
             for (int i=0;i<realmeesagebits.length();i++)
             {
                 if (realmeesagebits[i] =='0')
                 {
                     vecdigitsrecv.push_back(0);
                 }
                 else
                 {
                     vecdigitsrecv.push_back(1);
                 }
             }

             std::string hammingCoderecv= hamming_code(vecdigitsrecv,vecdigitsrecv.size());
             std::cout<<endl<<hammingCoderecv<<endl;

             k = 0;
             std::string calculatedParitystr="";
             for (int i = 0; i < n; i++) {
                 if (i == ((int)pow(2, k) - 1)) {
                     calculatedParitystr=calculatedParitystr+hammingCoderecv[i];
                    k++;
                 }
             }
             int result = paritybits.compare(calculatedParitystr);
             if (result!=0)
             {
                 std::cout<<"error detection"<<endl;
                 std::cout<<"The wrong bit is number "<<calculatedParitystr<<endl;
             }
             ////////////////////Hamming code////////////////////////////

            bubble("Message received");
            delete msg;
    }
}

