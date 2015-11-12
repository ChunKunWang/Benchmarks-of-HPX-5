#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "hpx/hpx.h"
#include <omp.h>

static void _usage(FILE *f, int error) {
	fprintf(f, "Usage: fibonacci [options] NUMBER\n"
			"\t-h, show help\n");
	hpx_print_help();
	fflush(f);
	exit(error);
}

static hpx_action_t _fib      = 0;
static hpx_action_t _fib_main = 0;

void do_omp_task(int fn)
{
	int count = 0;
	
	#pragma omp parallel
	#pragma omp single
	{
	for(int i = 0; i < 10000; i++) {
		#pragma omp task
		count++;
	}
	#pragma omp taskwait
	}
	#pragma omp barrier
	printf("fn[%d]: %d\n", fn, count);
}

static int _fib_action(int *args, size_t size) {
	int n = *args;

	if (n < 2)
		HPX_THREAD_CONTINUE(n);

	hpx_addr_t peers[] = {
		HPX_HERE,
		HPX_HERE
	};

	int ns[] = {
		n - 1,
		n - 2
	};

	hpx_addr_t futures[] = {
		hpx_lco_future_new(sizeof(int)),
		hpx_lco_future_new(sizeof(int))
	};

	int fns[] = {
		0,
		0
	};

	void *addrs[] = {
		&fns[0],
		&fns[1]
	};

	int sizes[] = {
		sizeof(int),
		sizeof(int)
	};

	hpx_call(peers[0], _fib, futures[0], &ns[0], sizeof(int));
	hpx_call(peers[1], _fib, futures[1], &ns[1], sizeof(int));
	hpx_lco_get_all(2, futures, sizes, addrs, NULL);
	hpx_lco_delete(futures[0], HPX_NULL);
	hpx_lco_delete(futures[1], HPX_NULL);

	int fn = fns[0] + fns[1];

	do_omp_task(fn);

	HPX_THREAD_CONTINUE(fn);
	return HPX_SUCCESS;
}

static int _fib_main_action(int *args, size_t size) {
	int n = *args;
	int fn = 0;                                   // fib result
	printf("fib(%d)=", n); fflush(stdout);
	hpx_time_t now = hpx_time_now();
	hpx_call_sync(HPX_HERE, _fib, &fn, sizeof(fn), &n, sizeof(n));
	double elapsed = hpx_time_elapsed_ms(now)/1e3;

	printf("%d\n", fn);
	printf("seconds: %.7f\n", elapsed);
	printf("localities: %d\n", HPX_LOCALITIES);
	printf("threads/locality: %d\n", HPX_THREADS);
	hpx_exit(HPX_SUCCESS);
}

int main(int argc, char *argv[]) 
{
	if (hpx_init(&argc, &argv) != 0) {
		fprintf(stderr, "HPX: failed to initialize.\n");
		return -1;
	}

	// parse the command line
	int opt = 0;
	while ((opt = getopt(argc, argv, "h?")) != -1) {
		switch (opt) {
			case 'h':
				_usage(stdout, EXIT_SUCCESS);
			case '?':
			default:
				_usage(stderr, EXIT_FAILURE);
		}
	}

	argc -= optind;
	argv += optind;

	int n = 0;
	switch (argc) {
		case 0:
			fprintf(stderr, "\nMissing fib number.\n"); // fall through
		default:
			_usage(stderr, EXIT_FAILURE);
		case 1:
			n = atoi(argv[0]);
			break;
	}

	// register the fib action
	HPX_REGISTER_ACTION(HPX_DEFAULT, HPX_MARSHALLED, _fib, _fib_action,
			HPX_POINTER, HPX_SIZE_T);
	HPX_REGISTER_ACTION(HPX_DEFAULT, HPX_MARSHALLED, _fib_main, _fib_main_action,
			HPX_POINTER, HPX_SIZE_T);

	// run the main action
	int e = hpx_run(&_fib_main, &n, sizeof(n));
	hpx_finalize();
	return e;
}

