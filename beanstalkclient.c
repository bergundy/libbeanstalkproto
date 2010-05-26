/*
 * =====================================================================================
 *
 *       Filename:  beanstalkclient.c
 *
 *    Description:  c implementation of beanstalk client protocol
 *
 *        Version:  1.0
 *        Created:  05/23/2010 03:55:21 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Roey Berman, royb@walla.net.il
 *        Company:  Walla!
 *
 * =====================================================================================
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "beanstalkclient.h"

static const char const *bsc_response_str[] = {
    /* general errors */
    "OUT_OF_MEMORY", "INTERNAL_ERROR", "BAD_FORMAT", "UNKOWN_COMMAND",
    /* general responses */
    "BURIED", "NOT_FOUND", "WATCHING",
    /* put cmd results */
    "INSERTED", "EXPECTED_CRLF", "JOB_TOO_BIG", "DRAINING", 
    /* use cmd result */
    "USING",
    /* reserve cmd result */
    "RESERVED", "DEADLINE_SOON", "TIMED_OUT",
    /* delete cmd result */
    "DELETED",
    /* release cmd result */
    "RELEASED",
    /* touch cmd result */
    "TOUCHED",
    /* ignore cmd result */
    "NOT_IGNORED",
    /* peek cmd result */
    "FOUND",
    /* kick cmd result */
    "KICKED"
};

static const size_t bsc_response_strlen[] = {
    13, 14, 10, 14,
    6, 9, 8,
    8, 13, 11, 8,
    5,
    8, 13, 9,
    7, 
    8,
    7,
    11,
    5,
    6
};
#define BSC_GET_ID_BYTES_DATA \
        p =  (char *)response + bsc_response_strlen[response_t] + 1;\
        bytes_matched = sscanf(p, "%u %u", id, bytes);\
        if (bytes_matched == EOF)\
            response_t = BSC_UNRECOGNIZED_RESPONSE;\
        else {\
            p += bytes_matched + 3;\
            if (dup) {\
                *data = NULL;\
                if ( ( *data = (char *)malloc( sizeof(char) * (*bytes+1) ) ) != NULL ) {\
                    memcpy(*data, p, *bytes);\
                    (*data)[*bytes] = '\0';\
                }\
            }\
            else {\
                p[*bytes] = '\0';\
                *data = p;\
            }\
        }\


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  bsc_get_response_t
 *  Description:  
 * =====================================================================================
 */
