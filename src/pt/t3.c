#include "pt/t3.h"

void printt3lines(t3l2lines *tab)
{
    printf("Printing table of size %d\n", tab->size);
    printf("Date Avg\n");

    // Print l2 table
    for (int i = 0; i < tab->size; i++)
    {
        char date[20];
        strftime(date, 20, "%F", localtime(&tab->tabs[i].date));

        for (int j = 0; j < tab->tabs[i].size; j++)
        {
            printf("%s %d %f \n", date, tab->tabs[i].lines[j].stationId, tab->tabs[i].lines[j].temperature);
        }
    }
}

t3l1lines *t3_tab_finddate(t3l2lines tab, time_t time)
{
    t3l1lines *retour = NULL; // Variable de retour

    // Find station in table
    for (int i = 0; i < tab.size; i++)
    {
        if (difftime(tab.tabs[i].date, time) == 0)
        {
            retour = &tab.tabs[i];
            break;
        }
    }

    return (retour);
}

void t3_tab_convert_line(t3l2lines *tab, char *line)
{

    write_log(4, "Converting line: %s \n", line);

    // Dub line for strtok
    char *tmp = strdup(line);
    write_log(4, "tmp: %s \n", tmp);
    // Get station id
    const char *id = getfield(tmp, 1);
    write_log(4, "Station id: %s \n", id);
    // Convert station id to int
    int stationId = atoi(id);
    // Free tmp
    free(tmp);

    // Dub line for strtok
    tmp = strdup(line);
    write_log(4, "tmp: %s \n", tmp);
    // Get time
    const char *time = getfield(tmp, 2);
    write_log(4, "Time: %s \n", time);
    // Read iso 8601 date
    struct tm tm = {0};
    strptime(time, "%Y-%m-%dT%H:%M:S%z", &tm);
    // Convert date to time_t
    time_t date = mktime(&tm);

    // Free tmp
    free(tmp);

    // Dub line for strtok
    tmp = strdup(line);
    // Get temperature
    const char *avg = getfield(tmp, 3);
    write_log(4, "Avg: %s \n", avg);
    // Ignore station if temp is null
    if (avg == NULL)
    {
        return;
    }

    // Convert temperature to float
    float avg_temperature = atof(avg);

    t3l1lines *dateTab = t3_tab_finddate(*tab, date);
    // If station is not in table
    if (dateTab == NULL)
    {
        // Add station to table
        tab->tabs = (t3l1lines *)realloc(tab->tabs, sizeof(t3l1lines) * (tab->size + 1));
        tab->tabs[tab->size].date = date;
        tab->tabs[tab->size].lines = (t3line *)malloc(sizeof(t3line));
        tab->tabs[tab->size].lines[0].stationId = stationId;
        tab->tabs[tab->size].lines[0].temperature = avg_temperature;
        tab->tabs[tab->size].size = 1;
        tab->size++;
    }
    else
    {
        // Update station average temperature
        dateTab->lines = (t3line *)realloc(dateTab->lines, sizeof(t3line) * (dateTab->size + 1));
        dateTab->lines[dateTab->size].stationId = stationId;
        dateTab->lines[dateTab->size].temperature = avg_temperature;
        dateTab->size++;
    }

    //  Free tmp
    free(tmp);
}

void t3_tab_convert(t3l2lines *tab, FILE *stream)
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
        // Convert line to t3line
        t3_tab_convert_line(tab, line);
        i++;
    }
}

/**
 *  @fn void t3_tab_sort_id (t3l1lines *tab)
 *  @version 0.1
 *  @date Fri 03 Feb 2023 14:30
 *
 *  @brief
 *
 *  @param[in]
 *
 */
void t3_tab_sort_id(t3l1lines *tab)
{
    // Sort table by station id, quick sort algorithm
    for (int i = 0; i < tab->size; i++)
    {
        for (int j = i + 1; j < tab->size; j++)
        {
            if (meteoconf.reverse ? (tab->lines[i].stationId < tab->lines[j].stationId) : (tab->lines[i].stationId > tab->lines[j].stationId))
            {
                t3line tmp = tab->lines[i];
                tab->lines[i] = tab->lines[j];
                tab->lines[j] = tmp;
            }
        }
    }
}

/**
 *  @fn void t3_tab_sort (t3lines *tab)
 *  @version 0.1
 *  @date Fri 03 Feb 2023 12:19
 *
 *  @brief
 *
 *
 */
void t3_tab_sort_date(t3l2lines *tab)
{
    // Sort table by date, quick sort algorithm
    for (int i = 0; i < tab->size; i++)
    {
        for (int j = i + 1; j < tab->size; j++)
        {
            if (meteoconf.reverse ? (tab->tabs[i].date < tab->tabs[j].date) : (tab->tabs[i].date > tab->tabs[j].date))
            {
                t3l1lines tmp = tab->tabs[i];
                tab->tabs[i] = tab->tabs[j];
                tab->tabs[j] = tmp;
            }
            t3_tab_sort_id(&tab->tabs[i]);
        }
    }
}

/**
 *  @fn void t3_tab_write (t3lines *tab)
 *  @version 0.1
 *  @date Fri 03 Feb 2023 12:23
 *
 *  @brief Write table to csv file
 *
 *  @param[in]
 *
 */
void t3_tab_write(t3l2lines *tab)
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
        // Convert date to iso 8601
        char date[20];
        strftime(date, 20, "%Y-%m-%dT%H:%M:%S", localtime(&tab->tabs[i].date));
        // Write date

        for (int j = 0; j < tab->tabs[i].size; j++)
        {
            fprintf(stream, "%s;%d;%f\n", date, tab->tabs[i].lines[j].stationId, tab->tabs[i].lines[j].temperature);
        }
    }

    // Close file
    fclose(stream);
}

void t3_tab()
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
    t3l2lines t3tabout = {(t3l1lines *)malloc(sizeof(t3l1lines) * 0), 0};
    write_log(4, "Table initialized !\n");

    // Convert file to table
    t3_tab_convert(&t3tabout, stream);
    fclose(stream);

    // Sort table
    t3_tab_sort_date(&t3tabout);
    // Print table
    if (meteoconf.verbose > 2)
    {
        printt3lines(&t3tabout);
    }
    // Write table to csv file
    t3_tab_write(&t3tabout);

    // Free tables
    for (int i = 0; i < t3tabout.size; i++)
    {
        free(t3tabout.tabs[i].lines);
    }
    free(t3tabout.tabs);

    exit(EXIT_SUCCESS);
}
