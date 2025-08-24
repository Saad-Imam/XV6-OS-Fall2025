#include "kernel/types.h"
#include "user/user.h"

// matrix dimensions
#define N 4

int matrix_a[N][N] = {
    {1, 2, 3, 4},
    {5, 6, 7, 8},
    {9, 10, 11, 12},
    {13, 14, 15, 16}
};

int matrix_b[N][N] = {
    {16, 15, 14, 13},
    {12, 11, 10, 9},
    {8, 7, 6, 5},
    {4, 3, 2, 1}
};

int result_matrix[N][N];

void print_matrix(int matrix[N][N]) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            printf("%d ", matrix[i][j]);
        }
        printf("\n");
    }
}

int main(void) {

    int child_to_parent[2];
    pipe(child_to_parent);

    int pid = fork();

    if (pid < 0) {
        fprintf(2, "fork failed\n");
        exit(1);
    }

    if (pid == 0) { 
        close(child_to_parent[0]); // close unused read end of the pipe.

        // child's task: compute the second half of the matrix.
        int start_row = N / 2;
        int rows_to_process = N / 2;
        int partial_result[rows_to_process][N];

        for (int i = 0; i < rows_to_process; i++) {
            for (int j = 0; j < N; j++) {
                partial_result[i][j] = 0; //initializing matrix
                for (int k = 0; k < N; k++) {
                    partial_result[i][j] += matrix_a[start_row + i][k] * matrix_b[k][j];
                }
            }
        }

        // send the partial result back to the parent.
        write(child_to_parent[1], partial_result, sizeof(partial_result));

        close(child_to_parent[1]);
        exit(0);

    } else { 
        close(child_to_parent[1]); // close unused write end of the pipe.

        // parent calculates the first half.
        for (int i = 0; i < N / 2; i++) {
            for (int j = 0; j < N; j++) {
                result_matrix[i][j] = 0;
                for (int k = 0; k < N; k++) {
                    result_matrix[i][j] += matrix_a[i][k] * matrix_b[k][j];
                }
            }
        }

        // wait for the child to finish and read its part of the result
        read(child_to_parent[0], &result_matrix[N / 2], (N / 2) * N * sizeof(int));
        wait(0); //waiting for child to terminate

        printf("Result Matrix:\n");
        print_matrix(result_matrix);

        close(child_to_parent[0]);
    }

    exit(0);
}
