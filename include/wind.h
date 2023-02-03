#ifndef __WIND_H__

#define __WIND_H__

#include <string.h>
#include <stdbool.h>
#include <math.h>

#include "common.h"

typedef struct
{

    int stationId;

    float orientation;

    float speed;

    coord coord;

} windline;

typedef struct
{

    windline *lines;

    int size;
} windlines;

void prinwindtlines(windlines *tab);

windline *wind_findstation(windlines tab, int stationId);

windline wind_convertline(char *line);

void wind_convert_line(windlines *tab, char *line);

void wind_convert(windlines *tab, FILE *stream);

void wind_sort(windlines *tab);

void wind_write(windlines *tab);

//*************************************//
// BST tree functions
//*************************************//

typedef struct windtbst
{

    windline *data;

    struct windtbst *left;

    struct windtbst *right;
} windtbst;

void wind_bst_insert(windtbst *tree, windline *line);

void wind_bst_convert(windtbst *tree, FILE *stream);

void wind_bst_inorder(windtbst *tree, FILE *stream);

void wind_bst_write(windtbst *tree);

void wind_bst();

void wind();

#endif // __WIND_H__
