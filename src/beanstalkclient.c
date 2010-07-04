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
    "OUT_OF_MEMORY", "INTERNAL_ERROR", "BAD_FORMAT", "UNKNOWN_COMMAND",
    /* general responses */
    "OK", "BURIED", "NOT_FOUND", "WATCHING",
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
    "KICKED",
    /* pause-tube cmd result */
    "PAUSED"
};

static const size_t bsc_response_strlen[] = {
    13, 14, 10, 14,
    2, 6, 9, 8,
    8, 13, 11, 8,
    5,
    8, 13, 9,
    7, 
    8,
    7,
    11,
    5,
    6,
    6
};

#define BSC_GET_ID_BYTES                                                             \
    p =  (char *)response + bsc_response_strlen[response_t] + 1;                     \
    char *p_tmp = NULL;                                                              \
    *id = strtoul(p, &p_tmp, 10);                                                    \
    if ( ( p = p_tmp ) == NULL)                                                      \
        response_t = BSC_UNRECOGNIZED_RESPONSE;                                      \
    p_tmp = NULL;                                                                    \
    *bytes = strtoul(p, &p_tmp, 10);                                                 \
    if ( ( p = p_tmp ) == NULL)                                                      \
        response_t = BSC_UNRECOGNIZED_RESPONSE;

static const bsc_response_t const bsc_general_error_responses[] = {
     BSC_RES_OUT_OF_MEMORY,
     BSC_RES_INTERNAL_ERROR,
     BSC_RES_BAD_FORMAT,
     BSC_RES_UNKNOWN_COMMAND
};


#define bsc_get_response_t( response, possibilities )                                \
    register unsigned int i;                                                         \
    response_t = BSC_UNRECOGNIZED_RESPONSE;                                          \
    for (i = 0; i < sizeof(possibilities)/sizeof(bsc_response_t); ++i)               \
        if ( ( strncmp(response, bsc_response_str[possibilities[i]],                 \
                bsc_response_strlen[possibilities[i]]) ) == 0 )                      \
        {                                                                            \
            response_t = possibilities[i];                                           \
            goto done;                                                               \
        }                                                                            \
    for (i = 0; i < sizeof(bsc_general_error_responses)/sizeof(bsc_response_t); ++i) \
        if ( ( strncmp(response, bsc_response_str[bsc_general_error_responses[i]],   \
                bsc_response_strlen[bsc_general_error_responses[i]]) ) == 0 )        \
        {                                                                            \
            response_t = bsc_general_error_responses[i];                             \
            goto done;                                                               \
        }                                                                            \
    done:

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
 *         Name:  bsc_gen_put_cmd
 *  Description:  
 * =====================================================================================
 */
