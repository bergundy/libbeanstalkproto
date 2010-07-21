/*
 * =====================================================================================
 *
 *       Filename:  beanstalkproto.h
 *
 *    Description:  header file for c implementation of beanstalk protocol
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
#ifndef BEANSTALKPROTO_H
#define BEANSTALKPROTO_H 

#ifdef __cplusplus
	extern "C" {
#endif

#include <stdint.h>
#include <sys/uio.h>

#define  BSP_DEFAULT_PORT "11300"
#define  BSP_PROTO_VALID_NAME_CHAR "-+/;.$_()"
#define  BSP_PROTO_VALID_NAME_START_CHAR "+/;.$_()"
#define  BSP_MAX_TUBE_NAME 200

#define  UINT32_T_STRLEN 10

//                            put SPACE 4*UINT32            CRLF \0
#define  BSP_PUT_CMD_HDR_SIZE 3 + 3   + 4*UINT32_T_STRLEN + 2  + 1

enum _bsp_response_t {
    BSP_UNRECOGNIZED_RESPONSE = -1,

    /* general errors */
    BSP_RES_OUT_OF_MEMORY,
    BSP_RES_INTERNAL_ERROR,
    BSP_RES_BAD_FORMAT,
    BSP_RES_UNKNOWN_COMMAND,

    /* general responses */
    BSP_RES_OK,
    BSP_RES_BURIED, //what does the id in put response mean?
    BSP_RES_NOT_FOUND,
    BSP_RES_WATCHING,

    /* put cmd results */
    BSP_PUT_RES_INSERTED,
    BSP_PUT_RES_EXPECTED_CRLF,
    BSP_PUT_RES_JOB_TOO_BIG, 
    BSP_PUT_RES_DRAINING, 

    /* use cmd result */
    BSP_USE_RES_USING, 

    /* reserve cmd result */
    BSP_RESERVE_RES_RESERVED,
    BSP_RESERVE_RES_DEADLINE_SOON,
    BSP_RESERVE_RES_TIMED_OUT,

    /* delete cmd result */
    BSP_DELETE_RES_DELETED,

    /* release cmd result */
    BSP_RELEASE_RES_RELEASED,

    /* touch cmd result */
    BSP_TOUCH_RES_TOUCHED,

    /* ignore cmd result */
    BSP_IGNORE_RES_NOT_IGNORED,

    /* peek cmd result */
    BSP_PEEK_RES_FOUND,

    /* kick cmd result */
    BSP_KICK_RES_KICKED,

    /* pause-tube cmd result */
    BSP_PAUSE_TUBE_RES_PAUSED
};

typedef enum _bsp_response_t bsp_response_t;

/*-----------------------------------------------------------------------------
 * producer methods
 *-----------------------------------------------------------------------------*/
/* put */
struct iovec *bsp_gen_put_iov( uint32_t, uint32_t, uint32_t, size_t, char * );
char *bsp_gen_put_cmd( int *, uint32_t, uint32_t, uint32_t , size_t, const char * );
char *bsp_gen_put_hdr( int *, uint32_t, uint32_t, uint32_t, size_t );
bsp_response_t bsp_get_put_res( const char *, uint32_t * );

/* use */
char *bsp_gen_use_cmd( int *, const char * );
bsp_response_t bsp_get_use_res( const char *, char ** );

/*-----------------------------------------------------------------------------
 * consumer methods
 *-----------------------------------------------------------------------------*/
/* reserve */
char *bsp_gen_reserve_cmd(int *);
char *bsp_gen_reserve_with_to_cmd( int *, uint32_t );
bsp_response_t bsp_get_reserve_res( const char *, uint32_t *, uint32_t * );

/* delete */
char *bsp_gen_delete_cmd( int *, uint32_t );
bsp_response_t bsp_get_delete_res( const char * );

/* release */
char *bsp_gen_release_cmd( int *, uint32_t, uint32_t, uint32_t );
bsp_response_t bsp_get_release_res( const char * );

/* bury */
char *bsp_gen_bury_cmd( int *, uint32_t, uint32_t );
bsp_response_t bsp_get_bury_res( const char * );

/* touch */
char *bsp_gen_touch_cmd( int *, uint32_t );
bsp_response_t bsp_get_touch_res( const char * );

/* watch */
char *bsp_gen_watch_cmd( int *, const char * );
bsp_response_t bsp_get_watch_res( const char *, uint32_t * );

/* ignore */
char *bsp_gen_ignore_cmd( int *, const char * );
bsp_response_t bsp_get_ignore_res( const char *, uint32_t * );

/*-----------------------------------------------------------------------------
 * other methods
 *-----------------------------------------------------------------------------*/

/* peek */
char *bsp_gen_peek_cmd(int *, uint32_t);
char *bsp_gen_peek_ready_cmd(int *);
char *bsp_gen_peek_delayed_cmd(int *);
char *bsp_gen_peek_buried_cmd(int *);
bsp_response_t bsp_get_peek_res( const char *, uint32_t *, uint32_t * );

