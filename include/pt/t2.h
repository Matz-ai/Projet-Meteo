
#ifndef __T2_H__

#define __T2_H__

#include <string.h>
#include <stdbool.h>

#include "common.h"

#define _XOPEN_SOURCE /* See feature_test_macros(7) */
#include <time.h>

typedef struct
{

    time_t date;

    float avg_temperature;
} t2line;

typedef struct
{

    t2line *lines;

    int size;
} t2lines;

void print2tlines(t2lines *tab);

t2line *t2_tab_finddate(t2lines tab, time_t time);

t2line t2_convertline(char *line);

void t2_tab_convert_line(t2lines *tab, char *line);

void t2_tab_convert(t2lines *tab, FILE *stream);

void t2_tab();

//*************************************//
// BST tree functions
//*************************************//

typedef struct t2bst
{

    t2line *data;

    struct t2bst *left;

    struct t2bst *right;
} t2bst;

void t2_bst_insert(t2bst *tree, t2line *line);

void t2_bst_convert(t2bst *tree, FILE *stream);

void t2_bst_inorder(t2bst *tree, FILE *stream);

void t2_bst_write(t2bst *tree);

void t2_bst();

void t2();

#endif // __T2_H__
