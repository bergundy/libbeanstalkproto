/*
 * =====================================================================================
 *
 *       Filename:  test_other.c
 *
 *    Description:  test suite for misc functions from beanstalk client library
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
#include <string.h>
#include "beanstalkclient.h"

#define ALL_TESTS 10

int main() {
    plan_tests(ALL_TESTS);

    /* variable declaration / intialization */
    char     *peek_cmd_msg,
             *peek_data,
             *peek_cmd_expected_msg = "peek 1\r\n", 
             *peek_test_response = "FOUND 3 4\r\nbaba\r\n",
             *kick_cmd_msg,
             *kick_cmd_expected_msg = "kick 4\r\n",
             *kick_test_response = "KICKED 3\r\n";

    int      peek_cmd_msg_len,
             peek_cmd_expected_msg_len,
             kick_cmd_msg_len,
             kick_cmd_expected_msg_len;

    uint32_t id = 0, bytes = 0;
    bsc_response_t res_t = BSC_UNRECOGNIZED_RESPONSE;
    
    /*-----------------------------------------------------------------------------
     *  testing command generation 
     *-----------------------------------------------------------------------------*/

    /* peek command */
    peek_cmd_expected_msg_len = strlen(peek_cmd_expected_msg);

    peek_cmd_msg = bsc_gen_peek_msg( &peek_cmd_msg_len, 1 );

    ok( peek_cmd_msg_len == peek_cmd_expected_msg_len,
        "bsc_gen_peek_msg -> length" );

    ok( strcmp( peek_cmd_msg, peek_cmd_expected_msg) == 0, 
        "bsc_gen_peek_msg" );

    /* peek output */
    res_t = bsc_get_peek_res( peek_test_response, strlen(peek_test_response), &id, &bytes, &peek_data, 1);
    ok( strcmp(peek_data, "baba") == 0, "bsc_get_peek_res -> data (dup)" );
    ok( id == 3, "bsc_get_peek_res -> id" );
    ok( bytes == 4, "bsc_get_peek_res -> bytes" );

    char *tmp_str = strdup(peek_test_response);
    res_t = bsc_get_peek_res( tmp_str, strlen(peek_test_response), &id, &bytes, &peek_data, 0);
    ok( strcmp(peek_data, "baba") == 0, "bsc_get_peek_res -> data (no dup)" );

    /* kick command */
    kick_cmd_expected_msg_len = strlen(kick_cmd_expected_msg);

    kick_cmd_msg = bsc_gen_kick_msg( &kick_cmd_msg_len, 4 );

    ok( kick_cmd_msg_len == kick_cmd_expected_msg_len,
        "bsc_gen_kick_msg -> length" );

    ok( strcmp( kick_cmd_msg, kick_cmd_expected_msg) == 0, 
        "bsc_gen_kick_msg" );

    uint32_t count = 0;
    res_t = bsc_get_kick_res( kick_test_response, &count );
    ok( res_t == BSC_KICK_RES_KICKED, "bsc_get_kick_res" );
    ok( count == 3, "bsc_get_kick_res -> count" );

    return exit_status();
}
