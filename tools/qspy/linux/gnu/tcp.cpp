//////////////////////////////////////////////////////////////////////////////
// Product: Quantum Spy -- TCP/IP HAL for Linux/g++
// Last Updated for Version: 4.0.00
// Date of the Last Update:  Apr 07, 2008
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
//
// Copyright (C) 2002-2008 Quantum Leaps, LLC. All rights reserved.
//
// This software may be distributed and modified under the terms of the GNU
// General Public License version 2 (GPL) as published by the Free Software
// Foundation and appearing in the file GPL.TXT included in the packaging of
// this file. Please note that GPL Section 2[b] requires that all works based
// on this software must also be made publicly available under the terms of
// the GPL ("Copyleft").
//
// Alternatively, this software may be distributed and modified under the
// terms of Quantum Leaps commercial licenses, which expressly supersede
// the GPL and are specifically designed for licensees interested in
// retaining the proprietary status of their code.
//
// Contact information:
// Quantum Leaps Web site:  http://www.quantum-leaps.com
// e-mail:                  info@quantum-leaps.com
//////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <time.h>
#include <termios.h>
#include <unistd.h>

#include "hal.h"

//............................................................................
int l_serverSock = -1;
int l_clentSock  = -1;
static termios l_saved;            // structure with saved terminal attributes

//............................................................................
bool tcpOpen(int portNum) {

    l_serverSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);    // TCP socket
    if (l_serverSock == -1){
        printf("Server socket cannot be created.\n"
               "socket error 0x%08X.",
               errno);
        return false;
    }

    struct sockaddr_in local;
    memset(&local, 0, sizeof(local));
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = INADDR_ANY;
    local.sin_port = htons((unsigned short)portNum);

    // bind() associates a local address and port combination with the
    // socket just created. This is most useful when the application is a
    // server that has a well-known port that clients know about in advance.
    if (bind(l_serverSock, (struct sockaddr *)&local, sizeof(local))
        == -1)
    {
        printf("Error by binding server socket.\n"
               "socket error 0x%08X.",
               errno);
        return false;
    }

    if (listen(l_serverSock, 1) == -1) {
        printf("Server socket listen failed.\n"
               "socket error 0x%08X.",
               errno);
        return false;
    }

    // modify the terminal attributes
    if (tcgetattr(0, &l_saved) == -1) {  // get the original terminal settings
        return false;                             // getting attributes failed
    }
    struct termios t;
    if (tcgetattr(0, &t) == -1) {        // get the modified terminal settings
        return false;                             // getting attributes failed
    }
    t.c_lflag &= ~(ICANON | ECHO);          // disable canonical mode and echo
    if (tcsetattr(0, TCSANOW, &t) == -1) {
        return false;                              // seting attributes failed
    }

    return true;                                                    // success
}
//............................................................................
void tcpClose(void) {
    tcsetattr(0, TCSANOW, &l_saved);    // restore the saved terminal settings

    if (l_serverSock != -1) {
        close(l_serverSock);
    }
}
//............................................................................
int tcpRead(unsigned char *buf, size_t size) {
    int n;

    if (l_clentSock == -1) {      // still waiting for the client?
        n = 0;                                      // no data from the client

        fd_set readSet;
        FD_ZERO(&readSet);
        FD_SET(0, &readSet);                                    // the console
        FD_SET(l_serverSock, &readSet);

        struct timeval delay;
        delay.tv_sec = 0;
        delay.tv_usec = 200000;

        (void)select(l_serverSock + 1, &readSet, 0, 0, &delay);

        if (FD_ISSET(0, &readSet)) { // any intput available from the console?
            return -1;                                            // terminate
        }
        if (FD_ISSET(l_serverSock, &readSet)) {
            struct sockaddr_in fromAddr;
            socklen_t fromLen = (socklen_t)sizeof(fromAddr);
            l_clentSock = accept(l_serverSock,
                                 (struct sockaddr *)&fromAddr, &fromLen);
            if (l_clentSock == -1) {
                printf("Server socket accept failed.\n"
                       "socket error 0x%08X.",
                       errno);
                return -1;                                        // terminate
            }
            printf("Accepted connection from %s, port %d\n",
                   inet_ntoa(fromAddr.sin_addr),
                   htons(fromAddr.sin_port));
        }
    }
    else {
        fd_set readSet;
        FD_ZERO(&readSet);
        FD_SET(l_clentSock, &readSet);

        (void)select(l_clentSock + 1, &readSet, 0, 0, 0); // wait indefinitely

        if (FD_ISSET(l_clentSock, &readSet)) {
            n = recv(l_clentSock, (char *)buf, size, 0);
            if (n == -1) {
                printf("Client socket error.\n"
                       "socket error 0x%08X.",
                       errno);
            }
            if (n <= 0) {                                // the client hang up
                close(l_clentSock);

                // go back to waiting for a client, or a keypress to terminate
                l_clentSock = -1;
                return 0;                               // no data from client
            }
        }
        else {
            n = 0;                                  // no data from the client
        }
    }

    return (int)n;
}
