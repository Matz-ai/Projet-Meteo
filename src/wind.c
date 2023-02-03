#include "wind.h"

void prinwindtlines(windlines *tab)
{
    printf("Printing table of size %d\n", tab->size);
    printf("StationId Orientation Speed Coord \n");

    for (int i = 0; i < tab->size; i++)
    {
        printf("%d %f %f %f \n", tab->lines[i].stationId, tab->lines[i].orientation, tab->lines[i].speed, tab->lines[i].coord.x);
    }
}

windline *wind_tab_findstation(windlines tab, int stationId)
{
    windline *retour = NULL; // Variable de retour

    // Find station in table
    for (int i = 0; i < tab.size; i++)
    {
        if (tab.lines[i].stationId == stationId)
        {
            retour = &tab.lines[i];
            break;
        }
    }

    return (retour);
}

// Used to indicate that an error occured during the conversion
bool wind_error_convert_line = false;

windline wind_convertline(char *line)
{
    // Reset error flag
    wind_error_convert_line = false;
    // init return variable
    windline retour = {0, 0, 0, {0, 0}};
    write_log(4, "Converting line: %s \n", line);

    // Dub line for strtok
    char *tmp = strdup(line);
    write_log(4, "tmp: %s \n", tmp);
    // Get station id
    const char *id = getfield(tmp, 1);
    write_log(4, "Station id: %s \n", id);
    // Convert station id to int
    retour.stationId = atoi(id);
    // Free tmp
    free(tmp);

    // Dub line for strtok
    tmp = strdup(line);
    // Get temperature
    const char *orientation = getfield(tmp, 2);
    write_log(4, "Orientation: %s \n", orientation);
    // Check if temperature is valid and is not empty
    if (orientation == NULL) // If empty
    {
        retour.orientation = 0;
        // Set error flag
        wind_error_convert_line = true;
    }
    else // If not empty
    {
        // Convert temperature to float
        float orientationf = atof(orientation);
        retour.orientation = orientationf;
    }
    //  Free tmp
    free(tmp);

    // Dub line for strtok
    tmp = strdup(line);
    // Get temperature
    const char *speed = getfield(tmp, 3);
    write_log(4, "Orientation: %s \n", speed);
    // Check if temperature is valid and is not empty
    if (speed == NULL) // If empty
    {
        retour.speed = 0;
        // Set error flag
        wind_error_convert_line = true;
    }
    else // If not empty
    {
        // Convert temperature to float
        float speedf = atof(speed);
        retour.speed = speedf;
    }

    // Dub line for strtok
    tmp = strdup(line);
    // Get coordinates
    const char *coord = getfield(tmp, 4);
    write_log(4, "Coord: %s \n", coord);
    // Check if temperature is valid and is not empty
    if (coord == NULL || !split_coord(&retour.coord, coord)) // If empty
    {
        retour.coord.x = 0;
        retour.coord.y = 0;

        wind_error_convert_line = true;
    }
    free(tmp);
    return (retour);
}

void wind_tab_convert_line(windlines *tab, char *line)
{
    windline tmpline = wind_convertline(line);

    if (wind_error_convert_line)
    {
        return;
    }

    // Check if station is already in table
    windline *station = wind_tab_findstation(*tab, tmpline.stationId);
    // If station is not in table
    if (station == NULL)
    {
        // Add station to table
        tab->lines = (windline *)realloc(tab->lines, sizeof(windline) * (tab->size + 1));
        tab->lines[tab->size] = tmpline;
        tab->size++;
    }
    else
    {
        // Update station average temperature
        station->orientation = (station->orientation + tmpline.orientation) / 2;

        float station_x = station->speed * cos(station->orientation * M_PI / 180);
        float station_y = station->speed * sin(station->orientation * M_PI / 180);
        float tmpline_x = tmpline.speed * cos(tmpline.orientation * M_PI / 180);
        float tmpline_y = tmpline.speed * sin(tmpline.orientation * M_PI / 180);

        float new_x = (station_x + tmpline_x) / 2;
        float new_y = (station_y + tmpline_y) / 2;

        station->speed = sqrt(pow(new_x, 2) + pow(new_y, 2));
        station->orientation = (2 * atan(new_y / (new_x + station->speed))) * (180 / M_PI);
    }
}

void wind_tab_convert(windlines *tab, FILE *stream)
{
    // Read file line by line using seek
    char line[1024];
    int i = 0;
    while (fgets(line, 1024, stream))
    {
        write_log(4, "hLine %d: %s\n", i, line);
        // Skip first line
        if (i == 0)
        {
            i++;
            continue;
        }
        // Convert line to windline
        wind_tab_convert_line(tab, line);
        i++;
    }
}

void wind_tab_sort(windlines *tab)
{
    // Sort table by station id, quick sort algorithm
    for (int i = 0; i < tab->size; i++)
    {
        for (int j = i + 1; j < tab->size; j++)
        {
            if (meteoconf.reverse ? tab->lines[i].stationId < tab->lines[j].stationId : tab->lines[i].stationId > tab->lines[j].stationId)
            {
                windline tmp = tab->lines[i];
                tab->lines[i] = tab->lines[j];
                tab->lines[j] = tmp;
            }
        }
    }
}

void wind_tab_write(windlines *tab)
{
    // Open file
    FILE *stream = fopen(meteoconf.output_file_name, "w");

    // Check if file is open
    if (stream == NULL)
    {
        fprintf(stderr, "Error: can't open file %s for writing !\n", meteoconf.output_file_name);
        exit(EXIT_OUT_FILE);
    }

    write_log(3, "File %s opened for writing !\n", meteoconf.output_file_name);

    // Write header
    fprintf(stream, "stationId;orientation;speed;coordx;coordy\n");

    // Write table
    for (int i = 0; i < tab->size; i++)
    {
        fprintf(stream, "%d;%f;%f;%f;%f\n", tab->lines[i].stationId, tab->lines[i].orientation, tab->lines[i].speed, tab->lines[i].coord.x, tab->lines[i].coord.y);
    }

    // Close file
    fclose(stream);
}

