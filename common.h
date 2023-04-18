//
// Created by Aaron on 12/03/2022.
//

#define _DEFAULT_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <netdb.h>
#include <pthread.h>
#include <sys/select.h>


#define T 1
#define Q 4
#define P 2
#define N 8
#define O 3
#define U 2
#define R 2
#define S 3
#define W 4

/* global variables */
bool debug_mode = false;
char *network_dev_config_file_name = NULL;
struct Server server_data;
struct Sockets sockets;
char *client_state = "DISCONECTED";
pthread_t tid = (pthread_t) NULL;

/*  PDU */
struct Package {
    unsigned char type;
    char id[11];
    char com[11];
    char data[61];
};

/* PDU tcp Enviar les dades al servidor */
struct PackagePDUtcp {
    unsigned char type;
    char id[11];
    char com[11];
    char elements[40];
    char value[6][16];
    char info[80];
};

struct Client {
    char id[11];
    char elements[40];
    char element[6][8];    
    char value[6][16];
    int tcp_port;
    int unsuccessful_signups;
};
struct Client client_data = { .unsuccessful_signups = 0 };
struct Server {
    char name[20];
    char *address;
    char elements[40];
    char rand_num[11];
};


struct Sockets {
    int udp_socket;
    int udp_port;
    struct timeval udp_timeout;
    struct sockaddr_in udp_addr_server;

    int tcp_socket;
    int tcp_port;
    struct timeval tcp_timeout;
    struct sockaddr_in tcp_addr_server;
};

