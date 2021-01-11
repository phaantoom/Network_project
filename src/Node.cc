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

#include <fstream>
Define_Module(Node);

void Node::initialize() {

  scheduleAt(getIndex() * 10, new cMessage(""));
  window_size = 3;

  no_frames = 0; //number of total frames
  drop_fram = 0; // number of the dropped frames.
  ret_frames = 0; //number of retransmitted frames.
  no_ack = 0; //number of ack.

 // freopen("logs.txt", "a", stdout);

  time = 0.6;
  EV << "start timer\n";
  timeoutEvent = new cMessage("timeoutEvent");
}

void Node::generateMessage(bool retransmit, int ackn) {

  int count = 0;
  count = frame_to_send - first_frame_sent;
  if (retransmit) {
      std::cout << "time out for frame number " << first_frame_sent << " at node " << send_to << endl;

    for (int i = first_frame_sent; i <= frame_to_send && i < msg_size; i++) {
      no_frames++;
      ret_frames++;
     // std::cout << "ret fraaaaaams " << ret_frames << "\n";

      msg_to_send[i] -> par("ack").setLongValue(ackn);
      std::string msggg = msg_to_send[i] -> getName();
      if (msggg[10]=='0'|| msggg[10]=='0'||msggg[11]=='1'|| msggg[11]=='0' )
      {
          std ::string output= removehamming( msg_to_send[i] -> getName())  ;
                       msg_to_send[i] -> setName(output.c_str());
      }
      std::string msgtosendd = msg_to_send[i] -> getName();
      std::cout << " retransmitting message payload: " << msgtosendd.substr(8,msgtosendd.length()-9) << "  with character count: "<< msgtosendd.substr(0,8)<<endl;

      std::string msgtossend = msg_to_send[i] -> getName();
      int n = msgtossend.length();
      char char_array[n + 1];
      strcpy(char_array, msgtossend.c_str());

      std::string bits = "";
      std::string newbits = "";
      for (int i = 0; i < n; i++) {
        std::bitset < 8 > chbits(char_array[i]);
        bits = bits + chbits.to_string();
      }

      std::string hammingCode = hamming_code(bits, bits.size());
      cMessage * msgee = new cMessage(hammingCode.c_str());
      msgee -> addPar("ack");
      msgee -> addPar("seq_num");
      msgee -> addPar("first");
      int seq = msg_to_send[i] -> par("seq_num").longValue();
      int ack = msg_to_send[i] -> par("ack").longValue();

      msgee -> par("seq_num").setLongValue(seq);
      msgee -> par("ack").setLongValue(ack);
      msgee -> par("first").setLongValue(0);

      send (msgee,"outs", send_to);

       }
    scheduleAt(simTime() + time, timeoutEvent);
  } else {
    if (count <= window_size && frame_to_send < msg_size) {
      //            msg_to_send[frame_to_send]->par("seq_num").setLongValue(seq_num);
      //            EV << "gwa generate Message "<< count<< "\n";

      msg_to_send[frame_to_send] -> par("ack").setLongValue(ackn);
      msg_to_send[frame_to_send] -> par("first").setLongValue(0);

      std::string msgtossend = msg_to_send[frame_to_send] -> getName();
      ////Haaming//
      int n = msgtossend.length();
      char char_array[n + 1];
      strcpy(char_array, msgtossend.c_str());
      std::string bits = "";
      std::string newbits = "";
      for (int i = 0; i < n; i++) {
        std::bitset < 8 > chbits(char_array[i]);
        bits = bits + chbits.to_string();
      }
      for (int i = 0; i < bits.size(); i++) {
        if (bits[i] == '0')
          newbits = newbits + '0';
        else if (bits[i] == '1')
          newbits = newbits + '1';
      }
     std::string hammingCode = hamming_code(newbits, newbits.size());

      cMessage * msgee = new cMessage(hammingCode.c_str());
      msgee -> addPar("ack");
      msgee -> addPar("seq_num");
      msgee -> addPar("first");
      int seq = msg_to_send[frame_to_send] -> par("seq_num").longValue();
      int ack = msg_to_send[frame_to_send] -> par("ack").longValue();

      msgee -> par("seq_num").setLongValue(seq);
      msgee -> par("ack").setLongValue(ack);
      msgee -> par("first").setLongValue(0);
      frame_to_send++;
      count++;


      /////losssss/////
      bool probLoss = probability(20);
      bool probCorrupted = probability(80);
      if (probLoss)
        sendMessage(msgee);
      else if (probCorrupted) {
        std::cout << "Error: corrupted frame" << endl;
        std::cout << "The message before corruption:  " << msgtossend.substr(8,msgtossend.length()-9) <<"  with character count: "<<msgtossend.substr(0,8) << endl;
        std::string corruptedMsg = corruption(msgee -> getName());

        std ::string output= removehamming(corruptedMsg)  ;
        std::string str3 = output.substr(8, output.length() - 9);
        std::string charactercount = output.substr(0, 8);

        std::cout << "Msg after corruption:  " << str3 << "  with character count: "<<charactercount<< endl;
        msgee -> setName(corruptedMsg.c_str());
        sendMessage(msgee);
      } else {
        drop_fram++;
        std::cout << "Error: Loss, dropped frame" << endl;
      }
    } else {
      if (frame_to_send >= msg_size) {
        EV << "should stop here";
        std::cout<<endl<<"Statistics : "<<endl;
        ////statistics
        calcStat();
        //std::cout<<"retransmitted "<<ret_frames;

        std::cout<<endl;

        cMessage * out = new cMessage("i'm outta here!!");
        scheduleAt(simTime() + 0.1, out);

      } else {
        EV << "fff " << first_frame_sent << " ,ccc " << count << " ,sss" << frame_to_send << "\n";
        bubble("الاهتمام مبيطلبش");

      }
    }
  }

}

