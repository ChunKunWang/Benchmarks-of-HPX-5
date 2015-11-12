#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include "hpx/hpx.h"


static hpx_action_t _Pi      = 0;
static hpx_action_t _Pi_main = 0;
static double PI_STEP;
double step, pi;

struct pi_data {
	int counter;
	double sum;
};

static void _usage(FILE *f, int error) {
	fprintf(f, "Usage: ./hpx-5 [options] NUMBER\n");
	//hpx_print_help();
	fflush(f);
	exit(error);
}

static int _Pi_action(void *args, size_t size)
{
	int counter;
	double sum;
	double x=0, result=0, result_2=0;
	struct pi_data *my_data;
	hpx_addr_t done = hpx_lco_future_new(sizeof(int));

	my_data = (struct pi_data *)args;
	counter = my_data->counter;
	sum = my_data->sum;

	if( counter < PI_STEP ) {
		x = (counter + 0.5) * step;
		result = sum + 4.0/(1.0+x*x);

		my_data->counter = counter+1;
		my_data->sum = result;

		hpx_call(HPX_HERE, _Pi, done, my_data, sizeof(struct pi_data));
		hpx_lco_wait(done);
		hpx_lco_get(done, sizeof(double), &result_2);
		hpx_lco_delete(done, HPX_NULL);

		HPX_THREAD_CONTINUE(result_2);
	}
	else
		HPX_THREAD_CONTINUE(sum);

	return HPX_SUCCESS;
}

static int _Pi_main_action(void *args, size_t size)
{
	double sum=0;
	struct pi_data pi_Input;

	pi_Input.counter   = 0;
	pi_Input.sum       = 0;

	step = 1.0/(double) PI_STEP;

	hpx_time_t now = hpx_time_now();

	hpx_call_sync(HPX_HERE, _Pi, &sum, sizeof(sum), &pi_Input, sizeof(struct pi_data));

	double elapsed = hpx_time_elapsed_ms(now)/1e3;

	pi = step*sum;

	printf("Steps: %f; Pi=%f\n", PI_STEP, pi);
	printf("seconds: %.7f\n", elapsed);
	//printf("localities: %d\n", HPX_LOCALITIES);
	//printf("threads/locality: %d\n", HPX_THREADS);

	hpx_exit(HPX_SUCCESS);
}

int main(int argc, char *argv[])
{

	if (hpx_init(&argc, &argv) != 0) {
		fprintf(stderr, "HPX: failed to initialize.\n");
		return -1;
	}

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

	switch (argc) {
		case 0:
			fprintf(stderr, "\nMissing Pi number.\n"); // fall through
		default:
			_usage(stderr, EXIT_FAILURE);
		case 1:
			PI_STEP = atof(argv[0]);
			break;
	}

	HPX_REGISTER_ACTION(HPX_DEFAULT, HPX_MARSHALLED, _Pi, _Pi_action, HPX_POINTER, HPX_SIZE_T);
	HPX_REGISTER_ACTION(HPX_DEFAULT, HPX_MARSHALLED, _Pi_main, _Pi_main_action, HPX_POINTER, HPX_SIZE_T);

	int e = hpx_run(&_Pi_main, NULL, 0);
	hpx_finalize();
	return e;
}


