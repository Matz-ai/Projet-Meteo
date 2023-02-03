#include "pt/t2.h"

void print2tlines(t2lines *tab)
{
    printf("Printing table of size %d\n", tab->size);
    printf("Date Avg\n");

    // print using iso 8601 format
    for (int i = 0; i < tab->size; i++)
    {
        char date[20];
        strftime(date, 20, "%F", localtime(&tab->lines[i].date));
        printf("%s %f \n", date, tab->lines[i].avg_temperature);
    }
}

t2line *t2_tab_finddate(t2lines tab, time_t time)
{
    t2line *retour = NULL; // Variable de retour

    // Find station in table
    for (int i = 0; i < tab.size; i++)
    {
        if (difftime(tab.lines[i].date, time) == 0)
        {
            retour = &tab.lines[i];
            break;
        }
    }

    return (retour);
}

// Used to indicate that an error occured during the conversion
bool t2_error_convert_line = false;

t2line t2_convertline(char *line)
{
    // Reset error flag
    t2_error_convert_line = false;
    // init return variable
    t2line retour = {0, 0};
    write_log(4, "Converting line: %s \n", line);

    // Dub line for strtok
    char *tmp = strdup(line);
    write_log(4, "tmp: %s \n", tmp);
    // Get time
    const char *time = getfield(tmp, 1);
    write_log(4, "Time: %s \n", time);

    if (time == NULL)
    {
        // Set error flag
        t2_error_convert_line = true;
        return (retour);
    }

    // Read iso 8601 date
    struct tm tm = {0};
    strptime(time, "%Y-%m-%dT%H:%M:S%z", &tm);
    // Convert date to time_t
    time_t t = mktime(&tm);
    retour.date = t;

    // Print date
    char date[20];
    strftime(date, 20, "%F", localtime(&t));
    write_log(4, "Date: %s \n", date);

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
        // Set error flag
        t2_error_convert_line = true;
    }
    else // If not empty
    {
        // Convert temperature to float
        float avgf = atof(avg);
        retour.avg_temperature = avgf;
    }

    //  Free tmp
    free(tmp);

    return (retour);
}

void t2_tab_convert_line(t2lines *tab, char *line)
{
    t2line tmpline = t2_convertline(line);

    if (t2_error_convert_line)
    {
        return;
    }

    // Check if station is already in table
    t2line *station = t2_tab_finddate(*tab, tmpline.date);
    // If station is not in table
    if (station == NULL)
    {
        // Add station to table
        tab->lines = (t2line *)realloc(tab->lines, sizeof(t2line) * (tab->size + 1));
        tab->lines[tab->size] = tmpline;
        tab->size++;
    }
    else
    {
        // Update station average temperature
        station->avg_temperature = (station->avg_temperature + tmpline.avg_temperature) / 2;
    }
}

void t2_tab_convert(t2lines *tab, FILE *stream)
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

        // Convert line to t2line
        t2_tab_convert_line(tab, line);
        i++;
    }
}

/**
 *  @fn void t2_tab_sort (t2lines *tab)
 *  @version 0.1
 *  @date Fri 03 Feb 2023 12:19
 *
 *  @brief
 *
 *
 */
void t2_tab_sort(t2lines *tab)
{
    // Sort table by station id, quick sort algorithm
    for (int i = 0; i < tab->size; i++)
    {
        for (int j = i + 1; j < tab->size; j++)
        {
            if (meteoconf.reverse ? (difftime(tab->lines[i].date, tab->lines[j].date) < 0) : (difftime(tab->lines[i].date, tab->lines[j].date) > 0))
            {
                t2line tmp = tab->lines[i];
                tab->lines[i] = tab->lines[j];
                tab->lines[j] = tmp;
            }
        }
    }
}

/**
 *  @fn void t2_tab_write (t2lines *tab)
 *  @version 0.1
 *  @date Fri 03 Feb 2023 12:23
 *
 *  @brief Write table to csv file
 *
 *  @param[in]
 *
 */
