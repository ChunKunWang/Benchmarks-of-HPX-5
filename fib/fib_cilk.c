#include <stdio.h> 
#include <unistd.h>
#include <time.h>
#include <cilk/cilk.h>

#define FIB_RESULTS_PRE 41
int fib_results[FIB_RESULTS_PRE] = {0,1,1,2,3,5,8,13,21,34,55,89,144,233,377,610,987,1597,2584,4181,6765,10946,17711,28657,46368,75025,121393,196418,317811,514229,832040,1346269,2178309,3524578,5702887,9227465,14930352,24157817,39088169,63245986,102334155};

static int par_res, seq_res;

static void _usage(FILE *f, int error) {
  fprintf(f, "Usage: fibonacci NUMBER\n");
  fflush(f);
}

int fib (int n)
{
	int x, y;
	if (n < 2) return n;

	x = cilk_spawn fib(n - 1);
	y = cilk_spawn fib(n - 2);

	cilk_sync;
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

	int n = 1;
	switch (argc) {
		case 0:
			fprintf(stderr, "\nMissing fib number.\n"); // fall through
		default:
			_usage(stderr, 0);
		case 1:
			n = atoi(argv[0]);
			break;
	}

	clock_t start = clock();
	par_res = fib(n);
	clock_t end = clock();
	double duration = (double)(end - start) / CLOCKS_PER_SEC;
	printf("Cilk took %f sec\n", duration);

	printf("Fibonacci result for %d is %d\n",n ,par_res);

}

