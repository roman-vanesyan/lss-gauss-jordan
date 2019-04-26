#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "lss.h"

/* Exit codes.
 * ============================================================================ */


const int _OK_CODE = 0;
const int _FILE_ERR_CODE = 12;
const int _NOT_ENOUGH_DATA_CODE = 13;
const int _CLI_ERR_CODE = 22;


/* Logging
 * ============================================================================ */


static
inline
void _logerror(const char *message)
{
    if (error_mode)
    {
        printf("[ERROR] ");
        printf("%s", message);
        printf("\n");
    }
}


/* String utilities.
 * ============================================================================ */


/*
 * Returns length of `str`; `str` is expected to be null-terminated string.
 *
 * \param str - String to process
 * \returns Length of the `str`.
 */
static
int _string_length(const char *str)
{
    int i = 0;

    for (; str[i] != 0; i++);

    return i;
}

/*
 * Compares if `left` and `right` strings are equal.
 *
 * \param left - String to compare
 * \param right - String to compare with
 * \returns `0` if strings are different, `1` if equal.
 */
static
int _string_equals(const char *left, const char *right)
{
    int llen = _string_length(left);
    int rlen = _string_length(right);

    /* Compare strings lengths to be equal. */
    if (llen - rlen != 0) return 0;

    for (int i = 0; i < llen; i++)
    {
        if (left[i] != right[i]) return 0;
    }

    return 1;
}

/*
 * Verifies whether `str` begins with a `beg`.
 *
 * \param str - String to verify
 * \param beg - String to compare beginning of `str` with
 * \returns `1` if starts with `beg`, otherwise `0`.
 */
int _string_starts_with(const char *str, const char *pattern)
{
    int slen = _string_length(str);
    int plen = _string_length(pattern);

    if (plen > slen) return 0;

    for (int i = 0; i < plen; i++)
    {
        if (str[i] != pattern[i]) return 0;
    }

    return 1;
}


/* I/O utilities
 * ============================================================================ */


/*
 * Closes `file`.
 *
 * \param file
 */
static
inline
void _file_close(FILE *file)
{
    if (file == NULL || fclose(file) != 0)
    {
        _logerror("Cannot close file!");
    }
}

/*
 * Reads `n`, `A` and `B` from the file at `path`.
 *
 * \param path - Path to file to read data (`n`, `A`, `B`) from
 * \param n - Dim of linear system
 * \param A - Matrix of coefficients
 * \param B - Vector of values
 * \returns Error code
 */
static
inline
int _read_input(const char *path, int *n, double **A, double **B)
{
    FILE *file = fopen(path, "r");
    int size;

    if (file == NULL)
    {
        if (error_mode)
        {
            char buffer[255];

            sprintf(buffer, "Cannot open %s for read", path);
            _logerror(buffer);
        }

        _file_close(file);

        return _FILE_ERR_CODE;
    }

    if (fscanf(file, "%d", &size) != 1)
    {
        _logerror("Not enough data to proceed!");
        _file_close(file);

        return _NOT_ENOUGH_DATA_CODE;
    }

    *n = size;

    /* Read input matrix of size NxN. */
    *A = (double *) malloc(size * size * sizeof(double));

    for (int i = 0; i < size * size; i++)
    {
        if (fscanf(file, "%lf", &(*A)[i]) != 1)
        {
            _logerror("Not enough data to proceed!");
            _file_close(file);

            return _NOT_ENOUGH_DATA_CODE;
        }
    }

    /* Read input vector of size N. */
    *B = (double *) malloc(size * sizeof(double));

    for (int i = 0; i < size; i++)
    {
        if (fscanf(file, "%lf", &(*B)[i]) != 1)
        {
            _logerror("Not enough data to proceed!");
            _file_close(file);

            return _NOT_ENOUGH_DATA_CODE;
        }
    }

    _file_close(file);

    return _OK_CODE;
}

/*
 * Writes `X` out to the file at `path`.
 *
 * \param path - Path to file
 * \param n - Length of result
 * \param X - result
 * \returns Error code
 */
static
int _write_output(const char *path, int n, const double *X)
{
    FILE *file = fopen(path, "w");

    if (file == NULL)
    {
        if (error_mode)
        {
            char buffer[255];

            sprintf(buffer, "Cannot open %s to write", path);
            _logerror(buffer);
        }

        return _FILE_ERR_CODE;
    }

    for (int i = 0; i < n; i++)
    {
        fprintf(file, "%1.9lf ", X[i]);
    }

    _file_close(file);

    return _OK_CODE;
}


/* CLI
 * ============================================================================ */


