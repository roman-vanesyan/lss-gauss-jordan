#include <stdio.h>
#include <math.h>

#include "lss.h"

/* Exit codes.
 * ============================================================================ */


static const int _OK_CODE = 0;
static const int _UNRESOLVED_SYSTEM_ERR_CODE = 1;


/* Utilities.
 * ============================================================================ */


/*
 * Swaps two elements.
 */
static
inline
void _swap(double *left, double *right)
{
    double temp = *left;
    *left = *right;
    *right = temp;
}


/* Logging
 * ============================================================================ */


static
inline
void _logdebug(const char *message)
{
    if (debug_mode)
    {
        printf("[DEBUG] ");
        printf("%s", message);
        printf("\n");
    }
}

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

/*
 * Prints matrix of size NxN.
 *
 * Assumes that matrix is stored as 1d array.
 */
static
inline
void _print_matrix(int n, double *A, double *tmp)
{
    for (int i = 0; i < n; i++)
    {
        printf("x%-19d", (int)tmp[i] + 1);
    }

    printf("\n");

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

static
inline
void _print_result(int n, double *B)
{
    for (int i = 0; i < n; i++)
    {
        printf("%lf\n", B[i]);
    }

    printf("\n");
}


/* SOLVER
 * ============================================================================ */

/*
 * Solve linear system of equations using Gauss-Jordan method.
 * see ЧИСЛЕННЫЕ МЕТОДЫ А. А. Самарский, А. В. Гулин (1989). §3 for details.
 */
int lss(int n, double *A, double *B, double *X, double *tmp)
{
    int i, j, k;
    int max_index;

    // Initialize `tmp` array to store position of each `x`.
    for (i = 0; i < n; i++)
    {
        tmp[i] = i;
    }

    for (i = 0; i < n; i++)
    {
        if (debug_mode)
        {
            printf("\n\nResolving row: %d\n", i);
            printf("================================================================================\n\n");
        }

        max_index = i;

        if (debug_mode)
        {
            _logdebug("Matrix before modification");
            _print_matrix(n, A, tmp);

            _logdebug("Result before modification");
            _print_result(n, B);
        }

        // Find main element of row (assuming main element is picking by rule: max(|Ai1|, |Ai2|, ..., |Ain|)).
        for (j = max_index + 1; j < n; j++)
        {
            if (fabs(A[i * n + j]) > fabs(A[i * n + max_index]))
            {
                max_index = j;
            }
        }

        if (debug_mode)
        {
            char message[1028];

            sprintf(message, "Found main element %lf at position %d in row %d", A[i * n + max_index], max_index + 1,
                    i + 1);
            _logdebug(message);
        }

        if (fabs(A[i * n + max_index]) < EPS)
        {
            // Checking for row degradation => checking for system of equations incompatibility.
            if (fabs(B[i]) > EPS)
            {
                _logerror("No result found for given linear system of equations!");

                return _UNRESOLVED_SYSTEM_ERR_CODE;
            }

            _logdebug("Linear relationship, skipping...\n");

            // Linear relationship.
            continue;
        }

        if (max_index != i)
        {
            if (debug_mode)
            {
                char message[255];

                // TODO: meaning of non-null?
                sprintf(message,
                        "Swapping column containing main element (position %d) "
                        "with first non-null column (position %d)",
                        max_index + 1, i + 1);
                _logdebug(message);
            }

            for (j = 0; j < n; j++)
            {
                _swap(&A[j * n + i], &A[j * n + max_index]);
            }

            _swap(&tmp[i], &tmp[max_index]);
            max_index = i;

            if (debug_mode)
            {
                _logdebug("Matrix after swapping");
                _print_matrix(n, A, tmp);
            }
        }

        double normalizer = A[i * n + max_index];

        // Normalize row containing main element.
        for (j = max_index; j < n; j++)
        {
            A[i * n + j] /= normalizer;
        }

        B[max_index] /= normalizer;

        if (debug_mode)
        {
            _logdebug("Matrix after normalization");
            _print_matrix(n, A, tmp);
            _logdebug("Result after normalization");
            _print_result(n, B);
        }

        for (j = 0; j < n; j++)
        {
            if (j == i)
            {
                continue;
            }

            double coef = A[j * n + max_index];

            for (k = max_index; k < n; k++)
            {
                A[j * n + k] -= coef * A[i * n + k];

                if (fabs(A[j * n + k]) < EPS)
                {
                    A[j * n + k] = 0;
                }
            }

            B[j] -= coef * B[i];

            if (fabs(B[j]) < EPS)
            {
                B[j] = 0;
            }
        }

        if (debug_mode)
        {
            _logdebug("Matrix after modification");
            _print_matrix(n, A, tmp);

            _logdebug("Result after modification");
            _print_result(n, B);
        }
    }

    for (i = 0; i < n; i++)
    {
        X[(int) tmp[i]] = B[i];
    }

    return _OK_CODE;
}

size_t lss_memsize(int n)
{
    return sizeof(double) * n;
}
