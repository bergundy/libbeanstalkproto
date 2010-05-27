/*
 * =====================================================================================
 *
 *       Filename:  producer.c
 *
 *    Description:  c implementation of beanstalk producer
 *
 *        Version:  1.0
 *        Created:  05/22/2010 08:44:18 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Roey Berman,
 *        Company:  Walla!
 *
 * =====================================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "producer.h"

void fifo_read(int fd, short event, void *arg)
{
}

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

bs_producer *bs_producer_new(const char *server_addr, const char *server_port)
{
    bs_producer *bsp;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];
	struct event *evfifo;
    if ( ( bsp = (bs_producer *) malloc( sizeof(bs_producer) ) ) == NULL )
    {
        return NULL;
    }
    memset(&hints, 0, sizeof hints);
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(server_addr, server_port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return NULL;
    }

    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((bsp->fd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        if (connect(bsp->fd, p->ai_addr, p->ai_addrlen) == -1) {
            close(bsp->fd);
            perror("client: connect");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return NULL;
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
            s, sizeof s);
    printf("client: connecting to %s\n", s);

    freeaddrinfo(servinfo); // all done with this structure

    if ( ( evfifo = (struct event *) malloc(sizeof(struct event)) ) == NULL )
    {
        fprintf(stderr, "error creating new event\n");
        goto failed_event;
    }


	event_set(evfifo, bsp->fd, EV_READ, fifo_read, evfifo);

	/* Add it to the active events, without a timeout */
	event_add(evfifo, NULL);

    return bsp;

failed_event:
    free(bsp);
    return NULL;
}
 
void bs_producer_destroy(bs_producer *bsp)
{
    free(bsp);
}

bsp_put_response_t bsp_put_msg( bs_producer *bsp,
                            const char      *data,
                            size_t   bytes, 
                            uint32_t *id,
                            uint32_t priority,
                            uint32_t delay,
                            uint32_t ttr
                          )
{
//    fprintf(bsp->fd, "%d %d %d %d\r\n%s\r\n",
//        priority, delay, ttr, bytes, data );
    *id = 2;
    return BSP_PUT_RES_INSERTED;
}
