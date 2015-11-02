/**********************************************************************************************/
/*  This program is part of the Barcelona OpenMP Tasks Suite                                  */
/*  Copyright (C) 2009 Barcelona Supercomputing Center - Centro Nacional de Supercomputacion  */
/*  Copyright (C) 2009 Universitat Politecnica de Catalunya                                   */
/*                                                                                            */
/*  This program is free software; you can redistribute it and/or modify                      */
/*  it under the terms of the GNU General Public License as published by                      */
/*  the Free Software Foundation; either version 2 of the License, or                         */
/*  (at your option) any later version.                                                       */
/*                                                                                            */
/*  This program is distributed in the hope that it will be useful,                           */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of                            */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                             */
/*  GNU General Public License for more details.                                              */
/*                                                                                            */
/*  You should have received a copy of the GNU General Public License                         */
/*  along with this program; if not, write to the Free Software                               */
/*  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA            */
/**********************************************************************************************/

#include <stdio.h> 
#include <unistd.h>
#include <omp.h>

#define FIB_RESULTS_PRE 41
int fib_results[FIB_RESULTS_PRE] = {0,1,1,2,3,5,8,13,21,34,55,89,144,233,377,610,987,1597,2584,4181,6765,10946,17711,28657,46368,75025,121393,196418,317811,514229,832040,1346269,2178309,3524578,5702887,9227465,14930352,24157817,39088169,63245986,102334155};

static int par_res, seq_res;

struct thread_data{
	int n;
	int extra[1023];
};

static void _usage(FILE *f, int error) {
  fprintf(f, "Usage: fibonacci NUMBER\n");
  fflush(f);
}

int fib (struct thread_data temp)
{
	int x, y;
	struct thread_data n1, n2;

	//temp.extra[2]=1;
	if (temp.n < 2) return temp.n;
	n1.n = temp.n - 1;
	n2.n = temp.n - 2;

#pragma omp task untied shared(x) firstprivate(n1)
	x = fib(n1);
#pragma omp task untied shared(y) firstprivate(n1)
	y = fib(n2);

#pragma omp taskwait
	return x + y;
}

void main(int argc, char *argv[])
{
	// parse the command line
	int opt = 0;
	while ((opt = getopt(argc, argv, "h?")) != -1) {
		switch (opt) {
			case 'h':
				_usage(stdout, 1);
			case '?':
			default:
				_usage(stderr, 0);
		}
	}

	argc -= optind;
	argv += optind;

	struct thread_data input;
	input.n = 1;
	switch (argc) {
		case 0:
			fprintf(stderr, "\nMissing fib number.\n"); // fall through
		default:
			_usage(stderr, 0);
		case 1:
			input.n = atoi(argv[0]);
			break;
	}

#pragma omp parallel
#pragma omp single
{ 
#ifdef _OPENMP
	double start = omp_get_wtime();;
#endif
	par_res = fib(input);

#ifdef _OPENMP
	double time = omp_get_wtime() - start;
	printf("OpenMP Work took %f sec.\n", time);
#endif
}
	printf("Fibonacci result for %d is %d\n",input.n ,par_res);

}

