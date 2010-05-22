/*
 * =====================================================================================
 *
 *       Filename:  test_producer.c
 *
 *    Description:  test suite for beanstalk producer library
 *
 *        Version:  1.0
 *        Created:  05/22/2010 09:29:13 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *        Company:  
 *
 * =====================================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "producer.h"

int main() {
    event_init();
    bs_producer *bsp;
    bsp_put_response_t res;
    char *msg;
    size_t msg_len;
    uint32_t q_id;

    if ( (bsp = bs_producer_new()) == NULL )
    {
        fprintf(stderr, "failed to create a new beanstalk producer");
    }
    msg = "baba";
    msg_len = strlen(msg);

    res =bsp_put_msg( bsp, msg, msg_len, &q_id, 1, 2, 3 );
	event_dispatch();
    return EXIT_SUCCESS;
}