void wind_tab()
{
    // Open file
    FILE *stream = fopen(meteoconf.input_file_name, "r");

    // Check if file is open
    if (stream == NULL)
    {
        fprintf(stderr, "Error: can't open file %s for reading !\n", meteoconf.input_file_name);
        exit(EXIT_IN_FILE);
    }

    write_log(4, "File %s opened for reading !\n", meteoconf.input_file_name);

    // Init table
    windlines windtabout = {(windline *)malloc(sizeof(windline) * 0), 0};
    write_log(4, "Table initialized !\n");

    // Convert file to table
    wind_tab_convert(&windtabout, stream);
    fclose(stream);

    // Sort table
    wind_tab_sort(&windtabout);
    // Print table
    if (meteoconf.verbose > 2)
    {
        prinwindtlines(&windtabout);
    }

    // Write table to csv file
    wind_tab_write(&windtabout);

    free(windtabout.lines);

    exit(EXIT_SUCCESS);
}

//*************************************//
// BST tree functions
//*************************************//

void wind_bst_insert(windtbst *tree, windline *line)
{
    if (tree->data == NULL)
    {
        write_log(3, "Inserting %d in tree\n", line->stationId);
        tree->data = line;
        tree->left = (windtbst *)malloc(sizeof(windtbst));
        tree->right = (windtbst *)malloc(sizeof(windtbst));
        tree->left->data = NULL;
        tree->right->data = NULL;
    }
    else
    {
        if (tree->data->stationId == line->stationId)
        {
            // Update station average temperature
            tree->data->orientation = (tree->data->orientation + line->orientation) / 2;

            float station_x = tree->data->speed * cos(tree->data->orientation * M_PI / 180);
            float station_y = tree->data->speed * sin(tree->data->orientation * M_PI / 180);
            float tmpline_x = line->speed * cos(line->orientation * M_PI / 180);
            float tmpline_y = line->speed * sin(line->orientation * M_PI / 180);

            float new_x = (station_x + tmpline_x) / 2;
            float new_y = (station_y + tmpline_y) / 2;

            tree->data->speed = sqrt(pow(new_x, 2) + pow(new_y, 2));
            tree->data->orientation = (2 * atan(new_y / (new_x + tree->data->speed))) * (180 / M_PI);
        }
        else if (meteoconf.reverse ? tree->data->stationId < line->stationId : tree->data->stationId > line->stationId)
        {
            wind_bst_insert(tree->left, line);
        }
        else
        {
            wind_bst_insert(tree->right, line);
        }
    }
}

void wind_bst_convert(windtbst *tree, FILE *stream)
{
    // Read file line by line using seek
    char line[1024];
    int i = 0;
    while (fgets(line, 1024, stream))
    {
        write_log(4, "hLine %d: %s\n", i, line);
        // Skip first line
        if (i == 0)
        {
            i++;
            continue;
        }
        // Convert line to windline
        windline tmpline = wind_convertline(line);
        // Copy line to memory
        windline *tmplineptr = (windline *)malloc(sizeof(windline));
        *tmplineptr = tmpline;
        tmplineptr->coord.x = tmpline.coord.y;
        tmplineptr->coord.y = tmpline.coord.x;
        tmplineptr->orientation = tmpline.orientation;

        // Insert line in tree
        wind_bst_insert(tree, tmplineptr);
        i++;
    }
}

void wind_bst_inorder(windtbst *tree, FILE *stream)
{
    if (tree->data != NULL)
    {
        wind_bst_inorder(tree->left, stream);
        fprintf(stream, "%d;%f;%f;%f;%f\n", tree->data->stationId, tree->data->orientation, tree->data->speed, tree->data->coord.x, tree->data->coord.y);
        wind_bst_inorder(tree->right, stream);
    }
}

void wind_bst_write(windtbst *tree)
{
    // Open file
    FILE *stream = fopen(meteoconf.output_file_name, "w");

    // Check if file is open
    if (stream == NULL)
    {
        fprintf(stderr, "Error: can't open file %s for writing !\n", meteoconf.output_file_name);
        exit(EXIT_OUT_FILE);
    }

    write_log(3, "File %s opened for writing !\n", meteoconf.output_file_name);

    // Write header
    fprintf(stream, "stationId;orientation;speed;coordx;coordy\n");

    wind_bst_inorder(tree, stream);

    fclose(stream);
}

void wind_bst()
{
    // Open file
    FILE *stream = fopen(meteoconf.input_file_name, "r");

    // Check if file is open
    if (stream == NULL)
    {
        fprintf(stderr, "Error: can't open file %s for reading !\n", meteoconf.input_file_name);
        exit(EXIT_IN_FILE);
    }

    write_log(4, "File %s opened for reading !\n", meteoconf.input_file_name);

    // Init tree
    windtbst *windtree = (windtbst *)malloc(sizeof(windtbst));
    windtree->data = NULL;

    // Convert file to tree
    wind_bst_convert(windtree, stream);
    fclose(stream);

    // Write tree to csv file
    wind_bst_write(windtree);

    exit(EXIT_SUCCESS);
}

void wind()
{
    switch (meteoconf.sort_algo)
    {
    case ALGO_TAB:
        wind_tab();
        break;
    case ALGO_ABR:
        wind_bst();
        break;
    default:
        printf("Error: unknown sort algorithm !\n");
        break;
    }
}
