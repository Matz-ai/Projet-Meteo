

#include "main.h"

void check_algo_exclu()
{
    if (meteoconf.sort_algo != ALGO_NONE)
    {
        fprintf(stderr, "Only one algorithm can be used at a time");
        exit(EXIT_ARG);
    }
}

void check_op_exclu()
{
    if (meteoconf.operation != OP_NONE)
    {
        fprintf(stderr, "Only one operation can be used at a time");
        exit(EXIT_ARG);
    }
}

int main(int argc, char *const *argv)
{

    // getopt variable
    int c;

    while (1)
    {
        int option_index = 0;

        // define long options
        struct option long_options[] =
            {
                {"abr", no_argument, 0, 'x'},
                {"tab", no_argument, 0, 'y'},
                {"avl", required_argument, 0, 'z'},
                {0, 0, 0, 0}};

        // define short options
        const char *short_options = "t:p:f:o:rwhmv:";

        // get option
        c = getopt_long(argc, argv, short_options, long_options, &option_index);

        // check if option is valid
        if (c == -1)
            break;

        // check option
        switch (c)
        {
        case 't':
            check_op_exclu();
            if (strcmp(optarg, "1") == 0)
            {
                meteoconf.operation = OP_T1;
            }
            else if (strcmp(optarg, "2") == 0)
            {
                meteoconf.operation = OP_T2;
            }
            else if (strcmp(optarg, "3") == 0)
            {
                meteoconf.operation = OP_T3;
            }
            else
            {
                printf("Invalide argument for -t option, use -h for help");
                exit(EXIT_ARG);
            }
            break;
        case 'p':
            check_op_exclu();
            if (strcmp(optarg, "1") == 0)
            {
                meteoconf.operation = OP_P1;
            }
            else if (strcmp(optarg, "2") == 0)
            {
                meteoconf.operation = OP_P2;
            }
            else if (strcmp(optarg, "3") == 0)
            {
                meteoconf.operation = OP_P3;
            }
            else
            {
                printf("Invalide argument for -p option, use -h for help");
                exit(EXIT_ARG);
            }
            break;
        case 'w':
            check_op_exclu();
            meteoconf.operation = OP_W;
            break;
        case 'h':
            check_op_exclu();
            meteoconf.operation = OP_H;
            break;
        case 'm':
            check_op_exclu();
            meteoconf.operation = OP_M;
            break;
        // Use abr
        case 'x':
            check_algo_exclu();
            meteoconf.sort_algo = ALGO_ABR;
            break;
        // Use tab
        case 'y':
            check_algo_exclu();
            meteoconf.sort_algo = ALGO_TAB;
            break;
        // Use avl
        case 'z':
            check_algo_exclu();
            meteoconf.sort_algo = ALGO_AVL;
            break;
        // Input file
        case 'f':
            // Check file exist
            if (access(optarg, F_OK) == -1)
            {
                printf("Input file does not exist");
                exit(EXIT_IN_FILE);
            }
            meteoconf.input_file_name = optarg;
            break;
        // Output file
        case 'o':
            meteoconf.output_file_name = optarg;
            break;
        // Reverse
        case 'r':
            meteoconf.reverse = true;
            break;
        case 'v':
            meteoconf.verbose = atoi(optarg);
            break;
        case '?':
            break;
        default:
            printf("?? getopt returned character code 0%o ??\n", c);
            exit(EXIT_ARG);
        }
    }

    // Check if input file is set
    if (meteoconf.input_file_name == NULL)
    {
        printf("Input file is not set, use -h for help");
        exit(EXIT_ARG);
    }
    // Check if output file is set
    if (meteoconf.output_file_name == NULL)
    {
        printf("Output file is not set, use -h for help");
        exit(EXIT_ARG);
    }
    // Check if operation is set
    if (meteoconf.operation == OP_NONE)
    {
        printf("Operation is not set, use -h for help");
        exit(EXIT_ARG);
    }

    if (meteoconf.sort_algo == ALGO_NONE)
    {
        meteoconf.sort_algo = ALGO_TAB;
    }

    execute();

    return (EXIT_SUCCESS);
}

void execute()
{
    switch (meteoconf.operation)
    {
    case OP_T1:
        printf("Sorting Temperature...\n");
        t1();
        break;
    case OP_T2:
        printf("Sorting Temperature...\n");
        t2();
        break;
    case OP_T3:
        printf("Sorting Temperature...\n");
        t3_tab();
        break;
    case OP_P1:
        printf("Sorting Pressure...\n");
        t1();
        break;
    case OP_P2:
        printf("Sorting Pressure...\n");
        t2();
        break;
    case OP_P3:
        printf("Sorting Pressure...\n");
        t3_tab();
        break;
    case OP_W:
        printf("Sorting wind...\n");
        wind();
        break;
    case OP_H:
        printf("Sorting height...\n");
        height();
        break;
    case OP_M:
        printf("Sorting moisture...\n");
        moisture();
        break;
    default:
        printf("No operation selected\n");
        exit(EXIT_ARG);
        break;
    }
    printf("Done\n");
}
