/*
 * =====================================================================================
 *
 *       Filename:  beanstalk.h
 *
 *    Description:  header file for c implementation of beanstalk client
 *
 *        Version:  1.0
 *        Created:  05/22/2010 08:47:40 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Roey Berman, royb@walla.net.il
 *        Company:  Walla!
 *
 * =====================================================================================
 */
#ifndef BEANSTALK_H
#define BEANSTALK_H 

#include <stdint.h>
#include <event.h>

#define  BS_DEFAULT_PORT "11300"

enum _bsc_put_response_t {
     BSP_PUT_RES_INSERTED,
     BSP_PUT_RES_BURIED,
     BSP_PUT_RES_EXPECTED_CRLF,
     BSP_PUT_RES_JOB_TOO_BIG
};

typedef enum _bsc_put_response_t bsc_put_response_t;

char *bsc_gen_put_msg(const char *, size_t, uint32_t *, uint32_t, uint32_t, uint32_t);

#endif /* BEANSTALK_H */
