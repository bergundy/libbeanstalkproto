/*
 * =====================================================================================
 *
 *       Filename:  test_producer.c
 *
 *    Description:  test suite for producer functions from beanstalk client library
 *
 *        Version:  1.0
 *        Created:  05/22/2010 09:29:13 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Roey Berman, royb@walla.net.il
 *        Company:  Walla!
 *
 * =====================================================================================
 */

#include <tap.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sockutils.h>
#include "beanstalkclient.h"

#define ALL_TESTS 9

int main() {
    plan_tests(ALL_TESTS);

    /* variable declaration / intialization */
    char     res[256], *put_cmd_msg, *use_cmd_msg, *hdr, error_str[256],
             expected_hdr[] = "put 1 2 3 4\r\n",
             data[] = "baba",
             tube[] = "my_tube", 
             use_cmd_expected_msg[40],
             put_cmd_expected_msg[40],
             *res_tube;

    size_t   data_len, tube_len;
    ssize_t  bytes_read, bytes_wrote;
    int      put_cmd_msg_len, use_cmd_msg_len, hdr_len,
             put_cmd_expected_msg_len, use_cmd_expected_msg_len,
             expected_hdr_len, fd;

    uint32_t id = 0;
    bsc_response_t res_t = BSC_UNRECOGNIZED_RESPONSE;
    
    /*-----------------------------------------------------------------------------
     *  testing command generation 
     *-----------------------------------------------------------------------------*/

    /* put command */
    data_len = strlen(data);
    expected_hdr_len = strlen(expected_hdr);
    memcpy(put_cmd_expected_msg, expected_hdr, expected_hdr_len);
    memcpy(put_cmd_expected_msg+expected_hdr_len, data, data_len);
    memcpy(put_cmd_expected_msg+expected_hdr_len+data_len, "\r\n\0", 3);
    put_cmd_expected_msg_len = strlen(put_cmd_expected_msg);

    put_cmd_msg = bsc_gen_put_msg( &put_cmd_msg_len, 1, 2, 3, data_len, data );
    hdr = bsc_gen_put_hdr( &hdr_len, 1, 2, 3, data_len );

    ok( put_cmd_msg_len == put_cmd_expected_msg_len,
        "bsc_gen_put_msg -> length" );

    ok( strcmp( hdr, expected_hdr ) == 0,
        "bsc_gen_put_hdr" );

    ok( strcmp( put_cmd_msg, put_cmd_expected_msg ) == 0,
        "bsc_gen_put_msg" );

    /* use command */
    tube_len = strlen(tube);
    memcpy(use_cmd_expected_msg, "use ", 4);
    memcpy(use_cmd_expected_msg+4, tube, tube_len);
    memcpy(use_cmd_expected_msg+4+tube_len, "\r\n\0", 3);
    use_cmd_expected_msg_len = strlen(use_cmd_expected_msg);

    use_cmd_msg = bsc_gen_use_msg( &use_cmd_msg_len, tube );

    ok( use_cmd_msg_len == use_cmd_expected_msg_len,
        "bsc_gen_use_msg -> length" );

    ok( strcmp( use_cmd_msg, use_cmd_expected_msg) == 0, 
        "bsc_gen_use_msg" );

    /*-----------------------------------------------------------------------------
     *  the tests from now on require a running beanstalkd on localhost:DEFALT_PORT
     *-----------------------------------------------------------------------------*/

    /* create a connetion to beanstalkd on localhost */
    if ( ( fd = tcp_client("localhost", BSC_DEFAULT_PORT, 0, error_str) ) == SOCKERR )
    {
        skip(ALL_TESTS-5, "failed to create tcp client");
        return exit_status();
    }

    /* put test */
    if ( ( bytes_wrote = write(fd, put_cmd_msg, put_cmd_msg_len) ) == put_cmd_msg_len ) {
        if ( ( bytes_read = read(fd, res, 255) ) > 0 ) {
            res[bytes_read] = '\0';
            res_t = bsc_get_put_res( res, &id );
            ok( res_t == BSC_PUT_RES_INSERTED, "put successful" );
            ok( id != 0, "put successful -> got id" );
        }
        else {
            skip( 2, "read error" );
        }
    }
    else {
        skip( 2, "write error" );
    }

    /* use test */
    if ( ( res_tube = (char *)malloc( sizeof(char) *BSC_MAX_TUBE_NAME+1 ) ) == NULL )
        skip( 2, "malloc error" );
    else if ( ( bytes_wrote = write(fd, use_cmd_msg, use_cmd_msg_len) ) == use_cmd_msg_len ) {
        if ( ( bytes_read = read(fd, res, 255) ) > 0 ) {
            res[bytes_read] = '\0';
            res_t = bsc_get_use_res( res, &res_tube );
            ok( res_t == BSC_USE_RES_USING, "use successful" );
            ok( strcmp( tube, res_tube ) == 0, "use successful -> got tube" );
        }
        else {
            skip( 2, "read error" );
        }
    }
    else {
        skip( 2, "write error" );
    }

    return exit_status();
}