/* kick */
char *bsp_gen_kick_cmd( int *, uint32_t );
bsp_response_t bsp_get_kick_res( const char *, uint32_t * );

/* quit */
char *bsp_gen_quit_cmd(int *);

/* pause-tube */
char *bsp_gen_pause_tube_cmd( int *, const char *, uint32_t );
bsp_response_t bsp_get_pause_tube_res( const char * );

/*-----------------------------------------------------------------------------
 * job definitions
 *-----------------------------------------------------------------------------*/

enum _bsp_job_state {
    BSP_JOB_STATE_UNKNOWN = -1,
    BSP_JOB_STATE_READY,
    BSP_JOB_STATE_BURIED,
    BSP_JOB_STATE_RESERVED,
    BSP_JOB_STATE_DELAYED
};

typedef enum _bsp_job_state bsp_job_state;

struct _bsp_job_stats {
    uint32_t id;
    char     *tube;
    bsp_job_state state;
    uint32_t pri;
    uint32_t age;
    uint32_t delay;
    uint32_t ttr;
    uint32_t time_left;
    uint16_t reserves;
    uint16_t timeouts;
    uint16_t releases;
    uint16_t buries;
    uint16_t kicks;
};

typedef struct _bsp_job_stats bsp_job_stats;

char *bsp_gen_stats_job_cmd(int *, uint32_t);
bsp_response_t bsp_get_stats_job_res( const char *, uint32_t * );
bsp_job_stats *bsp_parse_job_stats( const char * );
void bsp_job_stats_free( bsp_job_stats * );

/*-----------------------------------------------------------------------------
 * tube definitions
 *-----------------------------------------------------------------------------*/

struct _bsp_tube_stats {
    char *name;
    uint32_t current_jobs_urgent;
    uint32_t current_jobs_ready;
    uint32_t current_jobs_reserved;
    uint32_t current_jobs_delayed;
    uint32_t current_jobs_buried;
    uint32_t total_jobs;
    uint32_t current_using;
    uint32_t current_watching;
    uint32_t current_waiting;
    uint32_t cmd_pause_tube;
    uint32_t pause;
    uint32_t pause_time_left;
};

typedef struct _bsp_tube_stats bsp_tube_stats;

char *bsp_gen_stats_tube_cmd(int *, const char *);
bsp_response_t bsp_get_stats_tube_res( const char *, uint32_t * );
bsp_tube_stats *bsp_parse_tube_stats( const char * );
void bsp_tube_stats_free( bsp_tube_stats * );

/*-----------------------------------------------------------------------------
 * tube definitions
 *-----------------------------------------------------------------------------*/

struct _bsp_server_stats {
    uint32_t current_jobs_urgent;
    uint32_t current_jobs_ready;
    uint32_t current_jobs_reserved;
    uint32_t current_jobs_delayed;
    uint32_t current_jobs_buried;
    uint32_t cmd_put;
    uint32_t cmd_peek;
    uint32_t cmd_peek_ready;
    uint32_t cmd_peek_delayed;
    uint32_t cmd_peek_buried;
    uint32_t cmd_reserve;
    uint32_t cmd_reserve_with_timeout;
    uint32_t cmd_delete;
    uint32_t cmd_release;
    uint32_t cmd_use;
    uint32_t cmd_watch;
    uint32_t cmd_ignore;
    uint32_t cmd_bury;
    uint32_t cmd_kick;
    uint32_t cmd_touch;
    uint32_t cmd_stats;
    uint32_t cmd_stats_job;
    uint32_t cmd_stats_tube;
    uint32_t cmd_list_tubes;
    uint32_t cmd_list_tube_used;
    uint32_t cmd_list_tubes_watched;
    uint32_t cmd_pause_tube;
    uint32_t job_timeouts;
    uint32_t total_jobs;
    uint32_t max_job_size;
    uint32_t current_tubes;
    uint32_t current_connections;
    uint32_t current_producers;
    uint32_t current_workers;
    uint32_t current_waiting;
    uint32_t total_connections;
    uint32_t pid;
    char     *version;
    double   rusage_utime;
    double   rusage_stime;
    uint32_t uptime;
    uint32_t binlog_oldest_index;
    uint32_t binlog_current_index;
    uint32_t binlog_max_size;
};

typedef struct _bsp_server_stats bsp_server_stats;

char *bsp_gen_stats_cmd( int * );
bsp_response_t bsp_get_stats_res( const char *, uint32_t * );
bsp_server_stats *bsp_parse_server_stats( const char * );
void bsp_server_stats_free( bsp_server_stats * );

/* list tubes */
char *bsp_gen_list_tubes_cmd(int *);
char *bsp_gen_list_tubes_watched_cmd(int *);
bsp_response_t bsp_get_list_tubes_res( const char *, uint32_t * );
char **bsp_parse_tube_list( const char *data );

#ifdef __cplusplus
}
#endif

#endif /* BEANSTALKPROTO_H */
