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

#ifdef __cplusplus
	extern "C" {
#endif

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
    BSC_RES_UNKNOWN_COMMAND,

    /* general responses */
    BSC_RES_OK,
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
    BSC_KICK_RES_KICKED,

    /* pause-tube cmd result */
    BSC_PAUSE_TUBE_RES_PAUSED
};

typedef enum _bsc_response_t bsc_response_t;

/*-----------------------------------------------------------------------------
 * producer methods
 *-----------------------------------------------------------------------------*/
/* put */
struct iovec *bsc_gen_put_iov( uint32_t, uint32_t, uint32_t, size_t, char * );
char *bsc_gen_put_cmd( int *, uint32_t, uint32_t, uint32_t , size_t, const char * );
char *bsc_gen_put_hdr( int *, uint32_t, uint32_t, uint32_t, size_t );
bsc_response_t bsc_get_put_res( const char *, uint32_t * );

/* use */
char *bsc_gen_use_cmd( int *, const char * );
bsc_response_t bsc_get_use_res( const char *, char ** );

/*-----------------------------------------------------------------------------
 * consumer methods
 *-----------------------------------------------------------------------------*/
/* reserve */
char *bsc_gen_reserve_cmd(int *);
char *bsc_gen_reserve_with_to_cmd( int *, uint32_t );
bsc_response_t bsc_get_reserve_res( const char *, uint32_t *, uint32_t * );

/* delete */
char *bsc_gen_delete_cmd( int *, uint32_t );
bsc_response_t bsc_get_delete_res( const char * );

/* release */
char *bsc_gen_release_cmd( int *, uint32_t, uint32_t, uint32_t );
bsc_response_t bsc_get_release_res( const char * );

/* bury */
char *bsc_gen_bury_cmd( int *, uint32_t, uint32_t );
bsc_response_t bsc_get_bury_res( const char * );

/* touch */
char *bsc_gen_touch_cmd( int *, uint32_t );
bsc_response_t bsc_get_touch_res( const char * );

/* watch */
char *bsc_gen_watch_cmd( int *, const char * );
bsc_response_t bsc_get_watch_res( const char *, uint32_t * );

/* ignore */
char *bsc_gen_ignore_cmd( int *, const char * );
bsc_response_t bsc_get_ignore_res( const char *, uint32_t * );

/*-----------------------------------------------------------------------------
 * other methods
 *-----------------------------------------------------------------------------*/

/* peek */
char *bsc_gen_peek_cmd(int *, uint32_t);
char *bsc_gen_peek_ready_cmd(int *);
char *bsc_gen_peek_delayed_cmd(int *);
char *bsc_gen_peek_buried_cmd(int *);
bsc_response_t bsc_get_peek_res( const char *, uint32_t *, uint32_t * );

/* kick */
char *bsc_gen_kick_cmd( int *, uint32_t );
bsc_response_t bsc_get_kick_res( const char *, uint32_t * );

/* quit */
char *bsc_gen_quit_cmd(int *);

/* pause-tube */
char *bsc_gen_pause_tube_cmd( int *, const char *, uint32_t );
bsc_response_t bsc_get_pause_tube_res( const char * );

/*-----------------------------------------------------------------------------
 * job definitions
 *-----------------------------------------------------------------------------*/

enum _bsc_job_state {
    BSC_JOB_STATE_UNKNOWN = -1,
    BSC_JOB_STATE_READY,
    BSC_JOB_STATE_BURIED,
    BSC_JOB_STATE_RESERVED,
    BSC_JOB_STATE_DELAYED
};

typedef enum _bsc_job_state bsc_job_state;

struct _bsc_job_stats {
    uint32_t id;
    char     *tube;
    bsc_job_state state;
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

typedef struct _bsc_job_stats bsc_job_stats;

char *bsc_gen_stats_job_cmd(int *, uint32_t);
bsc_response_t bsc_get_stats_job_res( const char *, uint32_t * );
bsc_job_stats *bsc_parse_job_stats( const char * );
void bsc_job_stats_free( bsc_job_stats * );

/*-----------------------------------------------------------------------------
 * tube definitions
 *-----------------------------------------------------------------------------*/

struct _bsc_tube_stats {
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

typedef struct _bsc_tube_stats bsc_tube_stats;

char *bsc_gen_stats_tube_cmd(int *, const char *);
bsc_response_t bsc_get_stats_tube_res( const char *, uint32_t * );
bsc_tube_stats *bsc_parse_tube_stats( const char * );
void bsc_tube_stats_free( bsc_tube_stats * );

/*-----------------------------------------------------------------------------
 * tube definitions
 *-----------------------------------------------------------------------------*/

struct _bsc_server_stats {
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

typedef struct _bsc_server_stats bsc_server_stats;

char *bsc_gen_stats_cmd( int * );
bsc_response_t bsc_get_stats_res( const char *, uint32_t * );
bsc_server_stats *bsc_parse_server_stats( const char * );
void bsc_server_stats_free( bsc_server_stats * );

/* list tubes */
char *bsc_gen_list_tubes_cmd(int *);
char *bsc_gen_list_tubes_watched_cmd(int *);
bsc_response_t bsc_get_list_tubes_res( const char *, uint32_t * );
char **bsc_parse_tube_list( const char *data );

#ifdef __cplusplus
}
#endif

#endif /* BEANSTALKCLIENT_H */
