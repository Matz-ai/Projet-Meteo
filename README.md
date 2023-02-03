# Projet CY-Météo

## Dependencies

- `gnuplot`
- `gcc`
- `make`

## Usage

Make sure `meteo.sh` as permission to execute :

``` shell
chmod +x ./meteo.sh
```

Execute simply by start the script

```shell
./meteo.sh -w -G -f meteo_filtered_data_v1.csv
```

Compiled binary will be stored in `bin`

Output files will be in `out`

### Help

```shell
Help : Meteo.sh a script to filter, sort and plot weather data.

    Processed data will sent in the out directory.

    Usage : meteo.sh [OPTIONS] -f [FILE]
    Options :
    --t1 : Sort station number. output min, max and average temperature
    --t2 : Sort by date. output average temperature
    --t3 : Sort by date then station number. output temperature
    --p1 : Sort station number. output min, max and average pressure
    --p2 : Sort by date. output average pressure
    --p3 : Sort by date then station number. output pressure
    -d [start] [end] : Begining and end date
    -w : Sort by station number (increasing). output wind average wind orientation for each station
    -h : Sort by altitude (decreasing). output station number, altitude
    -m : Sort by max humidity (decreasing). output station number, max humidity
    -F : Select French stations
    -G : Select Guyane stations
    -S : Select Saint-Pierre et Miquelon stations
    -A : Select Antilles stations
    -O : Select Indian Ocean stations
    -Q : Select Antarctica stations
    -f <file> : Input file
    --tab : Use a table to sort
    --abr : Use a binary search tree to sort (not implemented on -t3 and -p3)
    --avl : Use an AVL tree to sort (curently not implemented)
    --help : Display this help

    -v : verbose level (0 = no output, 1 = errors, 2 = warnings, 3 = info, 4 = debug)
```

## Limitation

Due to lack of time and complexity of the AVL trees, we did not have enougth time to implement it.


## Screenshots

__Wind Plot__

![wind plot](doc/wind_plot.png)

__Height Plot__

![height plot](doc/height_plot.png)

__Pressure plot 1__

![Pressure plot 1](out/pressure_mode1_plot.png)

__Temperature plot 2__

![Temperature plot 2](out/temperature_mode2_plot.png)
