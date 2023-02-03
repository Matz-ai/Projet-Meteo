#include "moisture.h"

void prinmoisturetlines(moisturelines *tab)
{
    printf("Printing table of size %d\n", tab->size);
    printf("StationId Height \n");

    for (int i = 0; i < tab->size; i++)
    {
        printf("%d %d \n", tab->lines[i].stationId, tab->lines[i].moisture);
    }
}

moistureline *moisture_tab_findstation(moisturelines tab, int stationId)
{
    moistureline *retour = NULL; // Variable de retour

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
bool moisture_error_convert_line = false;

moistureline moisture_convertline(char *line)
{
    // Reset error flag
    moisture_error_convert_line = false;
    // init return variable
    moistureline retour = {0, 0, {0, 0}};
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
    const char *moisture = getfield(tmp, 2);
    write_log(4, "Orientation: %s \n", moisture);
    // Check if temperature is valid and is not empty
    if (moisture == NULL) // If empty
    {
        retour.moisture = 0;
        // Set error flag
        moisture_error_convert_line = true;
    }
    else // If not empty
    {
        // Convert temperature to float
        int moisturef = atoi(moisture);
        retour.moisture = moisturef;
    }
    //  Free tmp
    free(tmp);

    // Dub line for strtok
    tmp = strdup(line);
    // Get coordinates
    const char *coord = getfield(tmp, 3);
    write_log(4, "Coord: %s \n", coord);
    // Check if temperature is valid and is not empty
    if (coord == NULL || !split_coord(&retour.coord, coord)) // If empty
    {
        retour.coord.x = 0;
        retour.coord.y = 0;

        moisture_error_convert_line = true;
    }
    free(tmp);

    return (retour);
}

void moisture_tab_convert_line(moisturelines *tab, char *line)
{
    moistureline tmpline = moisture_convertline(line);

    if (moisture_error_convert_line)
    {
        return;
    }

    // Check if station is already in table
    moistureline *station = moisture_tab_findstation(*tab, tmpline.stationId);
    // If station is not in table
    if (station == NULL)
    {
        // Add station to table
        tab->lines = (moistureline *)realloc(tab->lines, sizeof(moistureline) * (tab->size + 1));
        tab->lines[tab->size] = tmpline;
        tab->size++;
    }
    else
    {
        if (tmpline.stationId == station->stationId)
        {
            // Update station average temperature if new temperature is higher
            if (tmpline.moisture > station->moisture)
            {
                station->moisture = tmpline.moisture;
            }
        }
    }
}

void moisture_tab_convert(moisturelines *tab, FILE *stream)
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
        // Convert line to moistureline
        moisture_tab_convert_line(tab, line);
        i++;
    }
}

void moisture_tab_sort(moisturelines *tab)
{
    // Sort table by station id, quick sort algorithm
    for (int i = 0; i < tab->size; i++)
    {
        for (int j = i + 1; j < tab->size; j++)
        {
            if (meteoconf.reverse ? tab->lines[i].stationId < tab->lines[j].stationId : tab->lines[i].stationId > tab->lines[j].stationId)
            {
                moistureline tmp = tab->lines[i];
                tab->lines[i] = tab->lines[j];
                tab->lines[j] = tmp;
            }
        }
    }
}

void moisture_tab_write(moisturelines *tab)
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
    fprintf(stream, "stationId;moisture;coordx;coordy\n");

    // Write table
    for (int i = 0; i < tab->size; i++)
    {
        fprintf(stream, "%d;%d;%f;%f\n", tab->lines[i].stationId, tab->lines[i].moisture, tab->lines[i].coord.x, tab->lines[i].coord.y);
    }

    // Close file
    fclose(stream);
}

void moisture_tab()
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
    moisturelines moisturetabout = {(moistureline *)malloc(sizeof(moistureline) * 0), 0};
    write_log(4, "Table initialized !\n");

    // Convert file to table
    moisture_tab_convert(&moisturetabout, stream);
    fclose(stream);

    // Sort table
    moisture_tab_sort(&moisturetabout);
    // Print table
    if (meteoconf.verbose > 2)
    {
        prinmoisturetlines(&moisturetabout);
    }

    // Write table to csv file
    moisture_tab_write(&moisturetabout);

    free(moisturetabout.lines);

    exit(EXIT_SUCCESS);
}

//*************************************//
// BST tree functions
//*************************************//

void moisture_bst_insert(moisturebst *tree, moistureline *line)
{
    // If tree is empty
    if (tree->data == NULL)
    {
        tree->data = line;
        tree->left = (moisturebst *)malloc(sizeof(moisturebst));
        tree->right = (moisturebst *)malloc(sizeof(moisturebst));
        tree->left->data = NULL;
        tree->right->data = NULL;
    }
    else
    {
        // If station id is lower than current node
        if (tree->data->stationId == line->stationId)
        {
            // Update station average temperature if new temperature is higher
            if (line->moisture > tree->data->stationId)
            {
                tree->data->moisture = line->moisture;
            }
        }
        else if (meteoconf.reverse ? line->stationId > tree->data->stationId : line->stationId < tree->data->stationId)
        {
            // Go to left node
            moisture_bst_insert(tree->left, line);
        }
        else
        {
            // Go to right node
            moisture_bst_insert(tree->right, line);
        }
    }
}

void moisture_bst_convert(moisturebst *tree, FILE *stream)
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
        // Convert line to moistureline
        moistureline tmpline = moisture_convertline(line);
        // If line is not valid
        if (tmpline.stationId == -1)
        {
            continue;
        }

        // Copy line to memory
        moistureline *lineptr = (moistureline *)malloc(sizeof(moistureline));
        *lineptr = tmpline;

        // Insert line to tree
        moisture_bst_insert(tree, lineptr);
        i++;
    }
}

void moisture_bst_inorder(moisturebst *tree, FILE *stream)
{
    if (tree->data != NULL)
    {
        moisture_bst_inorder(tree->left, stream);
        fprintf(stream, "%d;%d;%f;%f\n", tree->data->stationId, tree->data->moisture, tree->data->coord.x, tree->data->coord.y);
        moisture_bst_inorder(tree->right, stream);
    }
    if (tree->left == NULL)
    {
        free(tree->left);
    }
    if (tree->right == NULL)
    {
        free(tree->right);
    }
}

void moisture_bst_write(moisturebst *tree)
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
    fprintf(stream, "stationId;moisture;coordx;coordy\n");

    // Write tree
    moisture_bst_inorder(tree, stream);

    // Close file
    fclose(stream);
}

void moisture_bst()
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
    moisturebst *moisturetree = (moisturebst *)malloc(sizeof(moisturebst));
    moisturetree->data = NULL;

    // Convert file to tree
    moisture_bst_convert(moisturetree, stream);
    fclose(stream);

    // Write tree to csv file
    moisture_bst_write(moisturetree);

    exit(EXIT_SUCCESS);
}

void moisture()
{
    switch (meteoconf.sort_algo)
    {
    case ALGO_TAB:
        moisture_tab();
        break;
    case ALGO_ABR:
        moisture_bst();
        break;
    default:
        printf("Error: unknown sort algorithm !\n");
        break;
    }
}
