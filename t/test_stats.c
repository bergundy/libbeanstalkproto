/*
 * =====================================================================================
 *
 *       Filename:  test_stats.c
 *
 *    Description:  test suite for libbeanstalk stats commands
 *
 *        Version:  1.0
 *        Created:  05/27/2010 02:09:19 PM
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
#include <stdint.h>
#include <string.h>
#include <tap.h>

#include "beanstalkclient.h"
#define  PATH_TO(file) "t/yaml_samples/" file

static const char *files[] = {
};

#define ALL_TESTS 3

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  main
 *  Description:  
 * =====================================================================================
 */
char *open_stats( const char *filename )
{
    FILE *pFile;
    long lSize;
    char *buffer;

    if ( ( pFile = fopen( filename, "r" ) ) == NULL )
        skip(1, "File error");

    else {
        // obtain file size:
        fseek (pFile , 0 , SEEK_END);
        lSize = ftell (pFile);
        rewind (pFile);

        // allocate memory to contain the whole file:
        if ( ( buffer = (char *) malloc(sizeof(char)*lSize) ) == NULL )
            skip(1, "Memory error");
        else {
            // copy the file into the buffer:
            if ( fread(buffer,1,lSize,pFile) != lSize )
                skip(1, "Read error");
            else {
                return buffer;
            }
            free(buffer);
        }
        fclose(pFile);
    }
    return NULL;
}

int main ( int argc, char *argv[] )
{
    plan_tests(ALL_TESTS);

    bsc_job_stats  *job;
    bsc_tube_stats *tube;
    bsc_server_stats *server;
    char *buffer;
    
//    "list-tubes-watched.yaml",
//    "list-tubes.yaml"

    if ( ( buffer = open_stats(PATH_TO("stats-job.yaml")) ) != NULL ) {
        ok( ( job = bsc_parse_job_stats(buffer) ) != NULL, "bsc_parse_job_stats" );
        bsc_job_stats_free(job);
        free(buffer);
    }
    if ( ( buffer = open_stats(PATH_TO("stats-tube.yaml")) ) != NULL ) {
        ok( ( tube = bsc_parse_tube_stats(buffer) ) != NULL, "bsc_parse_tube_stats" );
        bsc_tube_stats_free(tube);
        free(buffer);
    }
    if ( ( buffer = open_stats(PATH_TO("stats.yaml")) ) != NULL ) {
        ok( ( server = bsc_parse_server_stats(buffer) ) != NULL, "bsc_parse_server_stats" );
        bsc_server_stats_free(server);
        free(buffer);
    }

    return exit_status();
}