inline char *bsc_gen_put_cmd( int        *cmd_len,
                              uint32_t   priority,
                              uint32_t   delay,
                              uint32_t   ttr,
                              size_t     bytes,
                              const char *data
                            )
{
    char *cmd = NULL, *p = NULL;

    if ( ( cmd = (char *)malloc( sizeof(char) * (BSC_PUT_CMD_HDR_SIZE + bytes + 2) ) ) == NULL )
        return NULL;

    *cmd_len = sprintf(cmd, "put %u %u %u %u\r\n", priority, delay, ttr, bytes );

    p = cmd + *cmd_len;
    memcpy(cmd+*cmd_len, data, bytes);
    p += bytes;
    memcpy(p, "\r\n\0", 3);
    *cmd_len += bytes + 2;

    return cmd;
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
 *         Name:  bsc_gen_use_cmd
 *  Description:  
 * =====================================================================================
 */
inline char *bsc_gen_use_cmd( int *cmd_len, const char *tube_name )
{
    char *cmd = NULL;
    /* use \r\n\0 */
    static const int use_cmd_len = 7;

    if ( ( cmd = (char *)malloc( sizeof(char) * (use_cmd_len+strlen(tube_name))) ) == NULL )
        return NULL;

    *cmd_len = sprintf(cmd, "use %s\r\n", tube_name );

    return cmd;
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

    bsc_get_response_t(response, bsc_put_cmd_responses);

    if ( response_t == BSC_PUT_RES_INSERTED || response_t == BSC_RES_BURIED )
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

    bsc_get_response_t(response, bsc_use_cmd_responses);

    if ( response_t == BSC_USE_RES_USING ) {
        *tube_name = strdup(response+1+bsc_response_strlen[response_t]);
        if( ( p = strchr( *tube_name, '\r' ) ) != NULL )
            *p = '\0';
    }

    return response_t;
}


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  bsc_gen_reserve_cmd
 *  Description:  
 * =====================================================================================
 */
inline char *bsc_gen_reserve_cmd( int *cmd_len )
{
    static const char reserve_cmd[] = "reserve\r\n";
    static const int  reserve_cmd_len = sizeof(reserve_cmd) / sizeof(char) - 1;
    char *cmd = NULL;

    if ( ( cmd = strdup(reserve_cmd) ) == NULL )
        return NULL;

    *cmd_len = reserve_cmd_len;
    return cmd;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  bsc_gen_reserve_with_to_cmd
 *  Description:  
 * =====================================================================================
 */
inline char *bsc_gen_reserve_with_to_cmd( int *cmd_len, uint32_t timeout )
{
    /* reserve-with-timeout <timeout>\r\n\0 */
    static const int reserve_cmd_len = 20 + 3 + 1 + UINT32_T_STRLEN;
    char *cmd = NULL;

    if ( ( cmd = (char *)malloc( sizeof(char) * reserve_cmd_len ) ) == NULL )
        return NULL;

    *cmd_len = sprintf(cmd, "reserve-with-timeout %u\r\n", timeout );

    return cmd;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  bsc_gen_delete_cmd
 *  Description:  
 * =====================================================================================
 */
inline char *bsc_gen_delete_cmd( int *cmd_len, uint32_t id )
{
    /* delete <id>\r\n\0 */
    static const int delete_cmd_len = 6 + 3 + 1 + UINT32_T_STRLEN;
    char *cmd = NULL;

    if ( ( cmd = (char *)malloc( sizeof(char) * delete_cmd_len ) ) == NULL )
        return NULL;

    *cmd_len = sprintf(cmd, "delete %u\r\n", id );

    return cmd;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  bsc_gen_release_cmd
 *  Description:  
 * =====================================================================================
 */
inline char *bsc_gen_release_cmd( int *cmd_len, uint32_t id, uint32_t priority, uint32_t delay )
{
    /* release <id> <priority> <delay>\r\n\0 */
    static const int release_cmd_len = 7 + 3 + 3 + 3 * UINT32_T_STRLEN;
    char *cmd = NULL;

    if ( ( cmd = (char *)malloc( sizeof(char) * release_cmd_len ) ) == NULL )
        return NULL;

    *cmd_len = sprintf(cmd, "release %u %u %u\r\n", id, priority, delay );

    return cmd;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  bsc_gen_bury_cmd
 *  Description:  
 * =====================================================================================
 */
inline char *bsc_gen_bury_cmd( int *cmd_len, uint32_t id, uint32_t priority )
{
    /* bury <id> <priority>\r\n\0 */
    static const int bury_cmd_len = 4 + 3 + 2 + 2 * UINT32_T_STRLEN;
    char *cmd = NULL;

    if ( ( cmd = (char *)malloc( sizeof(char) * bury_cmd_len ) ) == NULL )
        return NULL;

    *cmd_len = sprintf(cmd, "bury %u %u\r\n", id, priority );

    return cmd;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  bsc_gen_touch_cmd
 *  Description:  
 * =====================================================================================
 */
inline char *bsc_gen_touch_cmd( int *cmd_len, uint32_t id )
{
    /* touch <id>\r\n\0 */
    static const int touch_cmd_len = 5 + 3 + 1 + UINT32_T_STRLEN;
    char *cmd = NULL;

    if ( ( cmd = (char *)malloc( sizeof(char) * touch_cmd_len ) ) == NULL )
        return NULL;

    *cmd_len = sprintf(cmd, "touch %u\r\n", id );

    return cmd;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  bsc_gen_watch_cmd
 *  Description:  
 * =====================================================================================
 */
inline char *bsc_gen_watch_cmd( int *cmd_len, const char *tube_name )
{
    char *cmd = NULL;
    /* watch \r\n\0 */
    static const int watch_cmd_len = 5 + 3 + 1;

    if ( ( cmd = (char *)malloc( sizeof(char) * (watch_cmd_len+strlen(tube_name))) ) == NULL )
        return NULL;

    *cmd_len = sprintf(cmd, "watch %s\r\n", tube_name );

    return cmd;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  bsc_gen_ignore_cmd
 *  Description:  
 * =====================================================================================
 */
inline char *bsc_gen_ignore_cmd( int *cmd_len, const char *tube_name )
{
    char *cmd = NULL;
    /* ignore \r\n\0 */
    static const int ignore_cmd_len = 6 + 3 + 1;

    if ( ( cmd = (char *)malloc( sizeof(char) * (ignore_cmd_len+strlen(tube_name))) ) == NULL )
        return NULL;

    *cmd_len = sprintf(cmd, "ignore %s\r\n", tube_name );

    return cmd;
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
                                           uint32_t *id,
                                           uint32_t *bytes )
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

    bsc_get_response_t(response, bsc_reserve_cmd_responses);
    if ( response_t == BSC_RESERVE_RES_RESERVED ) {
        BSC_GET_ID_BYTES
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

    bsc_response_t response_t;
    bsc_get_response_t(response, bsc_delete_cmd_responses);
    return response_t;
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

    bsc_response_t response_t;
    bsc_get_response_t(response, bsc_release_cmd_responses);
    return response_t;
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

    bsc_response_t response_t;
    bsc_get_response_t(response, bsc_bury_cmd_responses);
    return response_t;
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

    bsc_response_t response_t;
    bsc_get_response_t(response, bsc_touch_cmd_responses);
    return response_t;
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
    ssize_t matched = EOF;

    bsc_get_response_t(response, bsc_watch_cmd_responses);
    if ( response_t == BSC_RES_WATCHING ) {
        p =  (char *)response + bsc_response_strlen[response_t] + 1;
        matched = sscanf(p, "%u", count );

        // got bad response format
        if (matched == EOF)
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
    ssize_t matched = EOF;

    bsc_get_response_t(response, bsc_ignore_cmd_responses);
    if ( response_t == BSC_RES_WATCHING ) {
        p =  (char *)response + bsc_response_strlen[response_t] + 1;
        matched = sscanf(p, "%u", count );

        // got bad response format
        if (matched == EOF)
            return BSC_UNRECOGNIZED_RESPONSE;
    }

    return response_t;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  bsc_gen_peek_cmd
 *  Description:  
 * =====================================================================================
 */
inline char *bsc_gen_peek_cmd( int *cmd_len, uint32_t id )
{
    /* peek <id>\r\n\0 */
    static const int peek_cmd_len = 4 + 3 + 1 + UINT32_T_STRLEN;
    char *cmd = NULL;

    if ( ( cmd = (char *)malloc( sizeof(char) * peek_cmd_len ) ) == NULL )
        return NULL;

    *cmd_len = sprintf(cmd, "peek %u\r\n", id );

    return cmd;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  bsc_gen_peek_ready_cmd
 *  Description:  
 * =====================================================================================
 */
inline char *bsc_gen_peek_ready_cmd( int *cmd_len )
{
    static const char peek_ready_cmd[] = "peek-ready\r\n";
    static const int  peek_ready_cmd_len = sizeof(peek_ready_cmd) / sizeof(char) - 1;
    char *cmd = NULL;

    if ( ( cmd = strdup(peek_ready_cmd) ) == NULL )
        return NULL;

    *cmd_len = peek_ready_cmd_len;
    return cmd;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  bsc_gen_peek_delayed_cmd
 *  Description:  
 * =====================================================================================
 */
inline char *bsc_gen_peek_delayed_cmd( int *cmd_len )
{
    static const char peek_delayed_cmd[] = "peek-delayed\r\n";
    static const int  peek_delayed_cmd_len = sizeof(peek_delayed_cmd) / sizeof(char) - 1;
    char *cmd = NULL;

    if ( ( cmd = strdup(peek_delayed_cmd) ) == NULL )
        return NULL;

    *cmd_len = peek_delayed_cmd_len;
    return cmd;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  bsc_gen_peek_buried_cmd
 *  Description:  
 * =====================================================================================
 */
inline char *bsc_gen_peek_buried_cmd( int *cmd_len )
{
    static const char peek_buried_cmd[] = "peek-buried\r\n";
    static const int  peek_buried_cmd_len = sizeof(peek_buried_cmd) / sizeof(char) - 1;
    char *cmd = NULL;

    if ( ( cmd = strdup(peek_buried_cmd) ) == NULL )
        return NULL;

    *cmd_len = peek_buried_cmd_len;
    return cmd;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  bsc_gen_kick_cmd
 *  Description:  
 * =====================================================================================
 */
inline char *bsc_gen_kick_cmd( int *cmd_len, uint32_t bound )
{
    /* kick <id>\r\n\0 */
    static const int kick_cmd_len = 4 + 3 + 1 + UINT32_T_STRLEN;
    char *cmd = NULL;

    if ( ( cmd = (char *)malloc( sizeof(char) * kick_cmd_len ) ) == NULL )
        return NULL;

    *cmd_len = sprintf(cmd, "kick %u\r\n", bound );

    return cmd;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  bsc_gen_quit_cmd
 *  Description:  
 * =====================================================================================
 */
inline char *bsc_gen_quit_cmd( int *cmd_len )
{
    static const char quit_cmd[] = "quit\r\n";
    static const int  quit_cmd_len = sizeof(quit_cmd) / sizeof(char) - 1;
    char *cmd = NULL;

    if ( ( cmd = strdup(quit_cmd) ) == NULL )
        return NULL;

    *cmd_len = quit_cmd_len;
    return cmd;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  bsc_gen_pause_tube_cmd
 *  Description:  
 * =====================================================================================
 */
inline char *bsc_gen_pause_tube_cmd( int *cmd_len, const char *tube_name, uint32_t delay )
{
    char *cmd = NULL;
    /* pause-tube \r\n\0 */
    static const int pause_tube_cmd_len = 10 + 3 + 1 + UINT32_T_STRLEN;

    if ( ( cmd = (char *)malloc( sizeof(char) * (pause_tube_cmd_len+strlen(tube_name))) ) == NULL )
        return NULL;

    *cmd_len = sprintf(cmd, "pause-tube %s %u\r\n", tube_name, delay );

    return cmd;
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
                                        uint32_t *id,
                                        uint32_t *bytes )
{
    static const bsc_response_t const bsc_peek_cmd_responses[] = {
        BSC_PEEK_RES_FOUND,
        BSC_RES_NOT_FOUND
    };

    bsc_response_t response_t;
    char *p = NULL;

    bsc_get_response_t(response, bsc_peek_cmd_responses);
    if ( response_t == BSC_PEEK_RES_FOUND ) {
        BSC_GET_ID_BYTES
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
    ssize_t matched = EOF;

    bsc_get_response_t(response, bsc_kick_cmd_responses);
    if ( response_t == BSC_KICK_RES_KICKED ) {
        p =  (char *)response + bsc_response_strlen[response_t] + 1;
        matched = sscanf(p, "%u", count );

        // got bad response format
        if (matched == EOF)
            return BSC_UNRECOGNIZED_RESPONSE;
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
inline bsc_response_t bsc_get_pause_tube_res( const char *response )
{
    static const bsc_response_t const bsc_pause_tube_cmd_responses[] = {
        BSC_PAUSE_TUBE_RES_PAUSED,
        BSC_RES_NOT_FOUND
    };

    bsc_response_t response_t;
    bsc_get_response_t(response, bsc_pause_tube_cmd_responses);
    return response_t;
}

#define get_int_from_yaml(var)\
    var = strtoul(p, &p_tmp, 10 );\
    if ( ( p = p_tmp + 3 + key_len[curr_key++] ) != NULL )\
        p_tmp = NULL;\
    else\
        goto parse_error;

#define get_dbl_from_yaml(var)\
    var = strtod(p, &p_tmp);\
    if ( ( p = p_tmp + 3 + key_len[curr_key++] ) != NULL )\
        p_tmp = NULL;\
    else\
        goto parse_error;

#define get_str_from_yaml(var)\
    p_tmp = strchr(p, '\n');\
    if (p_tmp == NULL)\
        goto parse_error;\
    len = p_tmp-p;\
    if ( ( var = strndup(p, len) ) == NULL )\
        goto parse_error;\
    var[len] = '\0';\
    p = p_tmp + 3 + key_len[curr_key++];\
    p_tmp = NULL;

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  bsc_gen_stats_job_cmd
 *  Description:  
 * =====================================================================================
 */
inline char *bsc_gen_stats_job_cmd( int *cmd_len, uint32_t id )
{
    /* stats-job <id>\r\n\0 */
    static const int stats_job_cmd_len = 9 + 3 + 1 + UINT32_T_STRLEN;
    char *cmd = NULL;

    if ( ( cmd = (char *)malloc( sizeof(char) * stats_job_cmd_len ) ) == NULL )
        return NULL;

    *cmd_len = sprintf(cmd, "stats-job %u\r\n", id );

    return cmd;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  bsc_get_stats_job_res
 *  Description:  
 *      Returns:  the response code (bsc_respone_t)
 * =====================================================================================
 */
inline bsc_response_t bsc_get_stats_job_res( const char *response, uint32_t *bytes )
{
    static const bsc_response_t const bsc_stats_job_cmd_responses[] = {
        BSC_RES_OK,
        BSC_RES_NOT_FOUND
    };

    bsc_response_t response_t;
    char *p = NULL;

    bsc_get_response_t(response, bsc_stats_job_cmd_responses);

    if (response_t == BSC_RES_OK) {
        *bytes = strtoul(response + bsc_response_strlen[response_t] + 1, &p, 10);
        if ( p == NULL)
            response_t = BSC_UNRECOGNIZED_RESPONSE;
    }

    return response_t;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  bsc_parse_job_stats
 *  Description:  
 * =====================================================================================
 */
bsc_job_stats *bsc_parse_job_stats( const char *data )
{
    static const size_t key_len[] = {
        2, 4, 5, 3, 3, 5, 3, 9, 8, 8, 8, 6, 5
    };

    static const char *job_state_str[] = {
        "ready",
        "buried",
        "reserved",
        "delayed"
    };

    bsc_job_stats *job;
    char *p = NULL, *p_tmp = NULL;
    int curr_key = 0, i;
    size_t len;

    if ( ( job = (bsc_job_stats *)malloc( sizeof(bsc_job_stats) ) ) == NULL )
        return NULL;

    char *state = NULL;
    job->tube = NULL;

    p = (char *)data + 5 + key_len[curr_key++];

    get_int_from_yaml(job->id);
    get_str_from_yaml(job->tube);
    get_str_from_yaml(state);
    get_int_from_yaml(job->pri);
    get_int_from_yaml(job->age);
    get_int_from_yaml(job->delay);
    get_int_from_yaml(job->ttr);
    get_int_from_yaml(job->time_left);
    get_int_from_yaml(job->reserves);
    get_int_from_yaml(job->timeouts);
    get_int_from_yaml(job->releases);
    get_int_from_yaml(job->buries);
    get_int_from_yaml(job->kicks);

    job->state = BSC_JOB_STATE_UNKNOWN;
    for ( i = 0; i < sizeof(job_state_str) / sizeof(char *); ++i )
        if ( ( strcmp(state, job_state_str[i]) ) == 0 ) {
            job->state = i;
            break;
        }

    return job;

parse_error:
    if (state != NULL)
        free(state);
    if (job->tube != NULL)
        free(job->tube);
    free(job);
    return NULL;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  bsc_job_stats_free
 *  Description:  
 * =====================================================================================
 */
void bsc_job_stats_free( bsc_job_stats *job )
{
    free(job->tube);
    free(job);
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  bsc_gen_stats_tube_cmd
 *  Description:  
 * =====================================================================================
 */
inline char *bsc_gen_stats_tube_cmd( int *cmd_len, const char *tube_name )
{
    char *cmd = NULL;
    /* stats-tube \r\n\0 */
    static const int stats_tube_cmd_len = 10 + 3 + 1;

    if ( ( cmd = (char *)malloc( sizeof(char) * (stats_tube_cmd_len+strlen(tube_name))) ) == NULL )
        return NULL;

    *cmd_len = sprintf(cmd, "stats-tube %s\r\n", tube_name );

    return cmd;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  bsc_get_stats_tube_res
 *  Description:  
 *      Returns:  the response code (bsc_respone_t)
 * =====================================================================================
 */
inline bsc_response_t bsc_get_stats_tube_res( const char *response, uint32_t *bytes )
{
    static const bsc_response_t const bsc_stats_tube_cmd_responses[] = {
        BSC_RES_OK,
        BSC_RES_NOT_FOUND
    };

    bsc_response_t response_t;
    char *p = NULL;

    bsc_get_response_t(response, bsc_stats_job_cmd_responses);

    if (response_t == BSC_RES_OK) {
        *bytes = strtoul(response + bsc_response_strlen[response_t] + 1, &p, 10);
        if ( p == NULL)
            response_t = BSC_UNRECOGNIZED_RESPONSE;
    }

    return response_t;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  bsc_parse_tube_stats
 *  Description:  
 * =====================================================================================
 */
bsc_tube_stats *bsc_parse_tube_stats( const char *data )
{
    static const size_t key_len[] = {
        4, 19, 18, 21, 20, 19, 10, 13, 16, 15, 14, 5, 15 
    };

    bsc_tube_stats *tube;
    char *p = NULL, *p_tmp = NULL;
    int curr_key = 0;
    size_t len;

    if ( ( tube = (bsc_tube_stats *)malloc( sizeof(bsc_tube_stats) ) ) == NULL )
        return NULL;

    tube->name = NULL;

    p = (char *)data + 6 + key_len[curr_key++];

    get_str_from_yaml(tube->name);
    get_int_from_yaml(tube->current_jobs_urgent);
    get_int_from_yaml(tube->current_jobs_ready);
    get_int_from_yaml(tube->current_jobs_reserved);
    get_int_from_yaml(tube->current_jobs_delayed);
    get_int_from_yaml(tube->current_jobs_buried);
    get_int_from_yaml(tube->total_jobs);
    get_int_from_yaml(tube->current_using);
    get_int_from_yaml(tube->current_watching);
    get_int_from_yaml(tube->current_waiting);
    get_int_from_yaml(tube->cmd_pause_tube);
    get_int_from_yaml(tube->pause);
    get_int_from_yaml(tube->pause_time_left);

    return tube;

parse_error:
    if (tube->name != NULL)
        free(tube->name);
    free(tube);
    return NULL;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  bsc_tube_stats_free
 *  Description:  
 * =====================================================================================
 */
void bsc_tube_stats_free( bsc_tube_stats *tube )
{
    free(tube->name);
    free(tube);
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  bsc_gen_stats_cmd
 *  Description:  
 * =====================================================================================
 */
inline char *bsc_gen_stats_cmd( int *cmd_len )
{
    static const char stats_cmd[] = "stats\r\n";
    static const int  stats_cmd_len = sizeof(stats_cmd) / sizeof(char) - 1;
    char *cmd = NULL;

    if ( ( cmd = strdup(stats_cmd) ) == NULL )
        return NULL;

    *cmd_len = stats_cmd_len;
    return cmd;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  bsc_get_stats_res
 *  Description:  
 *      Returns:  the response code (bsc_respone_t)
 * =====================================================================================
 */
inline bsc_response_t bsc_get_stats_res( const char *response, uint32_t *bytes )
{
    static const bsc_response_t const bsc_stats_cmd_responses[] = {
        BSC_RES_OK
    };

    bsc_response_t response_t;
    char *p = NULL;

    bsc_get_response_t(response, bsc_stats_job_cmd_responses);

    if (response_t == BSC_RES_OK) {
        *bytes = strtoul(response + bsc_response_strlen[response_t] + 1, &p, 10);
        if ( p == NULL)
            response_t = BSC_UNRECOGNIZED_RESPONSE;
    }

    return response_t;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  bsc_parse_server_stats
 *  Description:  
 * =====================================================================================
 */
bsc_server_stats *bsc_parse_server_stats( const char *data )
{
    static const size_t key_len[] = {
        19, 18, 21, 20, 19, 7, 8, 14, 16, 15, 11, 24, 10, 11, 7, 9, 10, 8, 8, 9, 9, 13, 14,
        14, 18, 22, 14, 12, 10, 12, 13, 19, 17, 15, 15, 17, 3, 7, 12, 12, 6, 19, 20, 15, 
    };

    bsc_server_stats *server;
    char *p = NULL, *p_tmp = NULL;
    int curr_key = 0;
    size_t len;

    if ( ( server = (bsc_server_stats *)malloc( sizeof(bsc_server_stats) ) ) == NULL )
        return NULL;

    p = (char *)data + 5 + key_len[curr_key++];

    get_int_from_yaml(server->current_jobs_urgent);
    get_int_from_yaml(server->current_jobs_ready);
    get_int_from_yaml(server->current_jobs_reserved);
    get_int_from_yaml(server->current_jobs_delayed);
    get_int_from_yaml(server->current_jobs_buried);
    get_int_from_yaml(server->cmd_put);
    get_int_from_yaml(server->cmd_peek);
    get_int_from_yaml(server->cmd_peek_ready);
    get_int_from_yaml(server->cmd_peek_delayed);
    get_int_from_yaml(server->cmd_peek_buried);
    get_int_from_yaml(server->cmd_reserve);
    get_int_from_yaml(server->cmd_reserve_with_timeout);
    get_int_from_yaml(server->cmd_delete);
    get_int_from_yaml(server->cmd_release);
    get_int_from_yaml(server->cmd_use);
    get_int_from_yaml(server->cmd_watch);
    get_int_from_yaml(server->cmd_ignore);
    get_int_from_yaml(server->cmd_bury);
    get_int_from_yaml(server->cmd_kick);
    get_int_from_yaml(server->cmd_touch);
    get_int_from_yaml(server->cmd_stats);
    get_int_from_yaml(server->cmd_stats_job);
    get_int_from_yaml(server->cmd_stats_tube);
    get_int_from_yaml(server->cmd_list_tubes);
    get_int_from_yaml(server->cmd_list_tube_used);
    get_int_from_yaml(server->cmd_list_tubes_watched);
    get_int_from_yaml(server->cmd_pause_tube);
    get_int_from_yaml(server->job_timeouts);
    get_int_from_yaml(server->total_jobs);
    get_int_from_yaml(server->max_job_size);
    get_int_from_yaml(server->current_tubes);
    get_int_from_yaml(server->current_connections);
    get_int_from_yaml(server->current_producers);
    get_int_from_yaml(server->current_workers);
    get_int_from_yaml(server->current_waiting);
    get_int_from_yaml(server->total_connections);
    get_int_from_yaml(server->pid);
    get_str_from_yaml(server->version);
    get_dbl_from_yaml(server->rusage_utime);
    get_dbl_from_yaml(server->rusage_stime);
    get_int_from_yaml(server->uptime);
    get_int_from_yaml(server->binlog_oldest_index);
    get_int_from_yaml(server->binlog_current_index);
    get_int_from_yaml(server->binlog_max_size);

    return server;

parse_error:
    free(server);
    return NULL;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  bsc_server_stats_free
 *  Description:  
 * =====================================================================================
 */
void bsc_server_stats_free( bsc_server_stats *server )
{
    free(server);
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  bsc_gen_list_tubes_cmd
 *  Description:  
 * =====================================================================================
 */
inline char *bsc_gen_list_tubes_cmd( int *cmd_len )
{
    static const char list_tubes_cmd[] = "list-tubes\r\n";
    static const int  list_tubes_cmd_len = sizeof(list_tubes_cmd) / sizeof(char) - 1;
    char *cmd = NULL;

    if ( ( cmd = strdup(list_tubes_cmd) ) == NULL )
        return NULL;

    *cmd_len = list_tubes_cmd_len;
    return cmd;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  bsc_gen_list_tubes_watched_cmd
 *  Description:  
 * =====================================================================================
 */
inline char *bsc_gen_list_tubes_watched_cmd( int *cmd_len )
{
    static const char list_tubes_watched_cmd[] = "list-tubes-watched\r\n";
    static const int  list_tubes_watched_cmd_len = sizeof(list_tubes_watched_cmd) / sizeof(char) - 1;
    char *cmd = NULL;

    if ( ( cmd = strdup(list_tubes_watched_cmd) ) == NULL )
        return NULL;

    *cmd_len = list_tubes_watched_cmd_len;
    return cmd;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  bsc_get_list_tubes_res
 *  Description:  
 *      Returns:  the response code (bsc_respone_t)
 * =====================================================================================
 */
inline bsc_response_t bsc_get_list_tubes_res( const char *response, uint32_t *bytes )
{
    static const bsc_response_t const bsc_list_tubes_cmd_responses[] = {
        BSC_RES_OK
    };

    bsc_response_t response_t;
    char *p = NULL;

    bsc_get_response_t(response, bsc_stats_job_cmd_responses);

    if (response_t == BSC_RES_OK) {
        *bytes = strtoul(response + bsc_response_strlen[response_t] + 1, &p, 10);
        if ( p == NULL)
            response_t = BSC_UNRECOGNIZED_RESPONSE;
    }

    return response_t;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  bsc_parse_tube_list
 *  Description:  
 *      Returns:  char ** with the last element being "\0"
 * =====================================================================================
 */
char **bsc_parse_tube_list( const char *data )
{
    char     **tube_list, *tubes, *tubes_p, *p1 = NULL, *p2 = NULL;
    register size_t num_tubes, i, tube_strlen, total_tubes_strlen = 0;

    // skip the yaml "---\n" header
    p1 = (char *)data + 4;

    for ( num_tubes = 0; ( p2 = strchr(p1, '\n') ) != NULL; ++num_tubes ) {
        total_tubes_strlen += p2 - p1 - 1; // w/o(-)
        p1 = p2 + 1; // skip \n
    }

    num_tubes--;

    // skip the yaml "---\n" header and the first list "- "
    p1 = (char *)data + 6;

    if ( ( tube_list = (char **)malloc( sizeof(char*) * (num_tubes +1) ) ) == NULL )
        return NULL;

    if ( ( tubes = (char *)malloc( sizeof(char) * (total_tubes_strlen+1) ) ) == NULL ){
        free(tube_list);
        return NULL;
    }

    for ( i = 0; i != num_tubes; ++i ) {
        p2 = strchr(p1, '\n');
        tube_strlen = p2 - p1;
        tube_list[i] = tubes;
        memcpy(tube_list[i], p1, tube_strlen);
        tube_list[i][tube_strlen] = '\0';
        p1 = p2 + 3; // skip "\n- "
        tubes += tube_strlen + 1;
    }

    // add the final "\0" element to the list
    tube_list[i] = tubes;
    tube_list[i][0] = '\0';

    return tube_list;
}

/*
 * template for creating strlen arrays
    static const char const *keys[] = {
        "name",
        "current-jobs-urgent",
        "current-jobs-ready",
        "current-jobs-reserved",
        "current-jobs-delayed",
        "current-jobs-buried",
        "total-jobs",
        "current-using",
        "current-watching",
        "current-waiting",
        "cmd-pause-tube",
        "pause",
        "pause-time-left"
    };

    int i;
    printf("static const size_t key_len[] = {\n    ");
    for ( i = 0; i < sizeof(keys)/sizeof(char *); ++i )
        printf("%d, ", strlen(keys[i]) );
    printf("\n};\n");
*/

