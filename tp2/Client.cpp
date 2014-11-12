#include "Utility.h"

int main(int argc, char *argv[])
{
  /* Test for correct number of arguments and correct inputs */
  if ( argc != 6 || (atoi(argv[4]) <= 0) || ((strcmp(argv[1], "UDP") != 0) && (strcmp(argv[1], "TCP") != 0)) || ((atoi(argv[5]) > 1) && (atoi(argv[5]) < 0)))
  {
    fprintf(stderr, "------------------------------------------------------------------------------------\n");
    fprintf(stderr, "\nUsage: %s <Protocol> <IP Server> <Port> <Times> <Mutiple TCP Connections>\n\n", argv[0]);
    fprintf(stderr, "------------------------------------------------------------------------------------\n");
    fprintf(stderr, "<Protocol> use \"TCP\" or \"UDP\"\n");
    fprintf(stderr, "<Times> has to be positive and diffrent from 0\n");
    fprintf(stderr, "<Multiple TCP Connections> has to be 1 (on) or 0 (off). Always use 0 on UDP sockets!\n");
    fprintf(stderr, "------------------------------------------------------------------------------------\n\n");
    exit(-1);
  }

  int sock;                                       /*  Socket descriptor */
  char **packages = NULL;                         /*  Vector of packages to send */
  char *echoPackage = NULL;                       /*  Buffer for recieved packages */

  struct Info Infos;                         			/*  Struct to collection final informations */

  struct timespec after_send, before_send;        /*  Structs to measure send timestamps */
  struct timespec after_resend, before_resend;    /*  Structs to measure resend timestamps */
  struct timespec timeout = {2, 0};               /*  Struct for 3 seconds timeout */
  struct addrinfo hints, *ServInfo;               /*  Struct to get Server socket information */
  struct sockaddr_storage ServAddr;               /*  Struct to connect on Ipv6 or Ipv4 servers */
  socklen_t ServAddrLen = sizeof(ServAddr);       /*  Size of Server socket */

  int total_pack;                                 /*  How many packages */
  int PackLen;                                    /*  Length of package */
  int packages_iterator;                          /*  Counter for packages */
  int files_iterator;                             /*  Counter for files */
  int times_iterator;                             /*  Counter for times */
  int tries_iterator;                             /*  Counter for tries */
  int error_number;                               /*  Get errors for perror() */
  int bytesRcvd;                                  /*  Bytes read in single read() */
  int totalBytesRcvd;                             /*  Total bytes read on read() */
  int open_connection;


  int width = MAX_UDP_DATA;                       /*  Width of each package protocol (UDP socket) */
  int total_tries = MAX_TRIES;                    /*  Max tries to send data on UDP socket */

  int filename;                                   /*  Filename to starting open */

  char *protocol = argv[1];                       /*  Choose UDP or TCP protocol */
  char *servIP = argv[2];                         /*  Server IP adress 4 or 6 version available */
  char *ServerPort = argv[3];                     /*  Server port */
  int times = atoi(argv[4]);                      /*  How many times to send the packages. Help to get the mean time */

  /* Clear the structs */
  memset(&Infos, 0, sizeof(Infos));
  memset(&hints, 0, sizeof(hints));

  Infos.e_multiple_connections = atoi(argv[5]);   /*  Enable multiple connections on TCP or use just one connection */

  /* Start setup socket */
  hints.ai_family = AF_UNSPEC;

  if (strcmp(protocol, "UDP") == 0)
    hints.ai_socktype = SOCK_DGRAM;
  else if (strcmp(protocol, "TCP") == 0)
    hints.ai_socktype = SOCK_STREAM;

  /* Associate hints on ServInfo with the correct port */
  if ((error_number = getaddrinfo(servIP, ServerPort, &hints, &ServInfo)) < 0)
    perror("getaddrinfo() failed");

  files_iterator = 0; /* First space on Infos to get the results */
  filename = 1; /* Start opening 1.txt*/
  open_connection = 1; /* Request a new connection */

  /* Start Iterations for UDP or TCP */
  if (strcmp(protocol, "TCP") == 0)
  {
    /* Start the first test. Loop for packages that will be 1B until 1MB */
    while (filename <= 1000000)
    {
      /* Opening the connection. On Multiple connections open_connection will be always 1 */
      if (open_connection == 1)
      {
        /* Get a socket */
        if ((sock = socket(ServInfo->ai_family, ServInfo->ai_socktype, 0)) < 0)
          perror("socket() failed");

        /* Establish the connection to the Server */
        if (connect(sock, ServInfo->ai_addr, ServInfo->ai_addrlen) < 0)
          perror("connect() failed");

        open_connection = 0;
      }

      Infos.filenames[files_iterator] = filename; /* Save correct filename on Infos */

      /* Save the correct size package for each package iterator. Max size will be MAX_SAFETY_SIZE */
      if (Infos.filenames[files_iterator] < 1000000)
        Infos.size_packages[files_iterator] = filename;
      else
        Infos.size_packages[files_iterator] = MAX_SAFETY_SIZE;

      /* Get packages. For TCP we get only one package while filename < MAX_SAFETY_SIZE */
      if ((total_pack = GetPackages(&packages, filename, MAX_SAFETY_SIZE)) == -1)
      {
        puts("Can't reach file.txt !");
        exit(-1);
      }

      /* Loop for mean times. Here we send and recieve the same package X times */
      for (times_iterator = 0; times_iterator < times; times_iterator++)
      {
        packages_iterator = 0; /* Start on first package */

        /* Loop for each divided package */
        while (packages_iterator < total_pack)
        {
          PackLen = strlen(packages[packages_iterator]); /* Get length package  */

          /* Resize and clear echoPackage depending on Packlen */
          echoPackage = (char *) realloc(echoPackage, (PackLen + 1) * sizeof(char));
          memset(echoPackage, 0, sizeof(echoPackage));

          /* Print which package will be send, your size and how many times */
          printf("Sending package (%d/%d) of %d bytes : times(%d/%d) : ", packages_iterator + 1, total_pack, filename, times_iterator + 1, times);

          /* Before send package */
          clock_gettime(CLOCK_REALTIME, &before_send);

          /* Send package to Server */
          if ((error_number = write(sock, packages[packages_iterator], PackLen)) < 0)
            perror("write() failed");

          totalBytesRcvd = 0; /* Clear the total bytes recieved from Server */

          /* Recieve the same package back from Server */
          while ((totalBytesRcvd < PackLen))
          {
            if ((bytesRcvd = read(sock, echoPackage, PackLen)) < 0)
              perror("read() failed");
            totalBytesRcvd += bytesRcvd;
          }

          echoPackage[totalBytesRcvd] = '\0'; /* null-terminate the received data */

          /* After send package */
          clock_gettime(CLOCK_REALTIME, &after_send);

          /* Get the current RTT */
          Infos.total_time[files_iterator] += GetTimestamp(after_send, before_send);

          packages_iterator++;
        }
      }

      /* Get the throughput with total time of package, how many times package was sended, which protocol and which package */
      Infos.throughput[files_iterator] = GetThroughput(Infos.total_time[files_iterator], times, filename);

      files_iterator++; /* Next space on Infos to write the information of next filename */

      /* How to increment to next filename */
      if (filename < 1000)
        filename += (100 - filename % 100);
      else if (filename >= 1000 && filename < 32000)
        filename += 1000;
      else if (filename == 32000)
        filename = 100000;
      else if (filename >= 100000)
        filename += 900000;

      /* If mutiple connections is enabled we have to close the client socket and request a new connection */
      if (Infos.e_multiple_connections == 1)
      {
        close(sock);
        open_connection = 1;
      }
    }

    /* Start second test for 1MB file. Just for TCP sockets */
    filename = 1000000;
    width = 1000; /* Start with 1000B size package */

    /* Again, if mutiple connections is enable we have to request a new connection */
    if (Infos.e_multiple_connections == 1)
      open_connection = 1;

    /* Loop until size package be more than 32kB */
    while (width <= 32000)
    {
      /* Opening the connection. On Multiple connections open_connection will be always 1 */
      if (open_connection == 1)
      {
        /* Get a socket */
        if ((sock = socket(ServInfo->ai_family, ServInfo->ai_socktype, 0)) < 0)
          perror("socket() failed");

        /* Establish the connection to the Server */
        if (connect(sock, ServInfo->ai_addr, ServInfo->ai_addrlen) < 0)
          perror("connect() failed");

        open_connection = 0;
      }

      Infos.filenames[files_iterator] = filename; /* Save correct filename on Infos */
      Infos.size_packages[files_iterator] = width; /* Save correct width iteration*/

      /* Get packages with 1000, 8000, 16000 and 32000 size */
      if ((total_pack = GetPackages(&packages, filename, Infos.size_packages[files_iterator])) == -1)
      {
        puts("Can't reach file.txt !");
        exit(-1);
      }

      /* Loop for mean times. Here we send and recieve the same package X times */
      for (times_iterator = 0; times_iterator < times; times_iterator++)
      {
        packages_iterator = 0; /* Start on first package */

        /* Loop for each divided package */
        while (packages_iterator < total_pack)
        {
          PackLen = strlen(packages[packages_iterator]); /* Get package size */

          /* resize and clear echoPackage depending on Packlen */
          echoPackage = (char *) realloc(echoPackage, (PackLen + 1) * sizeof(char));
          memset(echoPackage, 0, sizeof(echoPackage));

          /* Print which package will be send, your size and how many times */
          printf("Sending package (%d/%d) of %d bytes : times(%d/%d) : ", packages_iterator + 1, total_pack, filename, times_iterator + 1, times);

          /* Before send package */
          clock_gettime(CLOCK_REALTIME, &before_send);

          /* Send package to Server */
          if (write(sock, packages[packages_iterator], PackLen) < 0)
            perror("write() failed");

          /* Receive the same package back from Server */
          totalBytesRcvd = 0;
          while ((totalBytesRcvd < PackLen))
          {
            if ((bytesRcvd = read(sock, echoPackage, PackLen)) < 0)
              perror("read() failed");
            totalBytesRcvd += bytesRcvd;
          }

          echoPackage[totalBytesRcvd] = '\0'; /* null-terminate the received data */

          /* After send package */
          clock_gettime(CLOCK_REALTIME, &after_send);

          /* Get the current RTT */
          Infos.total_time[files_iterator] += GetTimestamp(after_send, before_send);

          packages_iterator++;
        }
      }

      /* Get the throughput with total time of package, how many times package was sended, which protocol and which package */
      Infos.throughput[files_iterator] = GetThroughput(Infos.total_time[files_iterator], times, filename);

      files_iterator++; /* Next space on Infos to write the information of next filename */

      /* How to increment for next size package */
      if (width == 1000)
        width = 8000;
      else if (width >= 8000)
        width *= 2;

      /* If mutiple connections is enabled we have to close the client socket and request a new connection */
      if (Infos.e_multiple_connections == 1)
      {
        close(sock);
        open_connection = 1;
      }
    }
  }
  else if (strcmp(protocol, "UDP") == 0)
  {
    /* Get a socket */
    if ((sock = socket(ServInfo->ai_family, ServInfo->ai_socktype, 0)) < 0)
      perror("socket() failed");

    /* Adjust the timeout for UDP socket. 2 seconds choosed */
    if (setsockopt (sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
      perror("setsockopt() failed");

    /* Start iterations for each specific file  until 1MB */
    while (filename <= 1000000)
    {
      Infos.filenames[files_iterator] = filename;  /* Save correct filename on Infos */
      Infos.size_packages[files_iterator] = width; /* size of each package */

      /* Get packages. For UDP we get filename/width packages. Never will be more than 1500B when including the header */
      if ((total_pack = GetPackages(&packages, filename, width)) == -1)
      {
        puts("Can't reach file.txt !");
        exit(-1);
      }

      /* Make the test i times to get a good mean times_iterator > 100 */
      for (times_iterator = 0; times_iterator < times; times_iterator++)
      {
        packages_iterator = 0;

        /* Iterator for each fragmented package */
        while (packages_iterator < total_pack)
        {
          PackLen = strlen(packages[packages_iterator]); /* Determine input length */

          /* resize and clear echoPackage depending on Packlen */
          echoPackage = (char *) realloc(echoPackage, (PackLen + 1) * sizeof(char));
          memset(echoPackage, 0, sizeof(echoPackage));

          tries_iterator = 0;

          printf("Sending package (%d/%d) of %d bytes : times(%d/%d) : ", packages_iterator + 1, total_pack, filename, times_iterator + 1, times);

          /* Send and resend when necessary */
          while (true)
          {
            /* Before send package */
            clock_gettime(CLOCK_REALTIME, &before_send);

            /* Send package */
            sendto(sock, packages[packages_iterator], PackLen, 0, ServInfo->ai_addr, ServInfo->ai_addrlen);

            /* Recieve the same package*/
            error_number = recvfrom(sock, echoPackage, PackLen, 0, (struct sockaddr *)&ServAddr, &ServAddrLen);

            /* After send package */
            clock_gettime(CLOCK_REALTIME, &after_send);

            if (error_number < 0) /* Case of error or timeout blow up */
            {
              printf("\nTrying to resending package %d from %d.txt (%d/%d)", packages_iterator + 1, filename, tries_iterator + 1, total_tries);
              tries_iterator++;

              /* if the program try 10 times without sucess the program is aborted*/
              if (tries_iterator == 10)
              {
                printf("\nCan't reach Server %s port %s after tried %d times...\nAborting...\n", servIP, ServerPort, total_tries);
                exit(-1);
              }
            }
            else /* case of sucess send and recieve the same package we terminate the loop */
            {
              break;
            }
          }

          echoPackage[PackLen] = '\0'; /* null-terminate the received data */

          /* Get the current RTT */
          Infos.total_time[files_iterator] += GetTimestamp(after_send, before_send);

          packages_iterator++;
        }
      }

      /* Get the throughput with total time of package, how many times package was sended, which protocol and which package */
      Infos.throughput[files_iterator] = GetThroughput(Infos.total_time[files_iterator], times, filename);

      files_iterator++; /* next space on Infos to write the information of next filename */

      /* How to increment to next filename*/
      if (filename < 1000)
        filename += (100 - filename % 100);
      else if (filename >= 1000 && filename < 32000)
        filename += 1000;
      else if (filename == 32000)
        filename = 100000;
      else if (filename >= 100000)
        filename += 900000;
    }
  }

  /* Print the results and save on UDP.txt or TCP.txt */
  ShowResults(&Infos, times, protocol, Infos.e_multiple_connections);

  /* Closing client socket and free memory*/
  free(echoPackage);
  close(sock);
  putchar('\n');
  exit(0);
}