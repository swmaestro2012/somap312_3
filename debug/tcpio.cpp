//
//  tcpio.cpp
//  TCPIO
//
//  Created by Chunghee Lee on 11/16/12.
//  Copyright (c) 2012 Chunghee Lee. All rights reserved.
//

#include "tcpio.h"

int setNonblocking(int fd)
{
    int flags;
    
    /* If they have O_NONBLOCK, use the Posix way to do it */
#if defined(O_NONBLOCK)
    /* Fixme: O_NONBLOCK is defined but broken on SunOS 4.1.x and AIX 3.2.5. */
    if (-1 == (flags = fcntl(fd, F_GETFL, 0)))
        flags = 0;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
#else
    /* Otherwise, use the old way of doing it */
    flags = 1;
    return ioctl(fd, FIONBIO, &flags);
#endif
}
