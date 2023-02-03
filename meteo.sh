#bin/sh

# Define arguments
SHORT=n,w,h,m,d,F,G,S,A,O,Q,f:,
LONG=t1,t2,t3,p1,p2,p3,tab,abr,avl,help
OPTS=$(getopt --alternative --name weather --options $SHORT --longoptions $LONG -- "$@")

#Set environement for getopt
eval set -- "$OPTS"

# Set default values
t1=false
t2=false
t3=false
p1=false
p2=false
p3=false
w=false
h=false
m=false
d1=""
d2=""

geo="ALL"
file=""
sortAlgo="" # 0 = table, 1 = binary search tree, 2 = AVL tree

check_geo() {
    if [ "$geo" != "ALL" ]; then
        echo "You can only select one geographical area."
        exit 1
    fi
}

check_algo() {
    if [ "$sortAlgo" != "" ]; then
        echo "You can only select one sorting algorithm."
        exit 1
    fi
}

help() {
    echo " Help : Meteo.sh a script to filter, sort and plot weather data.

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
    "
}

while :; do
    case "$1" in
    --t1) # Sort station number. output min, max and average temperature
        t1=true
        shift 1
        ;;
    --t2) # Sort by date. output average temperature
        t2=true
        shift 1
        ;;
    --t3) # Sort by date then station number. output temperature
        t3=true
        shift 1
        ;;
    --p1) # Sort station number. output min, max and average pressure
        p1=true
        shift 1
        ;;
    --p2) # Sort by date. output average pressure
        p2=true
        shift 1
        ;;
    --p3) # Sort by date then station number. output pressure
        p3=true
        shift 1
        ;;
    -d) # Begining and end date
        d1="$2"
        d2="$3"
        shift 3
        ;;
    -w) # Sort by station number (increasing). output wind average wind orientation for each station
        w=true
        shift 1
        ;;
    -h) # Sort by altitude (decreasing). output station number, altitude
        h=true
        shift 1
        ;;
    -m) # Sort by max humidity (decreasing). output station number, max humidity
        m=true
        shift 1
        ;;
    -F) # Select French stations
        check_geo
        geo="F"
        shift 1
        ;;
    -G) # Select Guyane stations
        check_geo
        geo="G"
        shift 1
        ;;
    -S) # Select Saint-Pierre et Miquelon stations
        check_geo
        geo="S"
        shift 1
        ;;
    -A) # Select Antilles stations
        check_geo
        geo="A"
        shift 1
        ;;
    -O) # Select Indian Ocean stations
        check_geo
        geo="O"
        shift 1
        ;;
    -Q) # Select Antarctica stations
        check_geo
        geo="Q"
        shift 1
        ;;
    -f) # Input file
        file="$2"
        shift 2
        ;;
    --tab) # Use a table to sort
        check_algo
        sortAlgo="--tab"
        shift 1
        ;;
    --abr) # Use a binary search tree to sort
        check_algo
        sortAlgo="--abr"
        shift 1
        ;;
    --avl) # Use an AVL tree to sort
        check_algo
        sortAlgo="--avl"
        shift 1
        ;;
    --help) # TODO : help
        "This is a weather script"
        exit 2
        ;;
    --)
        shift
        break
        ;;
    *)
        echo "Unexpected option: $1"
        exit 1
        ;;
    esac
done

# Check if output directory exists
if [ ! -d "out" ]; then
    echo "out directory does not exist. Creating it."
    mkdir out
fi

# Checkl if tmp directory exists
if [ ! -d "tmp" ]; then
    echo "tmp directory does not exist. Creating it."
    mkdir tmp
fi

# Checkl if tmp directory exists
if [ ! -d "bin" ]; then
    echo "bin directory does not exist. Creating it."
    mkdir bin
fi

# Check if input file exists
if [ ! -f "$file" ]; then
    echo "Input file does not exist."
    exit 1
fi

# Check if meteo is compiled
if [ ! -f "bin/meteo" ]; then
    echo "meteo executable does not exist. Compiling it."
    make
fi

find_column_number() {
    # $1 : column name
    # Extract the first line if the file
    # Split it by column separator into lines
    # Prefix each line with its number
    # Find the line with the column name
    # Extract the number of the line
    out=$(head "$file" -n 1 | sed "s/;/\n/g" | awk '{printf("%d %s\n", NR, $0)}' | grep "$1" | head -n1 | cut -d " " -f1)
    echo "$out"
}

