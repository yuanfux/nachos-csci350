/* matmult.c
 *    Test program to do matrix multiplication on large arrays.
 *
 *    Intended to stress virtual memory system.
 *
 *    Ideally, we could read the matrices off of the file system,
 *	and store the result back to the file system!
 */

#include "syscall.h"

#define Dim 	20	/* sum total of the arrays doesn't fit in 
			 * physical memory 
			 */

int A[Dim][Dim];
int B[Dim][Dim];
int C[Dim][Dim];

int
main()
{
	int i, j, k;
	Write("In Matmult Test\n", sizeof("In Matmult Test\n"), ConsoleOutput);

	for (i = 0; i < Dim; i++)		/* first initialize the matrices */
		for (j = 0; j < Dim; j++) {
			A[i][j] = i;
			B[i][j] = j;
			C[i][j] = 0;
		}

	Write("After Matmult Initialization\n", sizeof("After Matmult Initialization\n"), ConsoleOutput);

	for (i = 0; i < Dim; i++) {		/* then multiply them together */
		for (j = 0; j < Dim; j++) {
			for (k = 0; k < Dim; k++) {
				/*
				Write("C[", sizeof("C["), ConsoleOutput);
				Printint(i);
				Write("][", sizeof("]["), ConsoleOutput);
				Printint(j);
				Write("], A[", sizeof("], A["), ConsoleOutput);
				Printint(i);
				Write("][", sizeof("]["), ConsoleOutput);
				Printint(k);
				Write("], B[", sizeof("], B["), ConsoleOutput);
				Printint(k);
				Write("][", sizeof("]["), ConsoleOutput);
				Printint(j);
				Write("]: ", sizeof("]: "), ConsoleOutput);
				*/
				/* Printint(C[i][j]); */
				/* Write(" ", sizeof(" "), ConsoleOutput); */
				/* Printint(A[i][k]); */
				/* Write(" ", sizeof(" "), ConsoleOutput); */
				/* Printint(B[k][j]); */
				/* Write(" ", sizeof(" "), ConsoleOutput); */
				/* Write("A * B = ", sizeof("A * B = "), ConsoleOutput); */
				/* Printint(A[i][k] * B[k][j]); */
				/* Write("\n", sizeof("\n"), ConsoleOutput); */
				C[i][j] += A[i][k] * B[k][j];
				/* Write("C[", sizeof("C["), ConsoleOutput); */
				/* Printint(i); */
				/* Write("][", sizeof("]["), ConsoleOutput); */
				/* Printint(j); */
				/* Write("]= ", sizeof("]= "), ConsoleOutput); */
				/* Printint(C[i][j]); */
				/* Write("\n", sizeof("\n"), ConsoleOutput); */
			}
		}
		Printint(C[i][Dim - 1]);
		Write("\n", sizeof("\n"), ConsoleOutput);
	}


	Write("Result of matrix mult test: ", sizeof("Result of matrix mult test: "), ConsoleOutput);
	Printint(C[Dim - 1][Dim - 1]);
	Write("\n", sizeof("\n"), ConsoleOutput);

	Exit(C[Dim - 1][Dim - 1]);		/* and then we're done */
}
