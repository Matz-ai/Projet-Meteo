#include "pt/t1.h"

void print1tlines(t1lines *tab)
{
    printf("Printing table of size %d\n", tab->size);
    printf("StationId Avg Min Max \n");

    for (int i = 0; i < tab->size; i++)
    {
        printf("%d %f %f %f \n", tab->lines[i].stationId, tab->lines[i].avg_temperature, tab->lines[i].min_temperature, tab->lines[i].max_temperature);
    }
}

t1line *t1_tab_findstation(t1lines tab, int stationId)
{
    t1line *retour = NULL; // Variable de retour

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
bool t1_error_convert_line = false;

t1line t1_convertline(char *line)
{
    // Reset error flag
    t1_error_convert_line = false;
    // init return variable
    t1line retour = {0, 0, 0, 0};
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
    const char *avg = getfield(tmp, 2);
    write_log(4, "Avg: %s \n", avg);
    // Check if temperature is valid and is not empty
    if (avg == NULL) // If empty
    {

        retour.avg_temperature = 0;
        retour.min_temperature = 0;
        retour.max_temperature = 0;
        // Set error flag
        t1_error_convert_line = true;
    }
    else // If not empty
    {
        // Convert temperature to float
        float avgf = atof(avg);

        retour.avg_temperature = avgf;
        retour.min_temperature = avgf;
        retour.max_temperature = avgf;
    }

    //  Free tmp
    free(tmp);

    return (retour);
}

void t1_tab_convert_line(t1lines *tab, char *line)
{
    t1line tmpline = t1_convertline(line);

    if (t1_error_convert_line)
    {
        return;
    }

    // Check if station is already in table
    t1line *station = t1_tab_findstation(*tab, tmpline.stationId);
    // If station is not in table
    if (station == NULL)
    {
        // Add station to table
        tab->lines = (t1line *)realloc(tab->lines, sizeof(t1line) * (tab->size + 1));
        tab->lines[tab->size] = tmpline;
        tab->size++;
    }
    else
    {
        // Update station average temperature
        station->avg_temperature = (station->avg_temperature + tmpline.avg_temperature) / 2;
        // Update station min temperature
        if (station->min_temperature > tmpline.min_temperature)
        {
            station->min_temperature = tmpline.min_temperature;
        }
        // Update station max temperature
        if (station->max_temperature < tmpline.max_temperature)
        {
            station->max_temperature = tmpline.max_temperature;
        }
    }
}

void t1_tab_convert(t1lines *tab, FILE *stream)
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
        // Convert line to t1line
        t1_tab_convert_line(tab, line);
        i++;
    }
}

/**
 *  @fn void t1_tab_sort (t1lines *tab)
 *  @version 0.1
 *  @date Fri 03 Feb 2023 12:19
 *
 *  @brief
 *
 *
 */
void t1_tab_sort(t1lines *tab)
{
    // Sort table by station id, quick sort algorithm
    for (int i = 0; i < tab->size; i++)
    {
        for (int j = i + 1; j < tab->size; j++)
        {
            if (meteoconf.reverse ? tab->lines[i].stationId < tab->lines[j].stationId : tab->lines[i].stationId > tab->lines[j].stationId)
            {
                t1line tmp = tab->lines[i];
                tab->lines[i] = tab->lines[j];
                tab->lines[j] = tmp;
            }
        }
    }
}

/**
 *  @fn void t1_tab_write (t1lines *tab)
 *  @version 0.1
 *  @date Fri 03 Feb 2023 12:23
 *
 *  @brief Write table to csv file
 *
 *  @param[in]
 *
 */
void t1_tab_write(t1lines *tab)
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
    fprintf(stream, "stationId;avg_temperature;min_temperature;max_temperature\n");

    // Write table
    for (int i = 0; i < tab->size; i++)
    {
        fprintf(stream, "%d;%f;%f;%f\n", tab->lines[i].stationId, tab->lines[i].avg_temperature, tab->lines[i].min_temperature, tab->lines[i].max_temperature);
    }

    // Close file
    fclose(stream);
}

void t1_tab()
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
    t1lines t1tabout = {(t1line *)malloc(sizeof(t1line) * 0), 0};
    write_log(4, "Table initialized !\n");

    // Convert file to table
    t1_tab_convert(&t1tabout, stream);
    fclose(stream);

    // Sort table
    t1_tab_sort(&t1tabout);
    // Print table
    if (meteoconf.verbose > 2)
    {
        print1tlines(&t1tabout);
    }

    // Write table to csv file
    t1_tab_write(&t1tabout);

    free(t1tabout.lines);

    exit(EXIT_SUCCESS);
}

//*************************************//
// BST tree functions
//*************************************//

void t1_bst_insert(t1bst *tree, t1line *line)
{
    // If tree is empty
    if (tree->data == NULL)
    {
        tree->data = line;
        tree->left = (t1bst *)malloc(sizeof(t1bst));
        tree->right = (t1bst *)malloc(sizeof(t1bst));
        tree->left->data = NULL;
        tree->right->data = NULL;
    }
    else
    {
        // If station id is lower than current node station id
        if (line->stationId == tree->data->stationId)
        {
            // Update station average temperature
            tree->data->avg_temperature = (tree->data->avg_temperature + line->avg_temperature) / 2;
            // Update station min temperature
            if (tree->data->min_temperature > line->min_temperature)
            {
                tree->data->min_temperature = line->min_temperature;
            }
            // Update station max temperature
            if (tree->data->max_temperature < line->max_temperature)
            {
                tree->data->max_temperature = line->max_temperature;
            }
        }
        else if (meteoconf.reverse ? line->stationId > tree->data->stationId : line->stationId < tree->data->stationId)
        {
            // Insert in left subtree
            t1_bst_insert(tree->left, line);
        }
        else
        {
            // Insert in right subtree
            t1_bst_insert(tree->right, line);
        }
    }
}

void t1_bst_convert(t1bst *tree, FILE *stream)
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

        t1line linec = t1_convertline(line);

        // Check if line convcertion is ok
        if (t1_error_convert_line)
        {
            write_log(2, "Error: can't convert line %d !\n", i);
            continue;
        }

        // Copy line to malloced memory
        t1line *linep = (t1line *)malloc(sizeof(t1line));
        *linep = linec;

        t1_bst_insert(tree, linep);

        i++;
    }
}

void t1_bst_inorder(t1bst *tree, FILE *stream)
{
    if (tree->data != NULL)
    {
        t1_bst_inorder(tree->left, stream);
        fprintf(stream, "%d;%.2f;%.2f;%.2f \n", tree->data->stationId, tree->data->avg_temperature, tree->data->min_temperature, tree->data->max_temperature);
        t1_bst_inorder(tree->right, stream);
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

void t1_bst_write(t1bst *tree)
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
    fprintf(stream, "stationId;avg_temperature;min_temperature;max_temperature\n");

    // traverse tree in order
    t1_bst_inorder(tree, stream);

    // Close file
    fclose(stream);
}

void t1_bst()
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

    // Convert file to bst
    t1bst *tree = (t1bst *)malloc(sizeof(t1bst));
    tree->data = NULL;
    t1_bst_convert(tree, stream);
    fclose(stream);

    // Write table to csv file
    t1_bst_write(tree);

    exit(EXIT_SUCCESS);
}

void t1()
{
    switch (meteoconf.sort_algo)
    {
    case ALGO_TAB:
        t1_tab();
        break;
    case ALGO_ABR:
        t1_bst();
        break;
    default:
        printf("Error: unknown sort algorithm !\n");
        break;
    }
}