# Filter out row outside the date range
if [ "$d1" != "" ] && [ "$d2" != "" ]; then
    echo "Filtering dates"
    # Find the number of the column containing the date
    date=$(find_column_number "Date")
    # Filter out rows outside the date range
    awk 'BEGIN { FS = ";"; OFS = ";" } ; {if (NR==1) {print}} { if ($'"$date"' >= "'"$d1"'" && $'"$date"' <= "'"$d2"'") print $0 }' "$file" >tmp.csv
    # Save to temporary file
    file="tmp.csv"
fi

# Filter out rows outside the geographical area
if [ "$geo" != "ALL" ]; then
    echo "Filtering geographical area"
    # Find the number of the column containing the geographical area
    geoCol=$(find_column_number "communes (code)")

    # For used to be able to break out of the if and only execute one of the two awk
    for _ in once; do
        # Set the regex to match the geographical area
        regex=".*"
        case "$geo" in
        F)
            # Match every code not strating with 971, 973, 974 or 975
            regex="[0-9]{2}[026-9][0-9]{2}"
            ;;
        A)
            # Match Antille code (971)
            regex="971[0-9]{2}"
            ;;
        G)
            # Match Guyane code (973)
            regex="973[0-9]{2}"
            ;;
        O)
            # Match Indian Ocean code (974)
            regex="974[0-9]{2}"
            ;;
        S)
            # Match Saint-Pierre et Miquelon code (975)
            regex="975[0-9]{2}"
            ;;
        Q)
            # Match Antarctica code (nothing)
            awk -e 'BEGIN { FS = ";"; OFS = ";" } ; {if (NR==1) {print}} { if ($'"$geoCol"' == "") print $0 }' "$file" >tmp/tmp.csv
            break
            ;;
        esac

        # Filter out rows outside the geographical area
        awk -e 'BEGIN { FS = ";"; OFS = ";" } ; {if (NR==1) {print}} { if ($'"$geoCol"' ~ /'"$regex"'/) print $0 }' "$file" >tmp/tmp.csv
    done
    # Save to temporary file
    file="tmp/tmp.csv"
fi

# -t1
if [ "$t1" = true ]; then
    echo "Filtering Temperatures"
    # Find the number of the column containing the station ID
    ID=$(find_column_number "ID OMM station")
    # Find the number of the column containing the temperature
    T=$(find_column_number "Température")
    awk 'BEGIN { FS = ";"; OFS = ";" } ; { print $'"$ID"', $'"$T"' }' "$file" >tmp/t1.csv
    ./bin/meteo -f tmp/t1.csv -o out/t1.csv -t 1 "$sortAlgo"
    gnuplot -e "filename='out/t1.csv'" gnuplot/temp_mode1.plg

fi

# -t2
if [ "$t2" = true ]; then
    echo "Filtering Temperatures"
    # Find the number of the column containing the Date
    D=$(find_column_number "Date")
    # Find the number of the column containing the temperature
    T=$(find_column_number "Température")
    awk 'BEGIN { FS = ";"; OFS = ";" } ; { print $'"$D"', $'"$T"' }' "$file" >tmp/t2.csv
    ./bin/meteo -f tmp/t2.csv -o out/t2.csv -t 2 "$sortAlgo"
    gnuplot -e "filename='out/t2.csv'" gnuplot/temp_mode2.plg

fi

# -t3
if [ "$t3" = true ]; then
    echo "Filtering Temperatures"
    # Find the number of the column containing the station ID
    ID=$(find_column_number "ID OMM station")
    # Find the number of the column containing the Date
    D=$(find_column_number "Date")
    # Find the number of the column containing the temperature
    T=$(find_column_number "Température")
    awk 'BEGIN { FS = ";"; OFS = ";" } ; { print $'"$ID"', $'"$D"', $'"$T"' }' "$file" >tmp/t3.csv
    ./bin/meteo -f tmp/t3.csv -o out/t3.csv -t 3 "$sortAlgo"
    gnuplot -e "filename='out/t3.csv'" gnuplot/temp_mode3.plg
fi

