//
//  main.cpp
//  TCPIO
//
//  Created by Chunghee Lee on 11/16/12.
//  Copyright (c) 2012 Chunghee Lee. All rights reserved.
//

#include <iostream>
#include "tcpio.h"


int main(int argc, const char * argv[])
{

    TCPIO_SERVER server(20, 5000);
    sleep(1);
    TCPIO_CLIENT cliecnt(20, "127.0.0.1", 5000);
    
    while(1)
    {
        cout << "tick..\n";
        server.CheckServer();
        client.SendPacket((void*)"10000000000009249283049283480239000000");
        sleep(1);
    }
    
    // insert code here...
    std::cout << "Hello, World!\n";
    return 0;
}

