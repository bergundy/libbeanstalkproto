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
#include <stdbool.h>

#define  CRLF ("\r\n")
#define  BSP_DEFAULT_PORT ("11300")
#define  BSP_PROTO_VALID_NAME_CHAR ("-+/;.$_()")
#define  BSP_PROTO_VALID_NAME_START_CHAR ("+/;.$_()")
#define  BSP_MAX_TUBE_NAME (200)
#define  MACRO2STR(m) #m
#define  UINT32_STRL ( (sizeof( MACRO2STR(UINT32_MAX))/sizeof(char)) - 1 )
#define  UINT64_STRL ( (sizeof( MACRO2STR(UINT64_MAX))/sizeof(char)) - 1 )

enum _bsp_response_t {
    BSP_RES_CLIENT_OUT_OF_MEMORY = -2, // client is out of memory
    BSP_RES_UNRECOGNIZED = -1,         // parse error

    /* general errors */
    BSP_RES_OUT_OF_MEMORY,          // server is out of memory
    BSP_RES_INTERNAL_ERROR,
    BSP_RES_BAD_FORMAT,
    BSP_RES_UNKNOWN_COMMAND,

    /* general responses */
    BSP_RES_OK,
    BSP_RES_BURIED,
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

/** 
* generates a put command header
* 
* @param hdr_len      pointer to store length of the generated hdr
* @param is_allocated pointer to store weather the generated string is to be freed
* @param priority     the job's priority
* @param delay        the job's start delay
* @param ttr          the job's time to run
* @param bytes        the job's data length
* 
* @return the serialized header
*/
char *bsp_gen_put_hdr(int       *hdr_len,
                      bool      *is_allocated,
                      uint32_t   priority,
                      uint32_t   delay,
                      uint32_t   ttr,
                      size_t     bytes);

/** 
* parses response from the put command
* 
* @param response the response message
* @param id       a pointer to store the received id (if available)
* 
* @return the response code
*/
bsp_response_t bsp_get_put_res(const char *response, uint64_t *id);

/** 
* generates a use command
* 
* @param cmd_len    pointer to store the generated command's length in
* @param is_allocated pointer to store weather the generated string is to be freed
* @param tube_name  the tube name..
* 
* @return the serialized command
*/
char *bsp_gen_use_cmd(int *cmd_len, bool *is_allocated, const char *tube_name);

/** 
* parses response from the use command
* 
* @param response  the response message
* @param tube_name a pointer to store the received tube (if available)
* 
* @return the response code
*/
bsp_response_t bsp_get_use_res(const char *response, char **tube_name);

/*-----------------------------------------------------------------------------
 * consumer methods
 *-----------------------------------------------------------------------------*/

/** 
* generates a reserve command
* 
* @param cmd_len     pointer to store the generated command's length in
* @param is_allocated  pointer to store weather the generated string is to be freed
* 
* @return the serialized command
*/
char *bsp_gen_reserve_cmd(int *cmd_len, bool *is_allocated);

/** 
* generates a reserve with timeout command
* 
* @param cmd_len     pointer to store the generated command's length in
* @param is_allocated  pointer to store weather the generated string is to be freed
* @param timeout     timeout in seconds
* 
* @return the serialized command
*/
char *bsp_gen_reserve_with_to_cmd(int *cmd_len, bool *is_allocated, uint32_t timeout);

/** 
* parses a response to the reserve command
* 
* @param response the response message
* @param id       a pointer to store the received id (if available)
* @param bytes    a pointer to store the received amount of bytes (if available)
* 
* @return the response code
*/
bsp_response_t bsp_get_reserve_res(const char *response, uint64_t *id, size_t *bytes);

/** 
* generates a delete command
* 
* @param cmd_len     pointer to store the generated command's length in
* @param is_allocated  pointer to store weather the generated string is to be freed
* @param id          the to be deleted job's id
* 
* @return the serialized command
*/
char *bsp_gen_delete_cmd(int *cmd_len, bool *is_allocated, uint64_t id);

/** 
* parses a response to the delete command
* 
* @param response the response message
* 
* @return the response code
*/
bsp_response_t bsp_get_delete_res(const char *response);

/** 
* generates a release command
* 
* @param cmd_len     pointer to store the generated command's length in
* @param is_allocated  pointer to store weather the generated string is to be freed
* @param id          the to be released job's id
* @param priority
* @param delay
* 
* @return the serialized command
*/
char *bsp_gen_release_cmd(int *cmd_len, bool *is_allocated, uint64_t id, uint32_t priority, uint32_t delay);

/** 
* parses a response to the release command
* 
* @param response the response message
* 
* @return the response code
*/
bsp_response_t bsp_get_release_res(const char *response);

/** 
* generates a bury command
* 
* @param cmd_len     pointer to store the generated command's length in
* @param is_allocated  pointer to store weather the generated string is to be freed
* @param id          the to be buried job's id
* @param priority    a new priority to assign to the job
* 
* @return the serialized command
*/
char *bsp_gen_bury_cmd(int *cmd_len, bool *is_allocated, uint64_t id, uint32_t priority);

/** 
* parses a response to the bury command
* 
* @param response the response message
* 
* @return the response code
*/
bsp_response_t bsp_get_bury_res(const char *response);

/** 
* generates a touch command
* 
* @param cmd_len     pointer to store the generated command's length in
* @param is_allocated  pointer to store weather the generated string is to be freed
* @param id          the to be touched job's id
* 
* @return the serialized command
*/
char *bsp_gen_touch_cmd(int *cmd_len, bool *is_allocated, uint64_t id);

/** 
* parses a response to the touch command
* 
* @param response the response message
* 
* @return the response code
*/
bsp_response_t bsp_get_touch_res(const char *response);

/** 
* generates a watch command
* 
* @param cmd_len    pointer to store the generated command's length in
* @param is_allocated pointer to store weather the generated string is to be freed
* @param tube_name  the tube name
* 
* @return the serialized command
*/
char *bsp_gen_watch_cmd(int *cmd_len, bool *is_allocated, const char *tube_name);

/** 
* parses a response to the watch command
* 
* @param response the response message
* @param count    a pointer to store the tubes watched count
* 
* @return the response code
*/
bsp_response_t bsp_get_watch_res(const char *response, uint32_t *count);

/** 
* generates an ignore command
* 
* @param cmd_len    pointer to store the generated command's length in
* @param is_allocated pointer to store weather the generated string is to be freed
* @param tube_name  the tube name
* 
* @return the serialized command
*/
char *bsp_gen_ignore_cmd(int *cmd_len, bool *is_allocated, const char *tube_name);

/** 
* parses a response to the ignore command
* 
* @param response the response message
* @param count    a pointer to store the tubes watched count
* 
* @return the response code
*/
bsp_response_t bsp_get_ignore_res(const char *response, uint32_t *count);

/*-----------------------------------------------------------------------------
 * other methods
 *-----------------------------------------------------------------------------*/

/** 
* generates a peek command
* 
* @param cmd_len    pointer to store the generated command's length in
* @param is_allocated pointer to store weather the generated string is to be freed
* @param id         the id of the job to be peeked
* 
* @return the serialized command
*/
char *bsp_gen_peek_cmd(int *cmd_len, bool *is_allocated, uint64_t id);

/** 
* generates a peek-ready command
* 
* @param cmd_len    pointer to store the generated command's length in
* @param is_allocated pointer to store weather the generated string is to be freed
* 
* @return the serialized command
*/
char *bsp_gen_peek_ready_cmd(int *cmd_len, bool *is_allocated);

/** 
* generates a peek-delayed command
* 
* @param cmd_len    pointer to store the generated command's length in
* @param is_allocated pointer to store weather the generated string is to be freed
* 
* @return the serialized command
*/
char *bsp_gen_peek_delayed_cmd(int *cmd_len, bool *is_allocated);

/** 
* generates a peek-buried command
* 
* @param cmd_len    pointer to store the generated command's length in
* @param is_allocated pointer to store weather the generated string is to be freed
* 
* @return the serialized command
*/
char *bsp_gen_peek_buried_cmd(int *cmd_len, bool *is_allocated);

/** 
* parses a response to the peek command
* 
* @param response the response message
* @param id       a pointer to store the received id (if available)
* @param bytes    a pointer to store the received amount of bytes (if available)
* 
* @return the response code
*/
bsp_response_t bsp_get_peek_res(const char *response, uint64_t *id, size_t *bytes);

/** 
* generates a kick command
* 
* @param cmd_len    pointer to store the generated command's length in
* @param is_allocated pointer to store weather the generated string is to be freed
* @param bound      maximum jobs to kick
* 
* @return the serialized command
*/
char *bsp_gen_kick_cmd(int *cmd_len, bool *is_allocated, uint32_t bound);

/** 
* parses a response to the kick command
* 
* @param response the response message
* @param id       a pointer to store the received id (if available)
* @param bytes    a pointer to store the kicked count (if available)
* 
* @return the response code
*/
bsp_response_t bsp_get_kick_res(const char *response, uint32_t *count);

/** 
* generates a quit command
* 
* @param cmd_len    pointer to store the generated command's length in
* @param is_allocated pointer to store weather the generated string is to be freed
* 
* @return the serialized command
*/
char *bsp_gen_quit_cmd(int *cmd_len, bool *is_allocated);

/** 
* generates a pause-tube command
* 
* @param cmd_len    pointer to store the generated command's length in
* @param is_allocated pointer to store weather the generated string is to be freed
* @param tube_name  the tube's name
* @param delay      in seconds
* 
* @return the serialized command
*/
char *bsp_gen_pause_tube_cmd(int *cmd_len, bool *is_allocated, const char *tube_name, uint32_t delay);

/** 
* parses a response to the pause-tube command
* 
* @param response the response message
* @param id       a pointer to store the received id (if available)
* 
* @return the response code
*/
bsp_response_t bsp_get_pause_tube_res(const char *response);

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
    uint64_t id;
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

/** 
* generates a stats-job command
* 
* @param cmd_len    pointer to store the generated command's length in
* @param is_allocated pointer to store weather the generated string is to be freed
* @param id         the id of the job to be queried
* 
* @return the serialized command
*/
char *bsp_gen_stats_job_cmd(size_t *cmd_len, bool *is_allocated, uint64_t id);

/** 
* parses a response to the stats-job command
* 
* @param response the response message
* @param id       a pointer to store the received id (if available)
* @param bytes    a pointer to store the received amount of bytes (if available)
* 
* @return the response code
*/
bsp_response_t bsp_get_stats_job_res(const char *response, size_t *bytes);

/** 
* parses the job stats yaml
* 
* @param data the yaml
* 
* @return a job stats struct pointer
*/
bsp_job_stats *bsp_parse_job_stats(const char *data);


/** 
* frees all memory allocated by job
* 
* @param job the stats struct to free
*/
void bsp_job_stats_free(bsp_job_stats *job);

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

/** 
* generates a stats-tube command
* 
* @param cmd_len    pointer to store the generated command's length in
* @param is_allocated pointer to store weather the generated string is to be freed
* @param tube_name  the tube's name
* 
* @return the serialized command
*/
char *bsp_gen_stats_tube_cmd(int *cmd_len, bool *is_allocated, const char *tube_name);

/** 
* parses a response to the stats-tube command
* 
* @param response the response message
* @param bytes    a pointer to store the received amount of bytes (if available)
* 
* @return the response code
*/
bsp_response_t bsp_get_stats_tube_res(const char *response, size_t *bytes);

/** 
* parses the tube stats yaml
* 
* @param data the yaml
* 
* @return a tube stats struct pointer
*/
bsp_tube_stats *bsp_parse_tube_stats(const char *data);

/** 
* frees all memory allocated by tube
* 
* @param tube the stats struct to free
*/
void bsp_tube_stats_free(bsp_tube_stats *tube);

/*-----------------------------------------------------------------------------
 * server definitions
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

/** 
* generates a stats command
* 
* @param cmd_len    pointer to store the generated command's length in
* @param is_allocated pointer to store weather the generated string is to be freed
* 
* @return the serialized command
*/
char *bsp_gen_stats_cmd(int *cmd_len, bool *is_allocated);

/** 
* parses a response to the stats command
* 
* @param response the response message
* @param bytes    a pointer to store the received amount of bytes (if available)
* 
* @return the response code
*/
bsp_response_t bsp_get_stats_res(const char *response, size_t *bytes);

/** 
* parses the server stats yaml
* 
* @param data the yaml
* 
* @return a server stats struct pointer
*/
bsp_server_stats *bsp_parse_server_stats(const char *data);

/** 
* frees all memory allocated by server (stats)
* 
* @param stats the server stats struct to free
*/
void bsp_server_stats_free(bsp_server_stats *server);

/*-----------------------------------------------------------------------------
 * list tubes
 *-----------------------------------------------------------------------------*/

/** 
* generates a list tubes command
* 
* @param cmd_len    pointer to store the generated command's length in
* @param is_allocated pointer to store weather the generated string is to be freed
* 
* @return the serialized command
*/
char *bsp_gen_list_tubes_cmd(int *cmd_len, bool *is_allocated);

/** 
* generates a list tubes watched command
* 
* @param cmd_len    pointer to store the generated command's length in
* @param is_allocated pointer to store weather the generated string is to be freed
* 
* @return the serialized command
*/
char *bsp_gen_list_tubes_watched_cmd(int *cmd_len, bool *is_allocated);

/** 
* parses a response to the list-tubes command
* 
* @param response the response message
* @param bytes    a pointer to store the received amount of bytes (if available)
* 
* @return the response code
*/
bsp_response_t bsp_get_list_tubes_res(const char *response, size_t *bytes);

/** 
* parses the server stats yaml
* 
* @param data the yaml
* 
* @return a NULL terminated array of tubes (strings)
*/
char **bsp_parse_tube_list(const char *data);

#ifdef __cplusplus
}
#endif

#endif /* BEANSTALKPROTO_H */
