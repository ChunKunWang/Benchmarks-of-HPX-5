#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <alloca.h>
#include <time.h>
//#include "bots.h"
#include <omp.h>


/* Checking information */

static int solutions[] = {
	1,
	0,
	0,
	2,
	10, /* 5 */
	4,
	40,
	92,
	352,
	724, /* 10 */
	2680,
	14200,
	73712,
	365596,
	2279184, /* 15 */
	14772512,
};

int mycount=0;
#pragma omp threadprivate(mycount)

int total_count;

int ok(int n, char *a)
{
	int i, j;
	char p, q;

	for (i = 0; i < n; i++) {
		p = a[i];

		for (j = i + 1; j < n; j++) {
			q = a[j];
			if (q == p || q == p - (j - i) || q == p + (j - i))
				return 0;
		}
	}
	return 1;
}

void nqueens_ser (int n, int j, char *a, int *solutions)
{
	int i,res;

	if (n == j) {
		/* good solution, count it */
		*solutions = 1;
		return;
	}

	*solutions = 0;

	/* try each possible position for queen <j> */
	for (i = 0; i < n; i++) {
		{
			/* allocate a temporary array and copy <a> into it */
			a[j] = i;
			if (ok(j + 1, a)) {
				nqueens_ser(n, j + 1, a,&res);
				*solutions += res;
			}
		}
	}
}

void nqueens(int n, int j, char *a, int *solutions, int depth)
{
	int i;
	int *csols;


	if (n == j) {
		/* good solution, count it */
		*solutions = 1;
		mycount++;
		return;
	}

	*solutions = 0;
	csols = alloca(n*sizeof(int));
	memset(csols,0,n*sizeof(int));

	/* try each possible position for queen <j> */
	for (i = 0; i < n; i++) {
#pragma omp task untied
		{
			/* allocate a temporary array and copy <a> into it */
			char * b = alloca((j + 1) * sizeof(char));
			memcpy(b, a, j * sizeof(char));
			b[j] = i;
			if (ok(j + 1, b))
				nqueens(n, j + 1, b,&csols[i],depth);
		}
	}

#pragma omp taskwait

	for ( i = 0; i < n; i++) *solutions += csols[i];
}

void find_queens (int size)
{
	total_count=0;

	printf("%d-Queens: ", size);
#pragma omp parallel
	{
#pragma omp single
		{
			char *a;

			a = alloca(size * sizeof(char));
#ifdef _OPENMP
			double start = omp_get_wtime();;
#else
			clock_t start = clock();
#endif
			nqueens(size, 0, a, &total_count,0);
			//printf("completed!\n");
#ifdef _OPENMP
			double time = omp_get_wtime() - start;
			printf("%f sec\n", time);
#else
			double end = clock();
			float time = (float)(end-start) / CLOCKS_PER_SEC;
			printf("%f sec\n", time);
#endif
		}
#pragma omp atomic
		total_count += mycount;
	}
}

int verify_queens (int size)
{
	total_count = total_count / 2;
	//printf( "total_count = %d; solution = %d\n", total_count, solutions[size-1] );
	if ( total_count == solutions[size-1]) {
		//printf( "RESULT_SUCCESSFUL!\n" );
		return 1;
	}

	printf( "RESULT_UNSUCCESSFUL!\n" );

	return 2;
}

int main(int n, char **argv)
{
	if (n <= 1 || (n = atoi(argv[1])) <= 0) n = 8;
	if (n>16) {
		printf("Max size is 16.\n");
		n = 16;	
	}
	//printf("%d-nqueens:\n", n);

	find_queens(n);

	verify_queens(n);

	return 0;
}



