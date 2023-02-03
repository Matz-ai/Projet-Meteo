#ifndef __T1_H__

#define __T1_H__

#include <string.h>
#include <stdbool.h>

#include "common.h"

typedef struct
{

    int stationId;

    float avg_temperature;

    float min_temperature;

    float max_temperature;
} t1line;

typedef struct
{

    t1line *lines;

    int size;
} t1lines;

void print1tlines(t1lines *tab);

t1line *t1_tab_findstation(t1lines tab, int stationId);

t1line t1_convertline(char *line);

void t1_tab_convert_line(t1lines *tab, char *line);

void t1_tab_convert(t1lines *tab, FILE *stream);

void t1_tab();

//*************************************//
// BST tree functions
//*************************************//

typedef struct t1bst
{

    t1line *data;

    struct t1bst *left;

    struct t1bst *right;
} t1bst;

void t1_bst_insert(t1bst *tree, t1line *line);

void t1_bst_convert(t1bst *tree, FILE *stream);

void t1_bst_inorder(t1bst *tree, FILE *stream);

void t1_bst_write(t1bst *tree);

void t1_bst();

void t1();

#endif // __T1_H__
