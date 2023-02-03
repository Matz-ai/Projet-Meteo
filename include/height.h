#ifndef __HEIGHT_H__

#define __HEIGHT_H__

#include <string.h>
#include <stdbool.h>

#include "common.h"

typedef struct
{

    int stationId;

    int height;

    coord coord;
} heightline;

typedef struct
{

    heightline *lines;

    int size;
} heightlines;

void prinheighttlines(heightlines *tab);

heightline *height_tab_findstation(heightlines tab, int stationId);

heightline height_convertline(char *line);

void height_tab_convert_line(heightlines *tab, char *line);

void height_tab_convert(heightlines *tab, FILE *stream);

void height_tab_sort(heightlines *tab);

void height_tab_write(heightlines *tab);

void height_tab();

//*************************************//
// BST tree functions
//*************************************//

typedef struct heightbst
{

    heightline *data;

    struct heightbst *left;

    struct heightbst *right;
} heightbst;

void height_bst_insert(heightbst *tree, heightline *line);

void height_bst_convert(heightbst *tree, FILE *stream);

void height_bst_inorder(heightbst *tree, FILE *stream);

void height_bst_write(heightbst *tree);

void height_bst();

void height();
#endif // __HEIGHT_H__
