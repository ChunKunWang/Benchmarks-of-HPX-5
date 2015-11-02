#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <hpx/hpx.h>
#include <string.h>

#define MAX_SIZE 16
#define attack(i, j) (hist[j] == i || abs(hist[j] - i) == col - j)

static void _usage(FILE *f, int error) {
	fprintf(f, "Usage: hpx-5 [options] NUMBER\n");
	//hpx_print_help();
	fflush(f);
	exit(error);
}

static hpx_action_t _nqueens      = 0;
static hpx_action_t _dummy        = 0;
static hpx_action_t _nqueens_main = 0;

struct thread_data{
	//int *lyst;
	int lyst[MAX_SIZE];
	int n;
	int col;
};

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

hpx_addr_t mutex; //for count
int count = 0;

int verify_queens (int size)
{
	printf( "total_count = %d; solution = %d\n", count, solutions[size-1] );

	if ( count == solutions[size-1]) {
		printf( "RESULT_SUCCESSFUL!\n" );
		return 1;
	}

	printf( "RESULT_UNSUCCESSFUL!\n" );

	return 0;
}

void solve(int n, int col, int *hist)
{
	int i, j;

	if (col == n) {
		++count;
		/*
		   printf("\nNo. %d\n-----\n", ++count);
		   for (int i = 0; i < n; i++, putchar('\n'))
		   for (int j = 0; j < n; j++)
		   putchar(j == hist[i] ? 'Q' : ((i + j) & 1) ? ' ' : '.');
		   */
		return;
	}

	for (i = 0, j = 0; i < n; i++) {
		for (j = 0; j < col && !attack(i, j); j++);
		if (j < col) continue;

		hist[col] = i;
		solve(n, col + 1, hist);
	}
}

static int _dummy_action(void *args, size_t size)
{
	hpx_thread_exit(HPX_SUCCESS);
}

static int _nqueens_action(void *args, size_t size)
{
	int i, j;
	struct thread_data *my_data;
	my_data = (struct thread_data *) args;

	/*
	printf("n = %d, col = %d, count = %d\n", my_data->n
			, my_data->col
			, count);
	*/

	if (my_data->col == my_data->n) {
		hpx_lco_sema_p(mutex);
		++count;
		/*				
						printf("\nNo. %d\n-----\n", count);
						for (i = 0; i < my_data->n; i++, putchar('\n'))
						for(j = 0; j < my_data->n; j++)
						putchar(j == my_data->lyst[i] ? 'Q' : ((i + j) & 1) ? ' ' : '.');
						*/
		hpx_lco_sema_v_sync(mutex);

		hpx_thread_exit(HPX_SUCCESS);
		//hpx_thread_continue(NULL, 0);
		//return HPX_SUCCESS;
	}


#define p_attack(i, j) (my_data->lyst[j] == i || abs(my_data->lyst[j] - i) == my_data->col - j)

	int dummy=0;
	int num_spawns=0;
	for(i = 0, j = 0; i < my_data->n; i++) {
		for (j = 0; j < my_data->col && !p_attack(i, j); j++);
		if (j < my_data->col) {
			dummy++;
		}
	}
	//printf("dummy/spawns: %d/%d\n", dummy, my_data->n);

	num_spawns = my_data->n - dummy;
	bool D_CALL = false;

	//printf("num_spawns = %d\n", num_spawns);
	if( num_spawns == 0 ) {
		num_spawns = 1;
		D_CALL = true;
	}

	//num_spawns = my_data->n;
	struct thread_data temp[num_spawns];
	hpx_addr_t futures[num_spawns];
	hpx_addr_t threads[num_spawns];
	int pqs[num_spawns];
	int p_size[num_spawns];
	void *addrs[num_spawns];

	for(i = 0; i < num_spawns; i++) {
		futures[i] = hpx_lco_future_new(sizeof(int));
		threads[i] = HPX_HERE;
		pqs[i] = 0;
		addrs[i] = &pqs[i];
		p_size[i] = sizeof(int);
	}

	int k=0; // counter for hpx data
	for(i = 0, j = 0; i < my_data->n; i++) {
		for (j = 0; j < my_data->col && !p_attack(i, j); j++);
		if (j < my_data->col) {
			//printf("[%d] call continue.\n", i);
			continue;
		}

		//printf("[%d] call nqueens %d\n", i, k);

		my_data->lyst[my_data->col] = i;

		//temp[i].lyst = my_data->lyst;  
		memcpy(temp[k].lyst, my_data->lyst, MAX_SIZE*sizeof(int));
		temp[k].n    = my_data->n;
		temp[k].col  = my_data->col+1;

		//solve(n, col + 1, hist);
		hpx_call(threads[k], _nqueens, futures[k], (void *)&temp[k], sizeof(temp[k]));
		k++;
	}
	//hpx_lco_get_all(my_data->n, futures, p_size, addrs, NULL);
	if( D_CALL ) { 
		//printf("call dummy\n");
		hpx_call(threads[0], _dummy, futures[0], NULL, 0);
	}

	hpx_lco_get_all(num_spawns, futures, p_size, addrs, NULL);

	for(i = 0; i < num_spawns; i++)
		hpx_lco_delete(futures[i], HPX_NULL);

	return HPX_SUCCESS;
}

int parallel_nqueens(int n, int col, int *hist)
{
	hpx_addr_t theThread = HPX_HERE;
	struct thread_data td;
	//td.lyst = hist;
	td.n = n;
	td.col = col;
	memcpy(td.lyst, hist, MAX_SIZE*sizeof(int));
	mutex = hpx_lco_sema_new(1);

	//solve(td.n, td.col, td.lyst);
	hpx_addr_t done = hpx_lco_future_new(sizeof(uint64_t));
	hpx_call(theThread, _nqueens, done, &td, sizeof(td));
	hpx_lco_wait(done);
	hpx_lco_delete(done, HPX_NULL);

	return HPX_SUCCESS;
}

static int _nqueens_main_action(int *args, size_t size)
{
	int n = *args;
	int hist[n];
	hpx_time_t start;

	start = hpx_time_now();
	parallel_nqueens(n, 0, hist);
	printf("HPX-5 %d-nqueens took: %.7f (s)\n", n, hpx_time_elapsed_ms(start)/1e3);

	verify_queens(n);
	hpx_shutdown(HPX_SUCCESS);
}

int main(int argc, char *argv[]) 
{
	int e = hpx_init(&argc, &argv);
	if (e) {
		fprintf(stderr, "HPX: failed to initialize.\n");
		return e;
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
			fprintf(stderr, "\nMissing nqueens number.\n"); // fall through
		default:
			_usage(stderr, EXIT_FAILURE);
		case 1:
			n = atoi(argv[0]);
			break;
	}
	if( n > MAX_SIZE) {
		printf("Max size is %d.\n", MAX_SIZE);
		n = MAX_SIZE;
	}

	HPX_REGISTER_ACTION(HPX_DEFAULT, HPX_MARSHALLED, _nqueens_main, _nqueens_main_action,
			HPX_POINTER, HPX_SIZE_T);
	HPX_REGISTER_ACTION(HPX_DEFAULT, HPX_MARSHALLED, _dummy, _dummy_action,
			HPX_POINTER, HPX_SIZE_T);
	HPX_REGISTER_ACTION(HPX_DEFAULT, HPX_MARSHALLED, _nqueens, _nqueens_action,
			HPX_POINTER, HPX_SIZE_T);
	// run the main action
	return hpx_run(&_nqueens_main, &n, sizeof(n));
}