/*
 * Parses command line input flags.
 *
 * \param argc - Number of arguments
 * \param argv - Arguments to parse
 * \param help_flag - Help flag indicating that help message should be printed (mutable)
 * \param debug_flag - Debug flag indicating that program should start in verbose mode (mutable)
 * \param error_flag - Error flag indicating that program should print errors if occur (mutable)
 * \param matrix_flag - Flag indicating that program should print matrix (mutable)
 * \param trace_flag - Flag indicating that the program should track time (mutable)
 * \param input - Input file path (mutable)
 * \param output - Output file path (mutable)
 * \returns Error code; `0` if ok, otherwise positive integer.
 */
int parse_argv(int argc,
               const char **argv,
               int *help_flag,
               int *debug_flag,
               int *error_flag,
               int *matrix_flag,
               int *trace_flag,
               char **input,
               char **output)
{
    int is_input_set = 0;
    int is_output_set = 0;
    char *arg;

    /* The 0th argument is always program name. */
    for (int i = 1; i < argc; i++)
    {
        arg = (char *) argv[i];

        if (_string_starts_with(arg, "-"))
        {

            /* If help flag was passed, print help only and exit. */
            if (_string_equals(arg, "-h") || _string_equals(arg, "-?"))
            {
                *help_flag = 1;

                return _OK_CODE;
            }

            if (_string_equals(arg, "-d"))
            {
                *debug_flag = 1;
            } else if (_string_equals(arg, "-e"))
            {
                *error_flag = 1;
            } else if (_string_equals(arg, "-p"))
            {
                *matrix_flag = 1;
            } else if (_string_equals(arg, "-t"))
            {
                *trace_flag = 1;
            } else
            {
                if (error_mode)
                {
                    char buffer[255];

                    sprintf(buffer, "Unknown option is provided: %s", arg);
                    _logerror(buffer);
                }

                return _CLI_ERR_CODE;
            }
        } else
        {
            if (is_output_set)
            {
                _logerror("Too many positional arguments are provided!");

                return _CLI_ERR_CODE;
            }

            if (!is_input_set)
            {
                is_input_set = 1;
                *input = arg;
            } else
            {
                is_output_set = 1;
                *output = arg;
            }
        }
    }

    return _OK_CODE;
}

static
inline
void _print_matrix(int n, double *A)
{
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            printf("%-20lf", A[i * n + j]);
        }

        printf("\n");
    }

    printf("\n");
}

/*
 * Prints help message to stdout.
 */
static
inline
void _print_help()
{
    printf("Usage: lss [input] [output] [options]\n");
    printf("\n");
    printf("Options:\n");
    printf("  -d     print debug messages (Default: off)\n");
    printf("  -e     print errors (Default: off)\n");
    printf("  -p     print matrix (Default: off)\n");
    printf("  -t     print exec. time (Default: off)\n");
    printf("  -h, -? print this message (Default: off)\n");
    printf("\n");
    printf("Default input value is in.txt, default output value is out.txt\n");
}

int main(int argc, const char *argv[])
{
    int help_flag = 0;
    int debug_flag = 0;
    int error_flag = 0;
    int matrix_flag = 0;
    int trace_flag = 0;
    char *input = "in.txt";
    char *output = "out.txt";

    int code = parse_argv(argc,
                          argv,
                          &help_flag,
                          &debug_flag,
                          &error_flag,
                          &matrix_flag,
                          &trace_flag,
                          &input,
                          &output);

    if (code != _OK_CODE)
    {
        return code;
    }

    // Forward flags state to global vars.
    debug_mode = debug_flag;
    error_mode = error_flag;

    if (help_flag)
    {
        _print_help();

        return _OK_CODE;
    }

    int n;
    double *A = NULL;
    double *B = NULL;
    double *X = NULL;
    double *tmp = NULL;

    code = _read_input(input, &n, &A, &B);

    if (code != _OK_CODE) return code;

    X = (double *) malloc(sizeof(double) * n);
    tmp = (double *) malloc(lss_memsize(n));

    if (matrix_flag) _print_matrix(n, A);

    if (trace_flag)
    {
        time_t exec_time = clock();
        code = lss(n, A, B, X, tmp);
        double exec_sec = (double) (clock() - exec_time) / CLOCKS_PER_SEC;

        printf("Execution time: %lfsec.", exec_sec);
    } else
    {
        code = lss(n, A, B, X, tmp);
    }

    int wcode = _write_output(output, n, X);

    if (wcode != _OK_CODE)
    {
        return wcode;
    }

    free(A);
    free(B);
    free(X);
    free(tmp);

    A = NULL;
    B = NULL;
    X = NULL;
    tmp = NULL;

    return code;
}
