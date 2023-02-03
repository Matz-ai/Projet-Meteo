
#ifndef __MAIN_H__

#define __MAIN_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>

#include "common.h"
#include "pt/t1.h"
#include "pt/t2.h"
#include "pt/t3.h"
#include "wind.h"
#include "height.h"
#include "moisture.h"

void check_algo_exclu();

void check_op_exclu();

/**
 *  @fn int main (int argc, char const *argv[])
 *  @version 0.1
 *  @date Fri 03 Feb 2023 09:23
 *
 *  @brief Main function
 *
 *  @param[in] argc Number of arguments
 *  @param[in] *argv[] Arguments table
 *  @return EXIT_SUCCESS : The programm completd successfuly
 *  @return 1 : Error on arguments
 *  @return 2 : Error on input file
 *  @return 3 : Error on output file
 *  @return 4 : Other error
 */
int main(int argc, char *const *argv);

void execute();

#endif // __MAIN_H__
