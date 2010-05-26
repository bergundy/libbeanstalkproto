/*
 * =====================================================================================
 *
 *       Filename:  beanstalkclient.h
 *
 *    Description:  header file for c implementation of beanstalk client protocol
 *
 *        Version:  1.0
 *        Created:  05/23/2010 03:56:10 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Roey Berman, royb@walla.net.il
 *        Company:  Walla!
 *
 * =====================================================================================
 */
#ifndef BEANSTALKCLIENT_H
#define BEANSTALKCLIENT_H 

#include <stdint.h>
#include <sys/uio.h>

#define  BSC_DEFAULT_PORT "11300"
#define  BSC_PROTO_VALID_NAME_CHAR "-+/;.$_()"
#define  BSC_PROTO_VALID_NAME_START_CHAR "+/;.$_()"
#define  BSC_MAX_TUBE_NAME 200

#define  UINT32_T_STRLEN 10

//                            put SPACE 4*UINT32            CRLF \0
#define  BSC_PUT_CMD_HDR_SIZE 3 + 3   + 4*UINT32_T_STRLEN + 2  + 1

enum _bsc_response_t {
    BSC_UNRECOGNIZED_RESPONSE = -1,

    /* general errors */
    BSC_RES_OUT_OF_MEMORY,
    BSC_RES_INTERNAL_ERROR,
    BSC_RES_BAD_FORMAT,
    BSC_RES_UNKOWN_COMMAND,

    /* general responses */
    BSC_RES_BURIED, //what does the id in put response mean?
    BSC_RES_NOT_FOUND,
    BSC_RES_WATCHING,

    /* put cmd results */
    BSC_PUT_RES_INSERTED,
    BSC_PUT_RES_EXPECTED_CRLF,
    BSC_PUT_RES_JOB_TOO_BIG, 
    BSC_PUT_RES_DRAINING, 

    /* use cmd result */
    BSC_USE_RES_USING, 

    /* reserve cmd result */
    BSC_RESERVE_RES_RESERVED,
    BSC_RESERVE_RES_DEADLINE_SOON,
    BSC_RESERVE_RES_TIMED_OUT,

    /* delete cmd result */
    BSC_DELETE_RES_DELETED,

    /* release cmd result */
    BSC_RELEASE_RES_RELEASED,

    /* touch cmd result */
    BSC_TOUCH_RES_TOUCHED,

    /* ignore cmd result */
    BSC_IGNORE_RES_NOT_IGNORED,

    /* peek cmd result */
    BSC_PEEK_RES_FOUND,

    /* kick cmd result */
    BSC_KICK_RES_KICKED
};

typedef enum _bsc_response_t bsc_response_t;

/*-----------------------------------------------------------------------------
 * producer methods
 *-----------------------------------------------------------------------------*/
/* put */
inline struct iovec *bsc_gen_put_iov( uint32_t, uint32_t, uint32_t, size_t, char * );
inline char *bsc_gen_put_msg( int *, uint32_t, uint32_t, uint32_t , size_t, const char * );
inline char *bsc_gen_put_hdr( int *, uint32_t, uint32_t, uint32_t, size_t );
inline bsc_response_t bsc_get_put_res( const char *, uint32_t * );

/* use */
inline char *bsc_gen_use_msg( int *, const char * );
inline bsc_response_t bsc_get_use_res( const char *, char ** );

/*-----------------------------------------------------------------------------
 * consumer methods
 *-----------------------------------------------------------------------------*/
/* reserve */
inline char *bsc_gen_reserve_msg(int *);
inline char *bsc_gen_reserve_with_to_msg( int *, uint32_t );
inline bsc_response_t bsc_get_reserve_res( const char *, ssize_t, uint32_t *, uint32_t *, char **, int );

/* delete */
inline char *bsc_gen_delete_msg( int *, uint32_t );
inline bsc_response_t bsc_get_delete_res( const char * );

/* release */
inline char *bsc_gen_release_msg( int *, uint32_t, uint32_t, uint32_t );
inline bsc_response_t bsc_get_release_res( const char * );

/* bury */
inline char *bsc_gen_bury_msg( int *, uint32_t, uint32_t );
inline bsc_response_t bsc_get_bury_res( const char * );

/* touch */
inline char *bsc_gen_touch_msg( int *, uint32_t );
inline bsc_response_t bsc_get_touch_res( const char * );

/* watch */
inline char *bsc_gen_watch_msg( int *, const char * );
inline bsc_response_t bsc_get_watch_res( const char *, uint32_t * );

/* ignore */
inline char *bsc_gen_ignore_msg( int *, const char * );
inline bsc_response_t bsc_get_ignore_res( const char *, uint32_t * );

/*-----------------------------------------------------------------------------
 * other methods
 *-----------------------------------------------------------------------------*/

/* peek */
inline char *bsc_gen_peek_msg(int *, uint32_t);
inline char *bsc_gen_peek_ready_msg(int *);
inline char *bsc_gen_peek_delayed_msg(int *);
inline char *bsc_gen_peek_buried_msg(int *);
inline bsc_response_t bsc_get_peek_res( const char *, ssize_t, uint32_t *, uint32_t *, char **, int );

/* kick */
inline char *bsc_gen_kick_msg( int *, uint32_t );
inline bsc_response_t bsc_get_kick_res( const char *, uint32_t * );

#endif /* BEANSTALKCLIENT_H */