inline static bsc_response_t bsc_get_response_t( const char *response, const bsc_response_t const possibilities[] )
{
    register unsigned int i;

    static const bsc_response_t const bsc_general_error_responses[] = {
         BSC_RES_OUT_OF_MEMORY,
         BSC_RES_INTERNAL_ERROR,
         BSC_RES_BAD_FORMAT,
         BSC_RES_UNKOWN_COMMAND
    };

    for (i = 0; i < sizeof(possibilities); ++i) {
        printf("%d: %s\n", i, bsc_response_str[possibilities[i]] );
        if ( ( strncmp(response, bsc_response_str[possibilities[i]],
                bsc_response_strlen[possibilities[i]]) ) == 0 )
            return possibilities[i];
    }

    for (i = 0; i < sizeof(bsc_general_error_responses); ++i)
        if ( ( strncmp(response, bsc_response_str[bsc_general_error_responses[i]],
                bsc_response_strlen[bsc_general_error_responses[i]]) ) == 0 )
            return bsc_general_error_responses[i];

    return BSC_UNRECOGNIZED_RESPONSE;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  bsc_gen_put_iov
 *  Description:  
 * =====================================================================================
 */
inline struct iovec *bsc_gen_put_iov( uint32_t   priority,
                               uint32_t   delay,
                               uint32_t   ttr,
                               size_t     bytes, 
                               char       *data
                             )
{
    struct iovec *iov;
    char *hdr = NULL, *hdr_end = NULL;
    ssize_t hdr_len;

    if ( ( iov = (struct iovec *)malloc( sizeof(struct iovec) * 3 ) ) == NULL )
        return NULL;

    if ( ( hdr = bsc_gen_put_hdr(&hdr_len, priority, delay, ttr, bytes) ) == NULL )
        goto error_hdr;

    if ( ( hdr_end = strdup("\r\n") ) == NULL )
        goto error_hdr_end;

    iov[0].iov_base = hdr;
    iov[0].iov_len  = hdr_len;
    iov[1].iov_base = data;
    iov[1].iov_len  = bytes;
    iov[2].iov_base = hdr_end;
    iov[2].iov_len  = 2;

    return iov;

error_hdr_end:
    free(hdr);

error_hdr:
    free(iov);
    return NULL;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  bsc_gen_put_msg
 *  Description:  
 * =====================================================================================
 */
inline char *bsc_gen_put_msg( int        *msg_len,
                       uint32_t   priority,
                       uint32_t   delay,
                       uint32_t   ttr,
                       size_t     bytes,
                       const char *data
                     )
{
    char *msg = NULL, *p = NULL;

    if ( ( msg = (char *)malloc( sizeof(char) * (BSC_PUT_CMD_HDR_SIZE + bytes + 2) ) ) == NULL )
        return NULL;

    *msg_len = sprintf(msg, "put %u %u %u %u\r\n", priority, delay, ttr, bytes );

    p = msg + *msg_len;
    memcpy(msg+*msg_len, data, bytes);
    p += bytes;
    memcpy(p, "\r\n\0", 3);
    *msg_len += bytes + 2;

    return msg;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  bsc_gen_put_hdr
 *  Description:  
 * =====================================================================================
 */
inline char *bsc_gen_put_hdr( int        *hdr_len,
                       uint32_t   priority,
                       uint32_t   delay,
                       uint32_t   ttr,
                       size_t     bytes
                     )
{
    char *hdr = NULL;

    if ( ( hdr = (char *)malloc( sizeof(char) * BSC_PUT_CMD_HDR_SIZE ) ) == NULL )
        return NULL;

    *hdr_len = sprintf(hdr, "put %u %u %u %u\r\n", priority, delay, ttr, bytes );

    return hdr;
}
    
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  bsc_gen_use_msg
 *  Description:  
 * =====================================================================================
 */
inline char *bsc_gen_use_msg( int *msg_len, const char *tube_name )
{
    char *msg = NULL;
    /* use \r\n\0 */
    static const int use_msg_len = 7;

    if ( ( msg = (char *)malloc( sizeof(char) * (use_msg_len+strlen(tube_name))) ) == NULL )
        return NULL;

    *msg_len = sprintf(msg, "use %s\r\n", tube_name );

    return msg;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  bsc_get_put_res
 *  Description:  
 * =====================================================================================
 */
inline bsc_response_t bsc_get_put_res( const char *response, uint32_t *id )
{
    static const bsc_response_t const bsc_put_cmd_responses[] = {
         BSC_PUT_RES_INSERTED,
         BSC_RES_BURIED,
         BSC_PUT_RES_EXPECTED_CRLF,
         BSC_PUT_RES_JOB_TOO_BIG, 
         BSC_PUT_RES_DRAINING 
    };

    bsc_response_t response_t;

    response_t = bsc_get_response_t(response, bsc_put_cmd_responses);

    if ( response_t == BSC_PUT_RES_INSERTED )
        *id = strtoul(response+bsc_response_strlen[response_t], NULL, 10);

    return response_t;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  bsc_get_use_res
 *  Description:  
 * =====================================================================================
 */
inline bsc_response_t bsc_get_use_res( const char *response, char **tube_name )
{
    static const bsc_response_t const bsc_use_cmd_responses[] = {
         BSC_USE_RES_USING
    };

    bsc_response_t response_t;
    char *p;

    response_t = bsc_get_response_t(response, bsc_use_cmd_responses);

    if ( response_t == BSC_USE_RES_USING ) {
        *tube_name = strdup(response+1+bsc_response_strlen[response_t]);
        if( ( p = strchr( *tube_name, '\r' ) ) != NULL )
            *p = '\0';
    }

    return response_t;
}


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  bsc_gen_reserve_msg
 *  Description:  
 * =====================================================================================
 */
inline char *bsc_gen_reserve_msg( int *msg_len )
{
    static const char reserve_msg[] = "reserve\r\n";
    static const int  reserve_msg_len = 9;
    char *msg = NULL;

    if ( ( msg = strdup(reserve_msg) ) == NULL )
        return NULL;

    *msg_len = reserve_msg_len;
    return msg;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  bsc_gen_reserve_with_to_msg
 *  Description:  
 * =====================================================================================
 */
inline char *bsc_gen_reserve_with_to_msg( int *msg_len, uint32_t timeout )
{
    /* reserve-with-timeout <timeout>\r\n\0 */
    static const int reserve_msg_len = 20 + 3 + 1 + UINT32_T_STRLEN;
    char *msg = NULL;

    if ( ( msg = (char *)malloc( sizeof(char) * reserve_msg_len ) ) == NULL )
        return NULL;

    *msg_len = sprintf(msg, "reserve-with-timeout %u\r\n", timeout );

    return msg;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  bsc_gen_delete_msg
 *  Description:  
 * =====================================================================================
 */
inline char *bsc_gen_delete_msg( int *msg_len, uint32_t id )
{
    /* delete <id>\r\n\0 */
    static const int delete_msg_len = 6 + 3 + 1 + UINT32_T_STRLEN;
    char *msg = NULL;

    if ( ( msg = (char *)malloc( sizeof(char) * delete_msg_len ) ) == NULL )
        return NULL;

    *msg_len = sprintf(msg, "delete %u\r\n", id );

    return msg;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  bsc_gen_release_msg
 *  Description:  
 * =====================================================================================
 */
inline char *bsc_gen_release_msg( int *msg_len, uint32_t id, uint32_t priority, uint32_t delay )
{
    /* release <id> <priority> <delay>\r\n\0 */
    static const int release_msg_len = 7 + 3 + 3 + 3 * UINT32_T_STRLEN;
    char *msg = NULL;

    if ( ( msg = (char *)malloc( sizeof(char) * release_msg_len ) ) == NULL )
        return NULL;

    *msg_len = sprintf(msg, "release %u %u %u\r\n", id, priority, delay );

    return msg;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  bsc_gen_bury_msg
 *  Description:  
 * =====================================================================================
 */
inline char *bsc_gen_bury_msg( int *msg_len, uint32_t id, uint32_t priority )
{
    /* bury <id> <priority>\r\n\0 */
    static const int bury_msg_len = 4 + 3 + 2 + 2 * UINT32_T_STRLEN;
    char *msg = NULL;

    if ( ( msg = (char *)malloc( sizeof(char) * bury_msg_len ) ) == NULL )
        return NULL;

    *msg_len = sprintf(msg, "bury %u %u\r\n", id, priority );

    return msg;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  bsc_gen_touch_msg
 *  Description:  
 * =====================================================================================
 */
inline char *bsc_gen_touch_msg( int *msg_len, uint32_t id )
{
    /* touch <id>\r\n\0 */
    static const int touch_msg_len = 5 + 3 + 1 + UINT32_T_STRLEN;
    char *msg = NULL;

    if ( ( msg = (char *)malloc( sizeof(char) * touch_msg_len ) ) == NULL )
        return NULL;

    *msg_len = sprintf(msg, "touch %u\r\n", id );

    return msg;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  bsc_gen_watch_msg
 *  Description:  
 * =====================================================================================
 */
inline char *bsc_gen_watch_msg( int *msg_len, const char *tube_name )
{
    char *msg = NULL;
    /* watch \r\n\0 */
    static const int watch_msg_len = 5 + 3 + 1;

    if ( ( msg = (char *)malloc( sizeof(char) * (watch_msg_len+strlen(tube_name))) ) == NULL )
        return NULL;

    *msg_len = sprintf(msg, "watch %s\r\n", tube_name );

    return msg;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  bsc_gen_ignore_msg
 *  Description:  
 * =====================================================================================
 */
inline char *bsc_gen_ignore_msg( int *msg_len, const char *tube_name )
{
    char *msg = NULL;
    /* ignore \r\n\0 */
    static const int ignore_msg_len = 6 + 3 + 1;

    if ( ( msg = (char *)malloc( sizeof(char) * (ignore_msg_len+strlen(tube_name))) ) == NULL )
        return NULL;

    *msg_len = sprintf(msg, "ignore %s\r\n", tube_name );

    return msg;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  bsc_get_reserve_res
 *  Description:  gets bsc_response_t from response
 *                if response is "RESERVED": 
 *                  id, bytes and data will be put in the proper supplied variables
 *                the dup flag indicated weather to copy data from response or not
 *                NOTE: if the dup flag is not set response will be altered
 *      Returns:  the response code (bsc_respone_t)
 * =====================================================================================
 */
inline bsc_response_t bsc_get_reserve_res( const char *response,
                                    ssize_t response_len,
                                    uint32_t *id,
                                    uint32_t *bytes,
                                    char **data,
                                    int dup )
{
    static const bsc_response_t const bsc_reserve_cmd_responses[] = {
        BSC_RESERVE_RES_RESERVED,
        BSC_RESERVE_RES_DEADLINE_SOON,
        BSC_RESERVE_RES_TIMED_OUT
    };
    /* A timeout value of 0 will cause the server to immediately return either a
    response or TIMED_OUT. A positive value of timeout will limit the amount of
    time the client will block on the reserve request until a job becomes
    available.

    During the TTR of a reserved job, the last second is kept by the server as a
    safety margin, during which the client will not be made to wait for another
    job. If the client issues a reserve command during the safety margin, or if
    the safety margin arrives while the client is waiting on a reserve command,
    the server will respond with: */

    bsc_response_t response_t;
    char *p = NULL;
    ssize_t bytes_matched = EOF;

    response_t = bsc_get_response_t(response, bsc_reserve_cmd_responses);
    if ( response_t == BSC_RESERVE_RES_RESERVED ) {
        BSC_GET_ID_BYTES_DATA
    }

    return response_t;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  bsc_get_delete_res
 *  Description:  
 *      Returns:  the response code (bsc_respone_t)
 * =====================================================================================
 */
inline bsc_response_t bsc_get_delete_res( const char *response )
{
    static const bsc_response_t const bsc_delete_cmd_responses[] = {
         BSC_DELETE_RES_DELETED,
         BSC_RES_NOT_FOUND
    };

    return bsc_get_response_t(response, bsc_delete_cmd_responses);
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  bsc_get_release_res
 *  Description:  
 *      Returns:  the response code (bsc_respone_t)
 * =====================================================================================
 */
inline bsc_response_t bsc_get_release_res( const char *response )
{
    static const bsc_response_t const bsc_release_cmd_responses[] = {
         BSC_RELEASE_RES_RELEASED,
         BSC_RES_BURIED,
         BSC_RES_NOT_FOUND
    };

    return bsc_get_response_t(response, bsc_release_cmd_responses);
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  bsc_get_bury_res
 *  Description:  
 *      Returns:  the response code (bsc_respone_t)
 * =====================================================================================
 */
inline bsc_response_t bsc_get_bury_res( const char *response )
{
    static const bsc_response_t const bsc_bury_cmd_responses[] = {
         BSC_RES_BURIED,
         BSC_RES_NOT_FOUND
    };

    return bsc_get_response_t(response, bsc_bury_cmd_responses);
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  bsc_get_touch_res
 *  Description:  
 *      Returns:  the response code (bsc_respone_t)
 * =====================================================================================
 */
inline bsc_response_t bsc_get_touch_res( const char *response )
{
    static const bsc_response_t const bsc_touch_cmd_responses[] = {
         BSC_TOUCH_RES_TOUCHED,
         BSC_RES_NOT_FOUND
    };

    return bsc_get_response_t(response, bsc_touch_cmd_responses);
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  bsc_get_watch_res
 *  Description:  
 *      Returns:  the response code (bsc_respone_t)
 * =====================================================================================
 */
inline bsc_response_t bsc_get_watch_res( const char *response, uint32_t *count )
{
    static const bsc_response_t const bsc_watch_cmd_responses[] = {
        BSC_RES_WATCHING
    };

    bsc_response_t response_t;
    char *p = NULL;
    ssize_t bytes_matched = EOF;

    response_t = bsc_get_response_t(response, bsc_watch_cmd_responses);
    if ( response_t == BSC_RES_WATCHING ) {
        p =  (char *)response + bsc_response_strlen[response_t] + 1;
        bytes_matched = sscanf(p, "%u", count );

        // got bad response format
        if (bytes_matched == EOF)
            return BSC_UNRECOGNIZED_RESPONSE;
    }

    return response_t;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  bsc_get_ignore_res
 *  Description:  
 *      Returns:  the response code (bsc_respone_t)
 * =====================================================================================
 */
inline bsc_response_t bsc_get_ignore_res( const char *response, uint32_t *count )
{
    static const bsc_response_t const bsc_ignore_cmd_responses[] = {
        BSC_RES_WATCHING,
        BSC_IGNORE_RES_NOT_IGNORED
    };

    bsc_response_t response_t;
    char *p = NULL;
    ssize_t bytes_matched = EOF;

    response_t = bsc_get_response_t(response, bsc_ignore_cmd_responses);
    if ( response_t == BSC_RES_WATCHING ) {
        p =  (char *)response + bsc_response_strlen[response_t] + 1;
        bytes_matched = sscanf(p, "%u", count );

        // got bad response format
        if (bytes_matched == EOF)
            return BSC_UNRECOGNIZED_RESPONSE;
    }

    return response_t;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  bsc_gen_peek_msg
 *  Description:  
 * =====================================================================================
 */
inline char *bsc_gen_peek_msg( int *msg_len, uint32_t id )
{
    /* peek <id>\r\n\0 */
    static const int peek_msg_len = 4 + 3 + 1 + UINT32_T_STRLEN;
    char *msg = NULL;

    if ( ( msg = (char *)malloc( sizeof(char) * peek_msg_len ) ) == NULL )
        return NULL;

    *msg_len = sprintf(msg, "peek %u\r\n", id );

    return msg;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  bsc_gen_peek_ready_msg
 *  Description:  
 * =====================================================================================
 */
inline char *bsc_gen_peek_ready_msg( int *msg_len )
{
    static const char peek_ready_msg[] = "peek-ready\r\n";
    static const int  peek_ready_msg_len = 13;
    char *msg = NULL;

    if ( ( msg = strdup(peek_ready_msg) ) == NULL )
        return NULL;

    *msg_len = peek_ready_msg_len;
    return msg;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  bsc_gen_peek_delayed_msg
 *  Description:  
 * =====================================================================================
 */
inline char *bsc_gen_peek_delayed_msg( int *msg_len )
{
    static const char peek_delayed_msg[] = "peek-delayed\r\n";
    static const int  peek_delayed_msg_len = 15;
    char *msg = NULL;

    if ( ( msg = strdup(peek_delayed_msg) ) == NULL )
        return NULL;

    *msg_len = peek_delayed_msg_len;
    return msg;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  bsc_gen_peek_buried_msg
 *  Description:  
 * =====================================================================================
 */
inline char *bsc_gen_peek_buried_msg( int *msg_len )
{
    static const char peek_buried_msg[] = "peek-buried\r\n";
    static const int  peek_buried_msg_len = 14;
    char *msg = NULL;

    if ( ( msg = strdup(peek_buried_msg) ) == NULL )
        return NULL;

    *msg_len = peek_buried_msg_len;
    return msg;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  bsc_gen_kick_msg
 *  Description:  
 * =====================================================================================
 */
inline char *bsc_gen_kick_msg( int *msg_len, uint32_t bound )
{
    /* kick <id>\r\n\0 */
    static const int kick_msg_len = 4 + 3 + 1 + UINT32_T_STRLEN;
    char *msg = NULL;

    if ( ( msg = (char *)malloc( sizeof(char) * kick_msg_len ) ) == NULL )
        return NULL;

    *msg_len = sprintf(msg, "kick %u\r\n", bound );

    return msg;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  bsc_get_peek_res
 *  Description:  gets bsc_response_t from response
 *                if response is "FOUND": 
 *                  id, bytes and data will be put in the proper supplied variables
 *                the dup flag indicated weather to copy data from response or not
 *                NOTE: if the dup flag is not set response will be altered
 *      Returns:  the response code (bsc_respone_t)
 * =====================================================================================
 */
inline bsc_response_t bsc_get_peek_res( const char *response,
                                    ssize_t response_len,
                                    uint32_t *id,
                                    uint32_t *bytes,
                                    char **data,
                                    int dup )
{
    static const bsc_response_t const bsc_peek_cmd_responses[] = {
        BSC_PEEK_RES_FOUND,
        BSC_RES_NOT_FOUND
    };

    bsc_response_t response_t;
    char *p = NULL;
    ssize_t bytes_matched = EOF;

    response_t = bsc_get_response_t(response, bsc_peek_cmd_responses);
    if ( response_t == BSC_PEEK_RES_FOUND ) {
        BSC_GET_ID_BYTES_DATA
    }

    return response_t;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  bsc_get_kick_res
 *  Description:  
 *      Returns:  the response code (bsc_respone_t)
 * =====================================================================================
 */
inline bsc_response_t bsc_get_kick_res( const char *response, uint32_t *count )
{
    static const bsc_response_t const bsc_kick_cmd_responses[] = {
        BSC_KICK_RES_KICKED
    };

    bsc_response_t response_t;
    char *p = NULL;
    ssize_t bytes_matched = EOF;

    response_t = bsc_get_response_t(response, bsc_kick_cmd_responses);
    if ( response_t == BSC_KICK_RES_KICKED ) {
        p =  (char *)response + bsc_response_strlen[response_t] + 1;
        bytes_matched = sscanf(p, "%u", count );

        // got bad response format
        if (bytes_matched == EOF)
            return BSC_UNRECOGNIZED_RESPONSE;
    }

    return response_t;
}

