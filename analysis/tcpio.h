//
//  tcpio.h
//  TCPIO
//
//  Created by Chunghee Lee on 11/16/12.
//  Copyright (c) 2012 Chunghee Lee. All rights reserved.
//

#ifndef __TCPIO__tcpio__
#define __TCPIO__tcpio__

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <vector>
#include <unistd.h>
#include <errno.h>

//#include <fnctl.h>\
ifdef HAVE_SYS_FILIO_H
//#include <sys/filio.h>
//#endif
#include <sys/ioctl.h>

using namespace std;
int setNonblocking(int fd);


struct Client
{
    int iClientFD;
    string buf;
};

class TCPIO_SERVER
{
    int iPacketSize;
    int iListenFD;
    int iPort;
    
    vector<Client> vClients;
    
public:
    TCPIO_SERVER(int iSize, int iPort)
    {
        
        this->iPacketSize = iSize;
        this->iPort = iPort;
        
        struct sockaddr_in serv_addr;
                
        iListenFD = socket(AF_INET, SOCK_STREAM, 0);
        memset(&serv_addr, '0', sizeof(serv_addr));
        
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        serv_addr.sin_port = htons(this->iPort);
        
        bind(iListenFD, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
        
        listen(iListenFD, 10);
        setNonblocking(iListenFD);
    }
    
    void CheckServer()
    {
        /* Busy-polling method. Bad but easy to use. And I have no plan to support select() now! haha */
        /* Asynchronous Checking */
        OnAccept();
        char buf[255];
        
        for(int i=0; i<vClients.size(); i++)
        {
            int res = recv(vClients[i].iClientFD, buf, sizeof(buf)-1, 0);
            if(res>0)
            {
                vClients[i].buf.append(buf, res);
                while (vClients[i].buf.size() >= iPacketSize)
                {
                    OnPacket((void*)vClients[i].buf.c_str());
                    vClients[i].buf = vClients[i].buf.substr(iPacketSize);
                }
            } else if(res<0)
            {
                if (errno!=EAGAIN)
                {
                    cout << "Error!";
                }
            } else if(res==0)
            {
                close(vClients[i].iClientFD);
                vClients.erase(vClients.begin() + i);
                continue;
            }
        }
    }
    
    void OnAccept()
    {
        Client client;
        client.iClientFD = accept(iListenFD, NULL, NULL);
        if (client.iClientFD>=0)
        {
            cout << "Client has connected\n";
            setNonblocking(client.iClientFD);
            vClients.push_back(client);
        }
    }
        
    virtual void OnPacket(void* packet)
    {
        for(int i=0; i<iPacketSize; i++)
            cout << ((char*)packet)[i];
    }
};

class TCPIO_CLIENT
{
    int iPacketSize;
    int iClientFD;
    struct sockaddr_in serv_addr;
    
public:
    TCPIO_CLIENT(int iSize, string strAddr, int iPort)
    {
        iPacketSize = iSize;
        if((iClientFD = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            printf("\n Error : Could not create socket \n");
        }
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(iPort);
        if(inet_pton(AF_INET, strAddr.c_str(), &serv_addr.sin_addr)<=0)
        {
            printf("\n inet_pton error occured\n");
        }
        Connect();
    };
    
    void Connect()
    {
        if(connect(iClientFD, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        {
            printf("\n Error : Connect Failed \n");
			exit(1);
        }
    }
    
    void Disconnect()
    {
        close(iClientFD);
        iClientFD = 0;
    }
    
    void SendPacket(void* data)
    {
        write(iClientFD, data, iPacketSize);
    };
};


#endif /* defined(__TCPIO__tcpio__) */
