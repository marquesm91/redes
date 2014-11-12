#include "Utility.h"

int main(int argc, char *argv[])
{
    /* Test for correct number of arguments and correct inputs */
    if (argc != 4 || (strcmp(argv[1], "UDP") != 0 && strcmp(argv[1], "TCP") != 0) || (strcmp(argv[2], "4") != 0 && strcmp(argv[2], "6") != 0))
    {
    	fprintf(stderr, "------------------------------------------------------------------------------------\n");
        fprintf(stderr, "\nUsage: %s <Protocol> <IP version> <Port> \n\n", argv[0]);
        fprintf(stderr, "------------------------------------------------------------------------------------\n");
        fprintf(stderr, "<Protocol> use \"TCP\" or \"UDP\"\n");
        fprintf(stderr, "<IP version> use 4 or 6\n");
        fprintf(stderr, "------------------------------------------------------------------------------------\n\n");
        exit(-1);
    }

    int servsock;                               /*  Server descriptor */
    int clntsock;                               /*  Client descriptor */

    struct addrinfo hints, *ServInfo;           /*  Struct to get Server socket information */
    struct sockaddr_storage ClntAddr;           /*  Struct to get Client socket information */
    socklen_t cliAddrLen = sizeof(ClntAddr);    /*  Size of client socket */

    char str[INET6_ADDRSTRLEN];                 /*  To print the handle client */
    char echoPackUDP[MAX_UDP_DATA];             /*  Echo package for UDP socket */
    char echoPackTCP[MAX_TCP_PACK_SIZE];        /*  Echo package for TCP socket */
    int counter;                                /*  Counter for lines on UDP and current package number on TCP */
    int PackLen;                                /*  Get the size of pack on TCP socket */

    int bytesRcvd;                              /*  Bytes read in single read() */
    int error_number;                           /*  Get errors for perror() */
    int on = 1;                                 /*  To enable socket options */
    int bufferSizeOption = MAX_TCP_PACK_SIZE;   /*  Change the max TCP buffer on socket configuration */

    char *protocol = argv[1];                   /*  Choose UDP or TCP protocol */
    char *IPversion = argv[2];                  /*  Choose version of IP. 4 or 6 */
    char *ServerPort = argv[3];                 /*  Server port*/

    /* Start of the socket setup */
    memset(&hints, 0, sizeof(hints));

    /* Choosing the correct IP version*/
    if (strcmp(IPversion, "4") == 0)
        hints.ai_family = AF_INET;
    else if (strcmp(IPversion, "6") == 0)
        hints.ai_family = AF_INET6;

    /* Choosing the correct protocol */
    if (strcmp(protocol, "UDP") == 0)
        hints.ai_socktype = SOCK_DGRAM;
    else if (strcmp(protocol, "TCP") == 0)
        hints.ai_socktype = SOCK_STREAM;

    hints.ai_flags = AI_PASSIVE; /* wildcard to accept both IP - 4 or 6 - connections */

    /* Associate hints on ServInfo with the correct port */
    if (( error_number = getaddrinfo(NULL, ServerPort, &hints, &ServInfo)) != 0)
        perror("getaddrinfo() failed");

    /* Get a socket */
    if ((servsock = socket(ServInfo->ai_family, ServInfo->ai_socktype, 0)) < 0)
        perror("socket() failed");

    /* Set SO_REUSEADDR to rebind ports when necessary */
    if ((error_number = setsockopt(servsock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))) < 0)
        perror("setsockopt() failed");

    /* Binding the server */
    if ((error_number = bind(servsock, (const struct sockaddr *)ServInfo->ai_addr, ServInfo->ai_addrlen)) < 0)
        perror("bind() failed");

    /* Start Iterations for UDP or TCP */
    if (strcmp(protocol, "UDP") == 0)
    {
        counter = 0; /* Clear the counter for lines */

        while (true)
        {
            /* block until receive message from a client */
            if ((bytesRcvd = recvfrom(servsock, echoPackUDP, MAX_UDP_DATA, 0, (struct sockaddr *)&ClntAddr, &cliAddrLen)) < 0)
                perror("recvfrom() failed");

            counter++; /* +1 line */

            /* Printf the handled client */
            inet_ntop(ClntAddr.ss_family, GetAddr((struct sockaddr *)&ClntAddr), str, sizeof(str));
            printf("(%d) Handling client : %s\n", counter, str);

            /* Send recieved datagram back to the client */
            if ((error_number = sendto(servsock, echoPackUDP, bytesRcvd, 0, (struct sockaddr *) &ClntAddr, cliAddrLen)) < 0)
                perror("sendto() failed");
        }
    }
    else if (strcmp(protocol, "TCP") == 0)
    {
        /* Listen until MAX_PENDING connections */
        if ((error_number = listen(servsock, MAX_PENDING)) < 0)
            perror("listen() failed");

        while (true)
        {
            /* Wait for client connections */
            if ((clntsock = accept(servsock, (struct sockaddr *)&ClntAddr, &cliAddrLen)) < 0)
                perror("accept() failed");

            /* Print the correct handled client */
            inet_ntop(ClntAddr.ss_family, GetAddr((struct sockaddr *)&ClntAddr), str, sizeof(str));
            printf("Handling client : %s\n", str);

            /* When accepted start transmission */
            puts("Begin transmission...");

            counter = 0; /* Clear the counter for current package */

            while (true)
            {
                memset(echoPackTCP, 0, sizeof(echoPackTCP)); /* Clear the the buffer */
                
                /* Read until MAX_TCP_PACK_SIZE bytes */
                if ((bytesRcvd = read(clntsock, echoPackTCP, MAX_TCP_PACK_SIZE - 1)) < 0)
                    perror("read()");
                echoPackTCP[bytesRcvd] = '\0'; /* Write the terminator on buffer */

                counter++; /* +1 package */

                if (bytesRcvd <= 0) /* If we get a error (-1) or end of data (0) we terminate the loop */
                    break;
                else /* Print the current package that will be sended back and your size */
                    printf("Sending back package %d...(%d)\n", counter, bytesRcvd);

                /* Send the buffer back to client */
                if ((bytesRcvd = write(clntsock, echoPackTCP, bytesRcvd)) < 0)
                    perror("write()");
            }

            /* Closing the transmission */
            puts("Closing transmission...");
            close(clntsock);
        }
    }
}
