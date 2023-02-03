#ifndef __COMMON_C__

#define __COMMON_C__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <getopt.h>
#include <string.h>

#define EXIT_ARG 1
#define EXIT_IN_FILE 2
#define EXIT_OUT_FILE 3
#define EXIT_ERROR 4

#define write_log(priority, format, args...) \
    if (priority <= meteoconf.verbose)       \
    {                                        \
        printf(format, ##args);              \
    }

typedef enum
{
    ALGO_NONE = -1,
    /*! Sort using tables */
    ALGO_TAB = 0,
    /*! Sort using an ABR tree */
    ALGO_ABR = 1,
    /*! Sort usin an AVL tree */
    ALGO_AVL = 2
} algo;

typedef struct
{

    float x;

    float y;
} coord;

typedef enum
{
    OP_NONE = -1,
    /*! Calculate min, max and average temp for each station, sort them by id */
    OP_T1 = 0,
    /*! Calculate average temp by date/hour, sort by date*/
    OP_T2 = 1,
    /*! temp by date/hour per station, sort by date then station*/
    OP_T3 = 2,
    /*! Calculate min, max and average pressure for each station, sort them by id */
    OP_P1 = 3,
    /*! Calculate average pressure by date/hour, sort by date*/
    OP_P2 = 4,
    /*! pressure by date/hour per station, sort by date then station*/
    OP_P3 = 5,
    /*! Calculate average wind orientation and speed by station, sort by station id */
    OP_W = 6,
    /*! Altitude by station, decreasing order */
    OP_H = 7,
    /*! Calculate max humidity by station, decreasing order */
    OP_M = 8,

} operation;

typedef struct
{
    /*! Input file name */
    char *input_file_name;
    /*! Output file name */
    char *output_file_name;
    /*! Sorting algorithm */
    algo sort_algo;
    /*! Operation to do */
    operation operation;
    /*! Reverse */
    bool reverse;
    /*! Verbose mode */
    int verbose;
} meteo_config;

extern meteo_config meteoconf;

bool split_coord(coord *co, const char *coord);

const char *getfield(char *line, int num);

#endif // __COMMON_C__
