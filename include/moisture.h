#ifndef __MOISTURE_H__

#define __MOISTURE_H__

#include <string.h>
#include <stdbool.h>

#include "common.h"

typedef struct
{

    int stationId;

    int moisture;

    coord coord;
} moistureline;

typedef struct
{

    moistureline *lines;

    int size;
} moisturelines;

void prinmoisturetlines(moisturelines *tab);

moistureline *moisture_tab_findstation(moisturelines tab, int stationId);

moistureline moisture_convertline(char *line);

void moisture_tab_convert_line(moisturelines *tab, char *line);

void moisture_tab_convert(moisturelines *tab, FILE *stream);

void moisture_tab_sort(moisturelines *tab);

void moisture_tab_write(moisturelines *tab);

void moisture_tab();

//*************************************//
// BST tree functions
//*************************************//

typedef struct moisturebst
{

    moistureline *data;

    struct moisturebst *left;

    struct moisturebst *right;
} moisturebst;

void moisture_bst_insert(moisturebst *tree, moistureline *line);

void moisture_bst_convert(moisturebst *tree, FILE *stream);

void moisture_bst_inorder(moisturebst *tree, FILE *stream);

void moisture_bst_write(moisturebst *tree);

void moisture_bst();

void moisture();

#endif // __MOISTURE_H__