void t2_tab_write(t2lines *tab)
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
    fprintf(stream, "date;avg_temperature\n");

    // Write table
    for (int i = 0; i < tab->size; i++)
    {
        // Convert date to string
        char date[20];
        strftime(date, 20, "%FT%T", localtime(&tab->lines[i].date));
        // Write line
        fprintf(stream, "%s;%f\n", date, tab->lines[i].avg_temperature);
    }

    // Close file
    fclose(stream);
}

void t2_tab()
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
    t2lines t2tabout = {(t2line *)malloc(sizeof(t2line) * 0), 0};
    write_log(4, "Table initialized !\n");

    // Convert file to table
    t2_tab_convert(&t2tabout, stream);
    fclose(stream);

    // Sort table
    t2_tab_sort(&t2tabout);
    // Print table
    if (meteoconf.verbose > 2)
    {
        print2tlines(&t2tabout);
    }
    // Write table to csv file
    t2_tab_write(&t2tabout);

    free(t2tabout.lines);

    exit(EXIT_SUCCESS);
}

//*************************************//
// BST tree functions
//*************************************//

void t2_bst_insert(t2bst *tree, t2line *line)
{
    // If tree is empty
    if (tree->data == NULL)
    {
        tree->data = line;
        tree->left = (t2bst *)malloc(sizeof(t2bst));
        tree->right = (t2bst *)malloc(sizeof(t2bst));
        tree->left->data = NULL;
        tree->right->data = NULL;
    }
    else
    {
        // If station id is lower than current node station id
        if (difftime(tree->data->date, line->date) == 0)
        {
            // Update station average pressure
            tree->data->avg_temperature = (tree->data->avg_temperature + line->avg_temperature) / 2;
        }
        else if (meteoconf.reverse ? (difftime(line->date, tree->data->date) > 0) : (difftime(line->date, tree->data->date) < 0))
        {
            // Insert in left subtree
            t2_bst_insert(tree->left, line);
        }
        else
        {
            // Insert in right subtree
            t2_bst_insert(tree->right, line);
        }
    }
}

void t2_bst_convert(t2bst *tree, FILE *stream)
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

        // Convert line to t2line
        t2line tmpline = t2_convertline(line);

        // Check if line convcertion is ok
        if (t2_error_convert_line)
        {
            write_log(2, "Error: can't convert line %d !\n", i);
            continue;
        }

        // copy tmpline to memory
        t2line *tmp = (t2line *)malloc(sizeof(t2line));
        *tmp = tmpline;

        // Insert line in tree
        t2_bst_insert(tree, tmp);

        i++;
    }
}

void t2_bst_inorder(t2bst *tree, FILE *stream)
{
    if (tree != NULL && tree->data != NULL)
    {
        // Write left subtree
        t2_bst_inorder(tree->left, stream);
        // Write current node
        // Convert date to string
        char date[30];
        strftime(date, 30, "%FT%T", localtime(&tree->data->date));
        // Write line
        fprintf(stream, "%s;%f\n", date, tree->data->avg_temperature);
        // Write right subtree
        t2_bst_inorder(tree->right, stream);
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

void t2_bst_write(t2bst *tree)
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
    fprintf(stream, "date;avg_temperature\n");

    // Write tree
    t2_bst_inorder(tree, stream);

    // Close file
    fclose(stream);
}

void t2_bst()
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
    t2bst *tree = (t2bst *)malloc(sizeof(t2bst));
    tree->data = NULL;
    // Convert file to tree
    t2_bst_convert(tree, stream);
    fclose(stream);

    // Write tree to csv file
    t2_bst_write(tree);

    exit(EXIT_SUCCESS);
}

void t2()
{
    switch (meteoconf.sort_algo)
    {
    case ALGO_TAB:
        t2_tab();
        break;
    case ALGO_ABR:
        t2_bst();
        break;
    default:
        printf("Error: unknown sort algorithm !\n");
        break;
    }
}
