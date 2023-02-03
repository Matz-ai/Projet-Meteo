#include "common.h"

meteo_config meteoconf = {
    .input_file_name = NULL,
    .output_file_name = NULL,
    .sort_algo = ALGO_NONE,
    .operation = OP_NONE,
    .reverse = false,
    .verbose = 0,
};

bool split_coord(coord *co, const char *coord)
{
    char *tmp = strdup(coord); // Dub coord for strtok

    // Split coord string into two substrings separated by a comma
    char *coordx = strtok(tmp, ",");
    char *coordy = strtok(NULL, ",");
    write_log(4, "coordx: %s \n", coordx);
    write_log(4, "coordy: %s \n", coordy);

    // Check if not null and convert coordx and coordy to int

    if (coordx != NULL && coordy != NULL)
    {
        co->x = atof(coordx);
        co->y = atof(coordy);
    }
    else
    {
        co->x = 0;
        co->y = 0;
        return false;
    }

    return true;
}

const char *getfield(char *line, int num)
{
    const char *tok;
    for (tok = strtok(line, ";");
         tok && *tok;
         tok = strtok(NULL, ";\n"))
    {
        if (!--num)
            return tok;
    }
    return NULL;
}
