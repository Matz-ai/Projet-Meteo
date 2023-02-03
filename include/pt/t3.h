
#ifndef __T3_H__

#define __T3_H__

#include <string.h>
#include <stdbool.h>

#include "common.h"
#define _XOPEN_SOURCE /* See feature_test_macros(7) */

#include <time.h>

typedef struct
{

    int stationId;

    float temperature;

} t3line;

typedef struct
{
    time_t date;

    t3line *lines;

    int size;
} t3l1lines;

typedef struct
{

    t3l1lines *tabs;

    int size;
} t3l2lines;

void printt3lines(t3l2lines *tab);

t3l1lines *t3_tab_finddate(t3l2lines tab, time_t time);

void t3_tab_convert_line(t3l2lines *tab, char *line);

void t3_tab_convert(t3l2lines *tab, FILE *stream);

void t3_tab();

#endif // __T3_H__