void Node::sendMessage(cMessage * msg) {
  //    EV << "haafsfst ack "<< msg->getName() << "\n";
    std::string correctedMsg= msg->getName();
    int n = correctedMsg.length();
    std::string correctedMsg2 = "";
           int k = 0;
           for (int i = 0; i < n; i++) {
             if (i == ((int) pow(2, k) - 1)) {
               k++;
             } else
               correctedMsg2 = correctedMsg2 + correctedMsg[i];
           }
           std::string str2 = correctedMsg2;
           std::stringstream sstream(str2);
           std::string output;
           while (sstream.good()) {
             std::bitset < 8 > bits;
             sstream >> bits;
             char c = char(bits.to_ulong());
             output += c;
           }
           std::string str3 = output.substr(8, output.length() - 1);
           std::string charactercount = output.substr(0, 8);
            str3 = str3.substr(0, str3.length() - 1);

  std::cout <<"Msg Send payload:  "<<str3<<endl;
  std::cout << "Msg Send character count :  "<<charactercount <<endl;

  /////Delay & Duplicate///////////

   bool probDelay = probability(80);
  bool probDuplicate = probability(80);
  no_frames++;
  if (probDelay) {
    std::cout << "Error: Delayed frame" << endl;
    sendDelayed(msg, 0.2, "outs", send_to);
  } else if (probDuplicate) {
    cMessage * copy = msg -> dup();
    no_frames++;
    std::cout << "Error: Duplicated frame" << endl;
    send(msg, "outs", send_to);
    send(copy, "outs", send_to);
    std::cout <<"Msg Send (Duplicated):  "<<str3<<endl;
    std::cout << "Msg Send character count :  "<<charactercount <<endl;
  } else {
    send(msg, "outs", send_to);
  }
  EV << "gwa send msg " << "\n";
}

