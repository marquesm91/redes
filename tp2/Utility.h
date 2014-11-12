#ifndef UTILITY_H
#define UTILITY_H

#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), send() and recv() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>     /* for atoi() */
#include <string.h>     /* for memset() and strcmp() */
#include <unistd.h>     /* for close() */
#include <time.h>       /* for clock_gettime() */
#include <netdb.h>      /* for ipv6 protocol */
#include <errno.h>      /* for perror() */
#include <netinet/in.h> /* Library for old version OS linux */

/* Constants for safety connection Client-Server */

#define MAX_UDP_DATA 1450       /* Max size of data on Datagram UDPs */
#define MAX_TCP_PACK_SIZE 65536 /* Max Size of receive buffer */
#define MAX_SAFETY_SIZE 250000  /* Max raw data to TCP operate */
#define MAX_PENDING 5           /* Maximum outstanding connection requests */
#define MAX_TRIES 10            /* Max tries UDP socket will try to send data */
#define MAX_RESULTS 1000        /* Max results to save */

#define UNIT 1000				/* Unit to save on file and print. Default is kilobits */

/* Struct to organize the collected information and print on the end */

struct Info
{
    int filenames[MAX_RESULTS];					/* File names */
    unsigned long long total_time[MAX_RESULTS];	/* RTT of file names */
    long double throughput[MAX_RESULTS];		/* Throughput of file names */
    int size_packages[MAX_RESULTS];				/* Size of each package when necessary */
    int e_multiple_connections;					/* Enable or Disable Multiple Connections on TCP. UDP is always 0 */
};

/* Declaration of all functions of the program */

void ShowResults(struct Info *Infos, int times, const char *protocol, int multi_connections);

void* GetAddr(struct sockaddr *sa);

unsigned long long GetTimestamp(struct timespec &after, struct timespec &before);

long double GetThroughput(unsigned long long total_time, int times, int filename);

int GetPackages(char ***cbuffer, int nbytes, int width);

#endif