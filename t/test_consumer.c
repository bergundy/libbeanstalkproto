/*
 * =====================================================================================
 *
 *       Filename:  test_consumer.c
 *
 *    Description:  test suite for consumer functions from beanstalk client library
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

#define ALL_TESTS 28

int main() {
    plan_tests(ALL_TESTS);

    /* variable declaration / intialization */
    char     *reserve_cmd_msg,
             *reserve_data,
             *reserve_cmd_expected_msg = "reserve\r\n", 
             *reserve_with_to_cmd_expected_msg = "reserve-with-timeout 1\r\n",
             *reserve_test_response = "RESERVED 3 4\r\nbaba\r\n",

             *delete_cmd_msg,
             *delete_cmd_expected_msg = "delete 12345\r\n",
             *delete_test_response = "DELETED\r\n",

             *release_cmd_msg,
             *release_cmd_expected_msg = "release 12345 3 4\r\n",
             *release_test_response = "RELEASED\r\n",

             *bury_cmd_msg,
             *bury_cmd_expected_msg = "bury 12345 3\r\n",
             *bury_test_response = "BURIED\r\n",

             *touch_cmd_msg,
             *touch_cmd_expected_msg = "touch 12345\r\n",
             *touch_test_response = "TOUCHED\r\n",

             *watch_cmd_msg,
             *watch_cmd_expected_msg = "watch baba\r\n",
             *watch_test_response = "WATCHING 1\r\n", 

             *ignore_cmd_msg,
             *ignore_cmd_expected_msg = "ignore baba\r\n";

    int      reserve_cmd_msg_len,
             reserve_cmd_expected_msg_len,

             delete_cmd_msg_len,
             delete_cmd_expected_msg_len,

             release_cmd_msg_len,
             release_cmd_expected_msg_len,

             bury_cmd_msg_len,
             bury_cmd_expected_msg_len,

             touch_cmd_msg_len,
             touch_cmd_expected_msg_len,

             watch_cmd_msg_len,
             watch_cmd_expected_msg_len,

             ignore_cmd_msg_len,
             ignore_cmd_expected_msg_len;

    uint32_t id = 0, bytes = 0;
    bsc_response_t res_t = BSC_UNRECOGNIZED_RESPONSE;
    
    /*-----------------------------------------------------------------------------
     *  testing command generation 
     *-----------------------------------------------------------------------------*/

    /* reserve command */
    reserve_cmd_expected_msg_len = strlen(reserve_cmd_expected_msg);

    reserve_cmd_msg = bsc_gen_reserve_msg( &reserve_cmd_msg_len );

    ok( reserve_cmd_msg_len == reserve_cmd_expected_msg_len,
        "bsc_gen_reserve_msg -> length" );

    ok( strcmp( reserve_cmd_msg, reserve_cmd_expected_msg) == 0, 
        "bsc_gen_reserve_msg" );

    /* reserve-with-timeout command */
    reserve_cmd_expected_msg_len = strlen(reserve_with_to_cmd_expected_msg);

    reserve_cmd_msg = bsc_gen_reserve_with_to_msg( &reserve_cmd_msg_len, 1 );

    ok( reserve_cmd_msg_len == reserve_cmd_expected_msg_len,
        "bsc_gen_reserve_with_to_msg -> length" );

    ok( strcmp( reserve_cmd_msg, reserve_with_to_cmd_expected_msg) == 0, 
        "bsc_gen_reserve_with_to_msg" );

    /* reserve output */
    res_t = bsc_get_reserve_res( reserve_test_response, strlen(reserve_test_response), &id, &bytes, &reserve_data, 1);
    ok( strcmp(reserve_data, "baba") == 0, "bsc_get_reserve_res -> data (dup)" );
    ok( id == 3, "bsc_get_reserve_res -> id" );
    ok( bytes == 4, "bsc_get_reserve_res -> bytes" );

    char *tmp_str = strdup(reserve_test_response);
    res_t = bsc_get_reserve_res( tmp_str, strlen(reserve_test_response), &id, &bytes, &reserve_data, 0);
    ok( strcmp(reserve_data, "baba") == 0, "bsc_get_reserve_res -> data (no dup)" );

    /* delete command */
    id = 12345;
    delete_cmd_expected_msg_len = strlen(delete_cmd_expected_msg);

    delete_cmd_msg = bsc_gen_delete_msg( &delete_cmd_msg_len, id );

    ok( delete_cmd_msg_len == delete_cmd_expected_msg_len,
        "bsc_gen_delete_msg -> length" );

    ok( strcmp( delete_cmd_msg, delete_cmd_expected_msg) == 0, 
        "bsc_gen_delete_msg" );

    res_t = bsc_get_delete_res( delete_test_response );
    ok( res_t == BSC_DELETE_RES_DELETED, "bsc_get_delete_res" );
    
    /* release command */
    id = 12345;
    release_cmd_expected_msg_len = strlen(release_cmd_expected_msg);

    release_cmd_msg = bsc_gen_release_msg( &release_cmd_msg_len, id, 3, 4 );

    ok( release_cmd_msg_len == release_cmd_expected_msg_len,
        "bsc_gen_release_msg -> length" );

    ok( strcmp( release_cmd_msg, release_cmd_expected_msg) == 0, 
        "bsc_gen_release_msg" );

    res_t = bsc_get_release_res( release_test_response );
    ok( res_t == BSC_RELEASE_RES_RELEASED, "bsc_get_release_res" );

    /* bury command */
    id = 12345;
    bury_cmd_expected_msg_len = strlen(bury_cmd_expected_msg);

    bury_cmd_msg = bsc_gen_bury_msg( &bury_cmd_msg_len, id, 3 );

    ok( bury_cmd_msg_len == bury_cmd_expected_msg_len,
        "bsc_gen_bury_msg -> length" );

    ok( strcmp( bury_cmd_msg, bury_cmd_expected_msg) == 0, 
        "bsc_gen_bury_msg" );

    res_t = bsc_get_bury_res( bury_test_response );
    ok( res_t == BSC_RES_BURIED, "bsc_get_bury_res" );

    /* touch command */
    id = 12345;
    touch_cmd_expected_msg_len = strlen(touch_cmd_expected_msg);

    touch_cmd_msg = bsc_gen_touch_msg( &touch_cmd_msg_len, id );

    ok( touch_cmd_msg_len == touch_cmd_expected_msg_len,
        "bsc_gen_touch_msg -> length" );

    ok( strcmp( touch_cmd_msg, touch_cmd_expected_msg) == 0, 
        "bsc_gen_touch_msg" );

    res_t = bsc_get_touch_res( touch_test_response );
    ok( res_t == BSC_TOUCH_RES_TOUCHED, "bsc_get_touch_res" );

    /* watch command */
    watch_cmd_expected_msg_len = strlen(watch_cmd_expected_msg);

    watch_cmd_msg = bsc_gen_watch_msg( &watch_cmd_msg_len, "baba" );

    ok( watch_cmd_msg_len == watch_cmd_expected_msg_len,
        "bsc_gen_watch_msg -> length" );

    ok( strcmp( watch_cmd_msg, watch_cmd_expected_msg) == 0, 
        "bsc_gen_watch_msg" );

    uint32_t count = 0;
    res_t = bsc_get_watch_res( watch_test_response, &count );
    ok( res_t == BSC_RES_WATCHING, "bsc_get_watch_res" );
    ok( count == 1, "bsc_get_watch_res -> count" );

    /* ignore command */
    ignore_cmd_expected_msg_len = strlen(ignore_cmd_expected_msg);

    ignore_cmd_msg = bsc_gen_ignore_msg( &ignore_cmd_msg_len, "baba" );

    ok( ignore_cmd_msg_len == ignore_cmd_expected_msg_len,
        "bsc_gen_ignore_msg -> length" );

    ok( strcmp( ignore_cmd_msg, ignore_cmd_expected_msg) == 0, 
        "bsc_gen_ignore_msg" );

    count = 0;
    res_t = bsc_get_ignore_res( watch_test_response, &count );
    ok( res_t == BSC_RES_WATCHING, "bsc_get_ignore_res" );
    ok( count == 1, "bsc_get_ignore_res -> count" );

    return exit_status();
}