void Node::handleMessage(cMessage * msg) {
  std::stringstream sstream(msg -> getName());
  std::string output;
  while (sstream.good()) {
    std::bitset < 8 > bits;
    sstream >> bits;
    char c = char(bits.to_ulong());
    output += c;
  }
  EV << "handle Message " << output << "\n";

  if (msg -> isSelfMessage() && (strcmp(msg -> getName(), "timeoutEvent") == 0)) {
    EV << "time out ";
    generateMessage(true, 0);

  } else if (msg -> isSelfMessage() && strcmp(msg -> getName(), "i'm outta here!!") != 0) { //Host wants to send

    int rand, dest;

    EV << " from source " << getIndex() << "\n";

    if (strcmp(msg -> getName(), "send") != 0) {

      do { //Avoid sending to yourself
        rand = uniform(0, gateSize("outs"));
      } while (rand == getIndex());
      dest = rand;

      std::cout <<endl<< "source node: " << getIndex() << "  reciever node : " << dest << endl;
      if (rand > getIndex())
        dest--;

      send_to = dest;

      frame_to_send = 0;
      first_frame_sent = 0;
      seq_num = 0;
      std::ifstream MyReadFile("D:/omnit/omnetpp-5.6.2-src-windows/omnetpp-5.6.2/samples/Mesh/src/node" + std::to_string(getIndex()) + ".txt");

      std::string myText;
      sendM = new cMessage("send");
      int i = 0;
      if (MyReadFile.is_open()) {
        EV << "opened " << "\n";\

        while (getline(MyReadFile, myText)) {

          cMessage * framed = sendFrame(new cMessage(myText.c_str()));

          ///////////////
          msg_to_send[i] = new cMessage(framed -> getName()); ////////here sends the msg.
          msg_to_send[i] -> addPar("ack");
          msg_to_send[i] -> addPar("seq_num");
          msg_to_send[i] -> addPar("first");
          msg_to_send[i] -> par("seq_num").setLongValue(seq_num);
          EV << "seq " << seq_num << "\n";
          seq_num++;
          if (seq_num >= window_size) {
            seq_num = 0;
          }
          //                    EV << "not opened " <<  hammingCode.c_str() <<"\n";
          EV << "not opened " << framed -> getName() << "\n";
          i++;
        }
        msg_size = i;
        MyReadFile.close();
      } else {
        EV << "not opened " << myText.c_str() << "\n";
      }

      ack_frame = 0;
      int ack = 0;
      msg_to_send[frame_to_send] -> par("ack").setLongValue(ack);
      msg_to_send[frame_to_send] -> par("first").setLongValue(1);

      std::string msgtossend = msg_to_send[frame_to_send] -> getName();

      ////Haaming//

      std::string frame = msgtossend;
      int n = frame.length();
      char char_array[n + 1];
      strcpy(char_array, frame.c_str());
      std::string bits = "";
      std::string newbits = "";
      for (int i = 0; i < n; i++) {
        std::bitset < 8 > chbits(char_array[i]);
        bits = bits + chbits.to_string();
      }
      for (int i = 0; i < bits.size(); i++) {
        if (bits[i] == '0')
          newbits = newbits + '0';
        else if (bits[i] == '1')
          newbits = newbits + '1';
      }

      std::string hammingCode = hamming_code(newbits, newbits.size());
      msg_to_send[frame_to_send] -> setName(hammingCode.c_str());
      /////Losss////
      bool probLoss = probability(20);
      bool probCorrupted = probability(80);
      if (probLoss)
        sendMessage(msg_to_send[frame_to_send]);
      else if (probCorrupted) {
        std::cout << "Error: corrupted frame" << endl;
        std::cout << "The message before correction:  " << msgtossend.substr(8,msgtossend.length()-9) <<"  with character count: "<<msgtossend.substr(0,8)<< endl;
        std::string corruptedMsg = corruption(msg_to_send[frame_to_send] -> getName());
        std ::string output= removehamming(corruptedMsg)  ;
               std::string str3 = output.substr(8, output.length() - 9);
               std::string charactercount = output.substr(0, 8);

        std::cout << "Msg after corruption:  " << str3 << "  with character count: "<<charactercount<< endl;
        msg_to_send[frame_to_send] -> setName(corruptedMsg.c_str());
        sendMessage(msg_to_send[frame_to_send]);
      } else {
        drop_fram++;
        std::cout << "Error: Loss, dropped frame" << endl;
      }
      frame_to_send++;
      cancelEvent(timeoutEvent);
      EV << "scheduleAt1 " << simTime() + time << "\n";
      scheduleAt(simTime() + time, timeoutEvent);

      scheduleAt(simTime() + 0.5, sendM);
    } else {
      generateMessage(false, 0);
      EV << "heeeeeeeey have fun hahaha!!" << "\n";
      scheduleAt(simTime() + 0.5, sendM);
    }
  } else if (strcmp(msg -> getName(), "i'm outta here!!") == 0) {
    EV << "should stop here";
    ack_frame = 0;
    /*no_frames = 0; //number of total frames
    drop_fram = 0; // number of the dropped frames.
    ret_frames = 0; //number of retransmitted frames.
    no_ack = 0;*/
    cancelEvent(timeoutEvent);
    cancelEvent(sendM);
    if (msg -> isSelfMessage()) {
      cMessage * out = new cMessage("i'm outta here!!");
      send(out, "outs", send_to);
    }
  } else if (msg -> par("ack").longValue() == 1 || msg -> par("first").longValue() == 0) {

    if (msg -> par("ack").longValue() == 1) {
      EV << "zwd ya bashmohnds tarek" << first_frame_sent << "\n";
      no_ack++;
      first_frame_sent++;
    }

    std::cout << "at node: " << getIndex() << endl;
    ////////////////////Hamming code////////////////////////////
    std::string payLoadstr = msg -> getName();
    std::string paritybits = "";
    std::string realmeesagebits = "";


    int n = payLoadstr.length();
    int k = 0;

    for (int i = 0; i < n; i++) {
      if (i == ((int) pow(2, k) - 1)) {
        paritybits = paritybits + payLoadstr[i];
        k++;
      } else
        realmeesagebits = realmeesagebits + payLoadstr[i];
    }
    std::string newbitsrecv = "";
    for (int i = 0; i < realmeesagebits.size(); i++) {
      if (realmeesagebits[i] == '0')
        newbitsrecv = newbitsrecv + '0';
      else if (realmeesagebits[i] == '1')
        newbitsrecv = newbitsrecv + '1';
    }
    std::string str2 = realmeesagebits;
    std::stringstream sstream(str2);
    std::string output;
    while (sstream.good()) {
      std::bitset < 8 > bits;
      sstream >> bits;
      char c = char(bits.to_ulong());
      output += c;
    }
    std::string str3 = output.substr(8, output.length() - 9);
    std::string charactercount = output.substr(0, 8);
   std::cout << "Msg received with payload : " << str3 << "   sequence number: " << msg -> par("seq_num").longValue() << "  type: ";
   if (msg -> par("ack").longValue() == 1) {
      std::cout << "piggybacking";
    } else if (msg -> par("ack").longValue() == 0) {
      std::cout << "data";
    }
   std::cout <<endl<< "Msg received  character count :  "<<charactercount <<endl;
    std::string hammingCoderecv = hamming_code(newbitsrecv, newbitsrecv.size());
    k = 0;
    std::string calculatedParitystr = "";
    for (int i = 0; i < n; i++) {
      if (i == ((int) pow(2, k) - 1)) {
        calculatedParitystr = calculatedParitystr + hammingCoderecv[i];
        k++;
      }
    }
    int result = paritybits.compare(calculatedParitystr);
    if (result != 0) {
      int wrongBitPosition = 0;
      std::vector < int > wrongParityPosition;
      for (int i = 0; i < paritybits.size(); i++) {
        if (paritybits[i] != calculatedParitystr[i]) {
          wrongParityPosition.push_back((int) pow(2, i));
        }
      }
      for (int i = 0; i < wrongParityPosition.size(); i++) {
        wrongBitPosition = wrongBitPosition + wrongParityPosition[i];
      }
      if (wrongParityPosition.size() == 1) {
        std::cout << "Error detection in parity bit" << endl;
        std::cout << "The message is right. The wrong bit is at parity number " << wrongBitPosition << endl;

        std::string correctedMsg = payLoadstr;
        if (payLoadstr[wrongBitPosition-1] == '0')
          correctedMsg[wrongBitPosition-1] = '1';
        else
          correctedMsg[wrongBitPosition-1] = '0';
        //std::cout << "The message after correction:  " << correctedMsg << endl;

        EV << "Error in parity bit" << endl;
        EV << "The message is right. The wrong bit is at parity number " << wrongBitPosition << endl;
      } else {
        std::cout << "Error detection in message by Hamming" << endl;
      //  std::cout << "The wrong bit is number " << wrongBitPosition << endl;
        std ::string output= removehamming(payLoadstr)  ;
               std::string str3 = output.substr(8, output.length() - 9);
               std::string charactercount = output.substr(0, 8);

        std::cout << "Message before correction:  " << str3 <<"  with character count: "<< charactercount<<endl;

        std::string correctedMsg = payLoadstr;
        if (payLoadstr[wrongBitPosition - 1] == '0')
          correctedMsg[wrongBitPosition - 1] = '1';
        else
          correctedMsg[wrongBitPosition - 1] = '0';

           output = removehamming(correctedMsg);
          str3 = output.substr(8, output.length() - 9);
          charactercount = output.substr(0, 8);
        std::cout << "Message after correction by hamming : " << str3 <<"  with character count: "<<charactercount<< endl;

      }
    } else {
      std::cout << "No error detected from hamming" << endl;
    }
    ////////////////////Hamming code////////////////////////////

    if (ack_frame == msg -> par("seq_num").longValue()) {
      EV << "index  " << getIndex() << "seq " << msg -> par("seq_num").longValue() << "ack_frame " << ack_frame << "\n";
      ack_frame++;
      cancelEvent(timeoutEvent);
      //            EV << unframed->getName();
      bubble("Message received");
      scheduleAt(simTime() + time, timeoutEvent);
      generateMessage(false, 1);
      if (ack_frame >= window_size) {
        ack_frame = 0;
      }
    } else {
      EV << "index  " << getIndex() << "seq " << msg -> par("seq_num").longValue() << "ack_frame " << ack_frame << "\n";
      std::cout << "Wrong frame" << "\n";
    }
  } else {

    EV << "gwa ack 0 " << ack_frame << "\n";
    if (ack_frame == msg -> par("seq_num").longValue()) {
      sendM = new cMessage("send");
      EV << "index  " << getIndex() << "seq " << msg -> par("seq_num").longValue() << "ack_frame " << ack_frame << "\n";
      send_to = msg -> getSenderGate() -> getIndex();
      frame_to_send = 0;
      seq_num = 0;

      std::string payLoadstr = msg -> getName();
      std::string paritybits = "";
      std::string realmeesagebits = "";
      int n = payLoadstr.length();
      int k = 0;
      for (int i = 0; i < n; i++) {
        if (i == ((int) pow(2, k) - 1)) {
          paritybits = paritybits + payLoadstr[i];
          k++;
        } else
          realmeesagebits = realmeesagebits + payLoadstr[i];
      }
      std::string newbitsrecv = "";
      for (int i = 0; i < realmeesagebits.size(); i++) {
        if (realmeesagebits[i] == '0')
          newbitsrecv = newbitsrecv + '0';
        else if (realmeesagebits[i] == '1')
          newbitsrecv = newbitsrecv + '1';
      }

      std::string str2 = realmeesagebits;
      std::stringstream sstream(str2);
      std::string output;
      while (sstream.good()) {
        std::bitset < 8 > bits;
        sstream >> bits;
        char c = char(bits.to_ulong());
        output += c;
      }
      std::string str3 = output.substr(8, output.length() -9);

     std::cout << "Msg received with payload : " << str3 << "  sequence number: " << msg -> par("seq_num").longValue() << "  type: data";
     std::string charactercount = output.substr(0, 8);
     std::cout <<endl<< "Msg received  character count :  "<<charactercount <<endl;

     std::string hammingCoderecv = hamming_code(newbitsrecv, newbitsrecv.size());
      k = 0;
      std::string calculatedParitystr = "";
      for (int i = 0; i < n; i++) {
        if (i == ((int) pow(2, k) - 1)) {
          calculatedParitystr = calculatedParitystr + hammingCoderecv[i];
          k++;
        }
      }
      int result = paritybits.compare(calculatedParitystr);
      if (result != 0) {
        int wrongBitPosition = 0;
        std::vector < int > wrongParityPosition;
        for (int i = 0; i < paritybits.size(); i++) {
          if (paritybits[i] != calculatedParitystr[i]) {
            wrongParityPosition.push_back((int) pow(2, i));
          }
        }
        for (int i = 0; i < wrongParityPosition.size(); i++) {
          wrongBitPosition = wrongBitPosition + wrongParityPosition[i];
        }
        if (wrongParityPosition.size() == 1) {
          std::cout << "Error detection in parity bit" << endl;
          std::cout << "The message is right. The wrong bit is at parity number " << wrongBitPosition << endl;
          std::cout << "The message before correction:  " << payLoadstr << endl;

          std::string correctedMsg = payLoadstr;
          if (payLoadstr[wrongBitPosition - 1] == '0')
            correctedMsg[wrongBitPosition - 1] = '1';
          else
            correctedMsg[wrongBitPosition - 1] = '0';

          std::cout << "The message after correction by hamming:  " << correctedMsg << endl;

          EV << "Error in parity bit" << endl;
          EV << "The message is right. The wrong bit is at parity number " << wrongBitPosition << endl;
        } else {
          std::cout << "Error detection in message by Hamming" << endl;
       //   std::cout << "The wrong bit is number " << wrongBitPosition << endl;

          std::cout << "The message before correction:  " << payLoadstr << endl;

          std::string correctedMsg = payLoadstr;
          if (payLoadstr[wrongBitPosition - 1] == '0')
            correctedMsg[wrongBitPosition - 1] = '1';
          else
            correctedMsg[wrongBitPosition - 1] = '0';

          int n = correctedMsg.length();
          std::string correctedMsg2 = "";
          k = 0;
          for (int i = 0; i < n; i++) {
            if (i == ((int) pow(2, k) - 1)) {
              k++;
            } else
              correctedMsg2 = correctedMsg2 + correctedMsg[i];
          }
         // std::cout << "The message after correction without parity: " << correctedMsg2 << endl;
          std::string str2 = correctedMsg2;
          std::stringstream sstream(str2);
          std::string output;
          while (sstream.good()) {
            std::bitset < 8 > bits;
            sstream >> bits;
            char c = char(bits.to_ulong());
            output += c;
          }
          std::string str3 = output.substr(8, output.length() - 9);
          std::string charactercount = output.substr(0, 8);
           std::cout << "Message after correction : " << str3 <<"  with character count: "<<charactercount<< endl;

        }
      } else {
        std::cout << "No error detected from hamming" << endl;
      }

      EV << "index  " << getIndex() << "seq " << msg -> par("seq_num").longValue() << "ack_frame " << ack_frame << "\n";
      /////////////////////////////////////////////
      cMessage * unframed = receiveFrame(new cMessage(msg -> getName()));
      EV << unframed -> getName() << "  unframed" << "\n";
      std::ifstream MyReadFile("D:/omnit/omnetpp-5.6.2-src-windows/omnetpp-5.6.2/samples/Mesh/src/node" + std::to_string(getIndex()) + ".txt");

      std::string myText;
      int i = 0;
      if (MyReadFile.is_open()) {
        while (getline(MyReadFile, myText)) {

          cMessage * framed = sendFrame(new cMessage(myText.c_str()));

          msg_to_send[i] = new cMessage(framed -> getName()); ////////here sends the msg.
          msg_to_send[i] -> addPar("ack");
          msg_to_send[i] -> addPar("seq_num");
          msg_to_send[i] -> addPar("first");
          msg_to_send[i] -> par("seq_num").setLongValue(seq_num);
          EV << "seq " << seq_num << "\n";
          seq_num++;
          if (seq_num >= window_size) {
            seq_num = 0;
          }

          i++;
        }
        msg_size = i;
        MyReadFile.close();
      }

      ack_frame++;
      seq_num = 1;
      //            EV << unframed->getName();
      bubble("Message received");
      generateMessage(false, 1);
      //            cancelEvent(timeoutEvent);
      scheduleAt(simTime() + time, timeoutEvent);
      scheduleAt(simTime() + 0.5, sendM);
    } else {

      std::cout << "wrong frame " << "\n";
    }
  }
}

