## 1. Development purpose
Since tcpdump, which captures packets under Linux, can only be used in command line mode and can only be saved as a file when the amount of data is large, it is not real-time. Therefore, this software is designed to achieve data sharing and real-time tracking of data. This software package consists of two parts, one is the server (running under Linux), and the other is the client (windows platform). During the development and testing process, the client can be used to intuitively customize the packet capture conditions and view the data packet situation.
## 2. Features of this software
1: This software can visually check the data packet situation on the Linux side through the client.
2: Online version function, multiple clients can log in to the server at the same time, and can share packet capture data of other clients. This function is mainly aimed at data sharing between different developers or different modules to achieve a joint debugging function.
## 3. Development introduction
1: The server side is based on the Linux platform, the development language is C, and the packet capture library is based on the open source libraries pcap and iksemel (XML interpretation library).
2: The client is based on the windows platform VC6.0, the development language is C++, and is based on the MFC library and iksemel.
## 4. Introduction to nouns involved
This system is collectively called SeeSee, the server name is CuteDog; the client name is CuteCat.
Pionner: refers to the pioneer mode, the packet capture conditions need to be specified. Hooker refers to the query mode, and can directly share Pionner's data.
5. System outline design
1: Server side

![image](https://github.com/sytpb/netflow/assets/12178686/374ccaa0-6feb-432b-a9c7-3bb9f1c9fca8)

2: client side

![image](https://github.com/sytpb/netflow/assets/12178686/a3bcdf22-4181-47d3-a914-336fecf00d5f)