# -p1
if [ "$p1" = true ]; then
    echo "Filtering Precipitations"
    # Find the number of the column containing the station ID
    ID=$(find_column_number "ID OMM station")
    # Find the number of the column containing the temperature
    P=$(find_column_number "Pression station")
    awk 'BEGIN { FS = ";"; OFS = ";" } ; { print $'"$ID"', $'"$P"' }' "$file" >tmp/p1.csv
    ./bin/meteo -f tmp/p1.csv -o out/p1.csv -p 1 "$sortAlgo"
    gnuplot -e "filename='out/p1.csv'" gnuplot/pressure_mode1.plg
fi

# -p2
if [ "$p2" = true ]; then
    echo "Filtering Precipitations"
    # Find the number of the column containing the Date
    D=$(find_column_number "Date")
    # Find the number of the column containing the pressure
    P=$(find_column_number "Pression station")
    awk 'BEGIN { FS = ";"; OFS = ";" } ; { print $'"$D"', $'"$P"' }' "$file" >tmp/p2.csv
    ./bin/meteo -f tmp/p2.csv -o out/p2.csv -p 2 "$sortAlgo"
    gnuplot -e "filename='out/p2.csv'" gnuplot/pressure_mode2.plg
fi

# -p3
if [ "$p3" = true ]; then
    echo "Filtering Precipitations"
    # Find the number of the column containing the station ID
    ID=$(find_column_number "ID OMM station")
    # Find the number of the column containing the Date
    D=$(find_column_number "Date")
    # Find the number of the column containing the pressure
    P=$(find_column_number "Pression station")
    awk 'BEGIN { FS = ";"; OFS = ";" } ; { print $'"$ID"', $'"$D"', $'"$P"' }' "$file" >tmp/p3.csv
    ./bin/meteo -f tmp/p3.csv -o out/p3.csv -p 3 "$sortAlgo"
    gnuplot -e "filename='out/p3.csv'" gnuplot/pressure_mode3.plg
fi

# -w
if [ "$w" = true ]; then
    echo "Filtering Winds"
    # Find the number of the column containing the station ID
    ID=$(find_column_number "ID OMM station")
    # Find the number of the column containing the wind direction
    WD=$(find_column_number "Direction du vent moyen 10 mn")
    # Find the number of the column containing the wind speed
    WS=$(find_column_number "Vitesse du vent moyen 10 mn")
    # Find the number of the column containing coordinates
    CO=$(find_column_number "Coordonnees")

    awk 'BEGIN { FS = ";"; OFS = ";" } ; { print $'"$ID"', $'"$WD"', $'"$WS"', $'"$CO"' }' "$file" >tmp/w.csv
    ./bin/meteo -f tmp/w.csv -o out/w.csv -w "$sortAlgo"
    gnuplot -e "filename='out/w.csv'" gnuplot/wind.plg
fi

# -h
if [ "$h" = true ]; then
    echo "Filtering Altitudes"
    # Find the number of the column containing the station ID
    ID=$(find_column_number "ID OMM station")
    # Find the number of the column containing the altitude
    H=$(find_column_number "Altitude")
    # Find the number of the column containing coordinates
    CO=$(find_column_number "Coordonnees")
    awk 'BEGIN { FS = ";"; OFS = ";" } ; { print $'"$ID"', $'"$H"', $'"$CO"' }' "$file" >tmp/h.csv
    ./bin/meteo -f tmp/h.csv -o out/h.csv -h -r "$sortAlgo"
    gnuplot -e "filename='out/h.csv'" gnuplot/height.plg
fi

# -m
if [ "$m" = true ]; then
    echo "Filtering Humidity"
    # Find the number of the column containing the station ID
    ID=$(find_column_number "ID OMM station")
    # Find the number of the column containing the humidity
    M=$(find_column_number "Humidité")
    # Find the number of the column containing coordinates
    CO=$(find_column_number "Coordonnees")
    awk 'BEGIN { FS = ";"; OFS = ";" } ; { print $'"$ID"', $'"$M"', $'"$CO"' }' "$file" >tmp/m.csv
    ./bin/meteo -f tmp/m.csv -o out/m.csv -m -r "$sortAlgo"
    gnuplot -e "filename='out/m.csv'" gnuplot/humidity.plg
fi