cMessage * Node::sendFrame(cMessage * msg) {
  std::string final = "";
  std::string word;
  std::string oneMsg = "";

  word = msg -> getName();

  int charcount = word.size();

  charcount = word.size() + 1;
  std::bitset < 8 > countBits(charcount);
  final += countBits.to_string() + word;

  cMessage * finalFrame = new cMessage(final.c_str());
  return finalFrame;
}

cMessage * Node::receiveFrame(cMessage * frame) {
  std::string final = "";
  std::string word;

  //1) get the string from the user
  word = frame -> getName();

 // std::cout << "msg with frame: " << word << endl;

  std::string countBits = "";
  int count = 0;
  for (int i = 0; i < 8; i++) {
    if (word[i] == '1')
      count += pow(2, 7 - i);
  }

  for (int i = 8; i < count + 7; i++) {
    final += word[i];
  }

//  std::cout << "msg without frame: " << final << endl;

  cMessage * finalUnframe = new cMessage(final.c_str());
  return finalUnframe;
}

float Node::calcStat() {
    float eff=0;
   if (((float)(no_ack + no_frames)) ==0)
           {
                eff=0;
           }
   else
   {
   eff = no_ack / (float)(no_ack + no_frames);
   }

  std::cout << "total number of generated frames: " << no_frames << endl;
  std::cout << "total number of dropped frames: " << drop_fram << endl;
  std::cout << "total number of retransmitted frames: " << ret_frames << endl;
  std::cout << "efficiency: " << eff << endl;

  return eff;
}

