/*
 * =====================================================================================
 *
 *       Filename:  producer.h
 *
 *    Description:  header file for c implementation of beanstalk producer
 *
 *        Version:  1.0
 *        Created:  05/22/2010 08:47:40 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *        Company:  
 *
 * =====================================================================================
 */
#ifndef PRODUCER_H
#define PRODUCER_H 

#include <stdint.h>

struct _bs_producer {
    int fd;
};

typedef struct _bs_producer bs_producer;

enum _bsp_put_response_t {
     BSP_PUT_RES_INSERTED,
     BSP_PUT_RES_BURIED,
     BSP_PUT_RES_EXPECTED_CRLF,
     BSP_PUT_RES_JOB_TOO_BIG
};

typedef enum _bsp_put_response_t bsp_put_response_t;

bs_producer *bs_producer_new(static char *, int);
void bs_producer_destroy(bs_producer *);
bsp_put_response_t bsp_put_msg( bs_producer *producer,
                            const char      *data,
                            size_t   bytes, 
                            uint32_t *id,
                            uint32_t priority,
                            uint32_t delay,
                            uint32_t ttr
                          );

#endif /* PRODUCER_H */
