#include "height.h"

void prinheighttlines(heightlines *tab)
{
    printf("Printing table of size %d\n", tab->size);
    printf("StationId Height \n");

    for (int i = 0; i < tab->size; i++)
    {
        printf("%d %d \n", tab->lines[i].stationId, tab->lines[i].height);
    }
}

heightline *height_tab_findstation(heightlines tab, int stationId)
{
    heightline *retour = NULL; // Variable de retour

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
bool height_error_convert_line = false;

heightline height_convertline(char *line)
{
    // Reset error flag
    height_error_convert_line = false;
    // init return variable
    heightline retour = {
        0,
        0,
        {0, 0},
    };
    write_log(4, "Converting line: %s \n", line);

    // Dub line for strtok
    char *tmp = strdup(line);
    write_log(4, "tmp: %s \n", tmp);
    // Get station id
    const char *id = getfield(tmp, 1);
    write_log(4, "Station id: %s \n", id);
    if (id == NULL) // If empty
    {
        retour.stationId = 0;
        // Set error flag
        height_error_convert_line = true;
    }
    // Convert station id to int
    retour.stationId = atoi(id);
    // Free tmp
    free(tmp);

    // Dub line for strtok
    tmp = strdup(line);
    // Get temperature
    const char *height = getfield(tmp, 2);
    write_log(4, "Orientation: %s \n", height);
    // Check if temperature is valid and is not empty
    if (height == NULL) // If empty
    {
        retour.height = 0;
        // Set error flag
        height_error_convert_line = true;
    }
    else // If not empty
    {
        // Convert temperature to float
        int heightf = atoi(height);
        retour.height = heightf;
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

        height_error_convert_line = true;
    }
    free(tmp);

    return (retour);
}

void height_tab_convert_line(heightlines *tab, char *line)
{
    heightline tmpline = height_convertline(line);

    if (height_error_convert_line)
    {
        return;
    }

    // Check if station is already in table
    heightline *station = height_tab_findstation(*tab, tmpline.stationId);
    // If station is not in table
    if (station == NULL)
    {
        // Add station to table
        tab->lines = (heightline *)realloc(tab->lines, sizeof(heightline) * (tab->size + 1));
        tab->lines[tab->size] = tmpline;
        tab->size++;
    }
    else
    {
        // Update station average height
        station->height = tmpline.height;
    }
}

void height_tab_convert(heightlines *tab, FILE *stream)
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
        // Convert line to heightline
        height_tab_convert_line(tab, line);
        i++;
    }
}

void height_tab_sort(heightlines *tab)
{
    // Sort table by station id, quick sort algorithm
    for (int i = 0; i < tab->size; i++)
    {
        for (int j = i + 1; j < tab->size; j++)
        {
            if (meteoconf.reverse ? tab->lines[i].stationId < tab->lines[j].stationId : tab->lines[i].stationId > tab->lines[j].stationId)
            {
                heightline tmp = tab->lines[i];
                tab->lines[i] = tab->lines[j];
                tab->lines[j] = tmp;
            }
        }
    }
}

void height_tab_write(heightlines *tab)
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
    fprintf(stream, "stationId;height;coordx;coordy\n");

    // Write table
    for (int i = 0; i < tab->size; i++)
    {
        fprintf(stream, "%d;%d;%f;%f\n", tab->lines[i].stationId, tab->lines[i].height, tab->lines[i].coord.x, tab->lines[i].coord.y);
    }

    // Close file
    fclose(stream);
}

void height_tab()
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
    heightlines heighttabout = {(heightline *)malloc(sizeof(heightline) * 0), 0};
    write_log(4, "Table initialized !\n");

    // Convert file to table
    height_tab_convert(&heighttabout, stream);
    fclose(stream);

    // Sort table
    height_tab_sort(&heighttabout);
    // Print table
    if (meteoconf.verbose > 2)
    {
        prinheighttlines(&heighttabout);
    }

    // Write table to csv file
    height_tab_write(&heighttabout);

    free(heighttabout.lines);

    exit(EXIT_SUCCESS);
}

//*************************************//
// BST tree functions
//*************************************//

void height_bst_insert(heightbst *tree, heightline *line)
{
    if (tree->data == NULL)
    {
        write_log(3, "Inserting %d in tree\n", line->stationId);
        tree->data = line;
        tree->left = (heightbst *)malloc(sizeof(heightbst));
        tree->right = (heightbst *)malloc(sizeof(heightbst));
        tree->left->data = NULL;
        tree->right->data = NULL;
    }
    else
    {
        if (tree->data->stationId == line->stationId)
        {
            // write_log(3, "Updating %d in tree with %d\n", tree->data->stationId, line->stationId);
            tree->data->height = line->height;
        }
        else if (meteoconf.reverse ? tree->data->stationId > line->stationId : tree->data->stationId < line->stationId)
        {
            write_log(3, "Inserting %d in tree L\n", line->stationId);
            height_bst_insert(tree->left, line);
        }
        else
        {
            write_log(3, "Inserting %d in tree R\n", line->stationId);

            height_bst_insert(tree->right, line);
        }
    }
}

void height_bst_convert(heightbst *tree, FILE *stream)
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
        // Convert line to heightline
        heightline tmpline = height_convertline(line);
        if (height_error_convert_line)
        {
            write_log(2, "Error: can't convert line %d !\n", i);
            continue;
        }
        heightline *tmpline2 = (heightline *)malloc(sizeof(heightline));
        tmpline2->stationId = tmpline.stationId;
        tmpline2->height = tmpline.height;
        tmpline2->coord.x = tmpline.coord.x;
        tmpline2->coord.y = tmpline.coord.y;

        // Add line to tree
        height_bst_insert(tree, tmpline2);
        i++;
    }
}

void height_bst_inorder(heightbst *tree, FILE *stream)
{
    if (tree != NULL && tree->data != NULL)
    {
        height_bst_inorder(tree->left, stream);
        fprintf(stream, "%d;%d;%f;%f\n", tree->data->stationId, tree->data->height, tree->data->coord.x, tree->data->coord.y);
        height_bst_inorder(tree->right, stream);
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

void height_bst_write(heightbst *tree)
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
    fprintf(stream, "stationId;height;coordx;coordy\n");

    // Write tree
    height_bst_inorder(tree, stream);

    // Close file
    fclose(stream);
}

void height_bst()
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
    heightbst *tree = (heightbst *)malloc(sizeof(heightbst));
    tree->data = NULL;

    // Convert file to tree
    height_bst_convert(tree, stream);
    fclose(stream);

    // Write tree to csv file
    height_bst_write(tree);

    exit(EXIT_SUCCESS);
}

void height()
{
    switch (meteoconf.sort_algo)
    {
    case ALGO_TAB:
        height_tab();
        break;
    case ALGO_ABR:
        height_bst();
        break;
    default:
        printf("Error: unknown sort algorithm !\n");
        break;
    }
}