std::string Node::hamming_code(std::string bits, int n) {
  std::vector < int > inputVec;

  for (int i = 0; i < bits.size(); i++) {
    if (bits[i] == '0') {
      inputVec.push_back(0);
    } else if (bits[i] == '1') {
      inputVec.push_back(1);
    }
  }

  int ii, paritySize = 0, allSize, jj, kk;
  ii = jj = kk = 0;

  std::vector < int > hammingCodee;

  for (int i = 0; i < 1000; i++) {
    hammingCodee.push_back(0);
  }
  std::string hammingCodestr = "";

  while (n > (int) pow(2, ii) - (ii + 1)) {
    paritySize++;
    ii++;
  }

  allSize = paritySize + n;

  for (int ii = 0; ii < allSize; ii++) {

    if (ii == ((int) pow(2, kk) - 1)) {
      hammingCodee[ii] = 0;
      kk++;
    } else {
      hammingCodee[ii] = inputVec[jj];
      jj++;
    }
  }

  for (ii = 0; ii < paritySize; ii++) {
    int position = (int) pow(2, ii);
    int value;

    //////
    int countt = 0, i_c, j_c;
    i_c = position - 1;

    while (i_c < allSize) {
      for (j_c = i_c; j_c < i_c + position; j_c++) {

        if (hammingCodee[j_c] == 1)
          countt++;
      }
      i_c = i_c + 2 * position;
    }

    if (countt % 2 == 0)
      value = 0;
    else
      value = 1;
    /////

    hammingCodee[position - 1] = value;
  }

  std::cout << endl;

  for (ii = 0; ii < allSize; ii++) {
    std::stringstream ss;
    ss << hammingCodee[ii];
    std::string s;
    ss >> s;
    hammingCodestr = hammingCodestr + s;
  }

  return hammingCodestr;
}
bool Node::probability(float threshold) {
  int rand_val = rand() % 100;
  if (rand_val > threshold)
    return true;
  else
    return false;
}
std::string Node::corruption(std::string msgBits) {
  int index = rand() % (msgBits.length()-71);

  index = index +71;

  if (msgBits[index] == '1')
    msgBits[index] = '0';
  else
    msgBits[index] = '1';
//  std::cout << "Index: " << index << endl;
  return msgBits;
}
std::string Node:: removehamming(std::string msg)
{
    std::string correctedMsg=msg;
    int n = correctedMsg.length();
                  std::string correctedMsg2 = "";
                  int k = 0;
                  for (int i = 0; i < n; i++) {
                    if (i == ((int) pow(2, k) - 1)) {
                      k++;
                    } else
                      correctedMsg2 = correctedMsg2 + correctedMsg[i];
                  }
                  std::string str2 = correctedMsg2;
                  std::stringstream sstream(str2);
                  std::string output;
                  while (sstream.good()) {
                    std::bitset < 8 > bits;
                    sstream >> bits;
                    char c = char(bits.to_ulong());
                    output += c;
                  }
                   return output;

}
