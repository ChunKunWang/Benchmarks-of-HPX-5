#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <omp.h>
#include <cilk/cilk.h>
#include "hpx/hpx.h"

#define DNUM 1000000
#define THREAD_LEVEL 20

bool RUN_HPX    = false;
bool RUN_HPX_TL = false;
bool RUN_CILK   = false;
bool RUN_OPENMP = false;
bool RUN_SEQ    = false;
bool RUN_IN     = false;
bool SET_SEED   = false;
bool SORTED_AS  = false;
bool SORTED_DE  = false;

uint64_t SEED = 0;

static void _usage(FILE *f, int error) {
	fprintf(f, "Usage: hybrid [options] [-i SEED] NUMBER\n"
			"\t-x, HPX-5 qsort (default)\n"
			"\t-l, HPX-5 with thread level control\n"
			"\t-c, Cilk qsort\n"
			"\t-o, OpenMP qsort\n"
			"\t-s, Sequential qsort\n"
			"\t-b, Build-in qsort\n"
			"\t-a, Set ascending input\n"
			"\t-r, Set descending input\n"
			"\t-i, Set seed\n"
			"\t-?, show HPX-5 options\n"
			"\t-h, show help\n");
	fflush(f);
	exit(error);
}

//for sequential, OpenMP and parallel implementation
void swap(double lyst[], int i, int j);
int partition(double lyst[], int lo, int hi);
void quicksortHelper(double lyst[], int lo, int hi);
void quicksort(double lyst[], int size);
void Cilk_quicksortHelper(double lyst[], int lo, int hi);
void Cilk_quicksort(double lyst[], int size);
void OMP_quicksortHelper(double lyst[], int lo, int hi);
void OMP_quicksort(double lyst[], int size);
int isSorted(double lyst[], int size);
int isReSorted(double lyst[], int size);

static hpx_action_t _main = 0;
static hpx_action_t _parallelQuicksortHelper = 0;
static hpx_action_t _NoTLQuicksortHelper = 0;
//for parallel implementation
int parallelQuicksort(double lyst[], int size, int tlevel);
static int _parallelQuicksortHelper_action(void *threadarg, size_t size);
int NoTLQuicksort(double lyst[], int size);
static int _NoTLQuicksortHelper_action(void *threadarg, size_t size);

struct thread_data{
	double *lyst;
	int low;
	int high;
	int level;
};
//thread_data should be thread-safe, since while lyst is
//shared, [low, high] will not overlap among threads.

//for the builtin libc qsort:
int compare_doubles (const void *a, const void *b);
int re_compare_doubles (const void *a, const void *b);

/*
   Main action:
   -generate random list
   -time sequential quicksort
   -time OpenMP quicksort
   -time Cilk quicksort
   -time hpx-5 parallel quicksort
   -time standard qsort
   */
static int _main_action(uint64_t *args, size_t size) {
	hpx_time_t start;
	uint64_t NUM = *args;

	if( SET_SEED ) srand(SEED);
	else srand(time(NULL)); //seed random

	printf("# of elements: %"PRIu64"\n", NUM);
	printf("Generating random data... ");
	//Want to compare sorting on the same list,
	//so backup.
	double *lystbck = (double *) malloc(NUM*sizeof(double));
	double *lyst = (double *) malloc(NUM*sizeof(double));

	//Populate random original/backup list.
	for (int i = 0; i < NUM; i ++) {
		lystbck[i] = 1.0*rand()/RAND_MAX;
		//printf("%f ", lystbck[i]);
	}
	printf("finish!\n");

	if( SORTED_AS || SORTED_DE ) {
		if( SORTED_AS ) {
			start = hpx_time_now();
			qsort(lystbck, NUM, sizeof(double), compare_doubles);
			printf("Generating ascending input: %g ms. \n", hpx_time_elapsed_ms(start));
			if (!isSorted(lystbck, NUM))
				printf("Oops, lystbck did not get ascending sorted.\n");
		}
		else {
			start = hpx_time_now();
			qsort(lystbck, NUM, sizeof(double), re_compare_doubles);
			printf("Generating descending input: %g ms. \n", hpx_time_elapsed_ms(start));
			if (!isReSorted(lystbck, NUM))
				printf("Oops, lystbck did not get descending sorted.\n");
		}
		//for (int i = 0; i < NUM; i ++) 
		//	printf("%f ", lystbck[i]);
	}

	//HPX-5 parallel quicksort.
	if( RUN_HPX_TL ) {
		//copy list.
		memcpy(lyst, lystbck, NUM*sizeof(double));
		start = hpx_time_now();
		NoTLQuicksort(lyst, NUM);
		printf("HPX-5 quicksort       : %g ms.\n", hpx_time_elapsed_ms(start));
		if (!isSorted(lyst, NUM)) {
			printf("Oops, lyst did not get sorted by hpx-5 LT parallelQuicksort.\n");
		}
	}

	//HPX-5 parallel quicksort with thread level control.
	if( RUN_HPX ) {
		//copy list.
		memcpy(lyst, lystbck, NUM*sizeof(double));
		start = hpx_time_now();
		parallelQuicksort(lyst, NUM, THREAD_LEVEL);
		printf("HPX-5 with TL control : %g ms.\n", hpx_time_elapsed_ms(start));
		if (!isSorted(lyst, NUM)) {
			printf("Oops, lyst did not get sorted by hpx-5 parallelQuicksort.\n");
		}
	}
	
	//Cilk quicksort, and timing
	if( RUN_CILK ) {
		//copy list.
		memcpy(lyst, lystbck, NUM*sizeof(double));
		start = hpx_time_now();
		Cilk_quicksort(lyst, NUM);
		printf("Cilk quicksort        : %g ms. \n", hpx_time_elapsed_ms(start));
		if (!isSorted(lyst, NUM)) {
			printf("Oops, lyst did not get sorted by Cilk_quicksort.\n");
		}
	}

	//OpenMP quicksort, and timing
	if( RUN_OPENMP ) {
		//copy list.
		memcpy(lyst, lystbck, NUM*sizeof(double));
		start = hpx_time_now();
		OMP_quicksort(lyst, NUM);
		printf("OpenMP quicksort      : %g ms. \n", hpx_time_elapsed_ms(start));
		if (!isSorted(lyst, NUM)) {
			printf("Oops, lyst did not get sorted by OMP_quicksort.\n");
		}
	}

	//Sequential quicksort, and timing
	if( RUN_SEQ ) {
		//copy list.
		memcpy(lyst, lystbck, NUM*sizeof(double));
		start = hpx_time_now();
		quicksort(lyst, NUM);
		printf("Sequential quicksort  : %g ms. \n", hpx_time_elapsed_ms(start));
		if (!isSorted(lyst, NUM)) {
			printf("Oops, lyst did not get sorted by quicksort.\n");
		}
	}

	//Finally, built-in for reference:
	if( RUN_IN ) {
		memcpy(lyst, lystbck, NUM*sizeof(double));
		start = hpx_time_now();
		qsort(lyst, NUM, sizeof(double), compare_doubles);
		printf("Built-in qsort        : %g ms.\n", hpx_time_elapsed_ms(start));
		if (!isSorted(lyst, NUM)) {
			printf("Oops, lyst did not get sorted by qsort.\n");
		}
	}

	printf("\n");
	free(lyst);
	free(lystbck);
	hpx_exit(HPX_SUCCESS);
}

int main (int argc, char *argv[])
{
	if (hpx_init(&argc, &argv) != 0) {
		fprintf(stderr, "HPX: failed to initialize.\n");
		return -1;
	}

	int opt = 0;
	uint64_t NUM = DNUM;

	while ((opt = getopt(argc, argv,  "h?bscoxilar")) != -1) {
		switch (opt) {
			case 'a':
				SORTED_AS = true;
				break;
			case 'r':
				SORTED_DE = true;
				break;
			case 'l':
				RUN_HPX_TL = true;
				break;
			case 'i':
				SET_SEED = true;
				break;
			case 'b':
				RUN_IN = true;
				break;
			case 's':
				RUN_SEQ = true;
				break;
			case 'c':
				RUN_CILK = true;
				break;
			case 'o':
				RUN_OPENMP = true;
				break;
			case 'x':
				RUN_HPX = true;
				break;
			case 'h':
				_usage(stdout, EXIT_SUCCESS);
			case '?':
				hpx_print_help();
				exit(EXIT_SUCCESS);
			default:
				_usage(stderr, EXIT_FAILURE);
		}
	}

	argc -= optind;
	argv += optind;

	switch (argc) {
		case 0:
			fprintf(stderr, "\nMissing user specified list size.\n"); // fall through
		default:
			_usage(stderr, EXIT_FAILURE);
		case 1:
			NUM = atoi(argv[0]);
			break;
		case 2:
			SEED = atoi(argv[0]);
			NUM = atoi(argv[1]);
			break;
	}

	// Register the main action
	HPX_REGISTER_ACTION(HPX_DEFAULT, HPX_MARSHALLED, _main, _main_action,
			HPX_POINTER, HPX_SIZE_T);
	HPX_REGISTER_ACTION(HPX_DEFAULT, HPX_MARSHALLED, _parallelQuicksortHelper,
			_parallelQuicksortHelper_action, HPX_POINTER, HPX_SIZE_T);
	HPX_REGISTER_ACTION(HPX_DEFAULT, HPX_MARSHALLED, _NoTLQuicksortHelper,
			_NoTLQuicksortHelper_action, HPX_POINTER, HPX_SIZE_T);

	// Run the main action
	int e = hpx_run(&_main, &NUM, sizeof(NUM));
	hpx_finalize();
	return e;
}

void Cilk_quicksort(double lyst[], int size)
{
	Cilk_quicksortHelper(lyst, 0, size-1);
}

void Cilk_quicksortHelper(double lyst[], int lo, int hi)
{
	if (lo >= hi) return;
	int b = partition(lyst, lo, hi);
	cilk_spawn Cilk_quicksortHelper(lyst, lo, b-1);
	cilk_spawn Cilk_quicksortHelper(lyst, b+1, hi);
	cilk_sync;
}

void OMP_quicksort(double lyst[], int size)
{
#pragma omp parallel
#pragma omp single nowait
	OMP_quicksortHelper(lyst, 0, size-1);
}

void OMP_quicksortHelper(double lyst[], int lo, int hi)
{
	if (lo >= hi) return;
	int b = partition(lyst, lo, hi);
#pragma omp task untied 
	OMP_quicksortHelper(lyst, lo, b-1);
#pragma omp task untied
	OMP_quicksortHelper(lyst, b+1, hi);
#pragma omp taskwait
}

void quicksort(double lyst[], int size)
{
	quicksortHelper(lyst, 0, size-1);
}

void quicksortHelper(double lyst[], int lo, int hi)
{
	if (lo >= hi) return;
	int b = partition(lyst, lo, hi);
	quicksortHelper(lyst, lo, b-1);
	quicksortHelper(lyst, b+1, hi);
}
void swap(double lyst[], int i, int j)
{
	double temp = lyst[i];
	lyst[i] = lyst[j];
	lyst[j] = temp;
}

int partition(double *lyst, int lo, int hi)
{
	int pivot = (int) ((lo + hi)/2);
	//int pivot = (int) (lo + (hi-lo + 1)*(1.0*rand()/RAND_MAX));
	double pivotValue = lyst[pivot];
	swap(lyst, pivot, hi);
	int storeIndex = lo;
	for (int i=lo ; i<hi ; i++) {
		if (lyst[i] <= pivotValue) {
			swap(lyst, i, storeIndex);
			storeIndex++;
		}
	}
	swap(lyst, storeIndex, hi);
	return storeIndex;
}

int NoTLQuicksort(double lyst[], int size)
{
	hpx_addr_t theThread = HPX_HERE;
	struct thread_data td;
	td.lyst = lyst;
	td.low = 0;
	td.high = size - 1;
	td.level = 0;

	//The top-level thread
	hpx_addr_t done = hpx_lco_future_new(sizeof(uint64_t));
	hpx_call(theThread, _NoTLQuicksortHelper, done, &td, sizeof(td));
	hpx_lco_wait(done);
	hpx_lco_delete(done, HPX_NULL);

	return HPX_SUCCESS;
}

static int _NoTLQuicksortHelper_action(void *threadarg, size_t size)
{
	int mid, t;

	struct thread_data *my_data;
	my_data = (struct thread_data *) threadarg;

	//fyi:
	//printf("Thread responsible for [%d, %d], level %d.\n",
	//my_data->low, my_data->high, my_data->level);

	if (my_data->low >= my_data->high) 
		hpx_thread_exit(HPX_SUCCESS);

	//Now we partition our part of the lyst.
	mid = partition(my_data->lyst, my_data->low, my_data->high);

	//At this point, we will create threads for the
	//left and right sides.  Must create their data args.
	struct thread_data thread_data_array[2];

	for (t = 0; t < 2; t ++) {
		thread_data_array[t].lyst = my_data->lyst;
		thread_data_array[t].level = my_data->level - 1;
	}
	thread_data_array[0].low = my_data->low;
	if (mid > my_data->low) {
		thread_data_array[0].high = mid-1;
	} else {
		thread_data_array[0].high = my_data->low;
	}
	if (mid < my_data->high) {
		thread_data_array[1].low = mid+1;
	} else {
		thread_data_array[1].low = my_data->high;
	}
	thread_data_array[1].high = my_data->high;

	//Now, instantiate the threads.
	//In quicksort of course, due to the transitive property,
	//no elements in the left and right sides of mid will have
	//to be compared again.
	hpx_addr_t futures[] = {
		hpx_lco_future_new(sizeof(int)),
		hpx_lco_future_new(sizeof(int))
	};

	hpx_addr_t threads[] = {
		HPX_HERE,
		HPX_HERE
	};

	int pqs[] = {
		0,
		0
	};

	void *addrs[] = {
		&pqs[0],
		&pqs[1]
	};

	int sizes[] = {
		sizeof(int),
		sizeof(int)
	};

	for (t = 0; t < 2; t++){
		hpx_call(threads[t], _parallelQuicksortHelper, futures[t],
				(void *)&thread_data_array[t], sizeof(thread_data_array[t]));
	}
	hpx_lco_get_all(2, futures, sizes, addrs, NULL);
	hpx_lco_delete(futures[0], HPX_NULL);
	hpx_lco_delete(futures[1], HPX_NULL);
	return HPX_SUCCESS;
}

/*
   parallel quicksort top level:
   instantiate parallelQuicksortHelper thread, and that's
   basically it.
   */
int parallelQuicksort(double lyst[], int size, int tlevel)
{
	hpx_addr_t theThread = HPX_HERE;
	struct thread_data td;
	td.lyst = lyst;
	td.low = 0;
	td.high = size - 1;
	td.level = tlevel;

	//The top-level thread
	hpx_addr_t done = hpx_lco_future_new(sizeof(uint64_t));
	hpx_call(theThread, _parallelQuicksortHelper, done, &td, sizeof(td));
	hpx_lco_wait(done);
	hpx_lco_delete(done, HPX_NULL);

	return HPX_SUCCESS;
}

/*
   parallelQuicksortHelper
   -if the level is still > 0, then partition and make
   parallelQuicksortHelper threads to solve the left and
   right-hand sides, then quit. Otherwise, call sequential.
   */
static int _parallelQuicksortHelper_action(void *threadarg, size_t size)
{
	int mid, t;

	struct thread_data *my_data;
	my_data = (struct thread_data *) threadarg;

	//fyi:
	//printf("Thread responsible for [%d, %d], level %d.\n",
	//my_data->low, my_data->high, my_data->level);

	if (my_data->level <= 0 || my_data->low == my_data->high) {
		//We have plenty of threads, finish with sequential.
		quicksortHelper(my_data->lyst, my_data->low, my_data->high);
		hpx_thread_exit(HPX_SUCCESS);
	}

	//Now we partition our part of the lyst.
	mid = partition(my_data->lyst, my_data->low, my_data->high);

	//At this point, we will create threads for the
	//left and right sides.  Must create their data args.
	struct thread_data thread_data_array[2];

	for (t = 0; t < 2; t ++) {
		thread_data_array[t].lyst = my_data->lyst;
		thread_data_array[t].level = my_data->level - 1;
	}
	thread_data_array[0].low = my_data->low;
	if (mid > my_data->low) {
		thread_data_array[0].high = mid-1;
	} else {
		thread_data_array[0].high = my_data->low;
	}
	if (mid < my_data->high) {
		thread_data_array[1].low = mid+1;
	} else {
		thread_data_array[1].low = my_data->high;
	}
	thread_data_array[1].high = my_data->high;

	//Now, instantiate the threads.
	//In quicksort of course, due to the transitive property,
	//no elements in the left and right sides of mid will have
	//to be compared again.
	hpx_addr_t futures[] = {
		hpx_lco_future_new(sizeof(int)),
		hpx_lco_future_new(sizeof(int))
	};

	hpx_addr_t threads[] = {
		HPX_HERE,
		HPX_HERE
	};

	int pqs[] = {
		0,
		0
	};

	void *addrs[] = {
		&pqs[0],
		&pqs[1]
	};

	int sizes[] = {
		sizeof(int),
		sizeof(int)
	};

	for (t = 0; t < 2; t++){
		hpx_call(threads[t], _parallelQuicksortHelper, futures[t],
				(void *)&thread_data_array[t], sizeof(thread_data_array[t]));
	}
	hpx_lco_get_all(2, futures, sizes, addrs, NULL);
	hpx_lco_delete(futures[0], HPX_NULL);
	hpx_lco_delete(futures[1], HPX_NULL);
	return HPX_SUCCESS;
}

//check if the elements of lyst are in non-decreasing order.
//one is success.
int isSorted(double lyst[], int size)
{
	for (int i = 1; i < size; i ++) {
		if (lyst[i] < lyst[i-1]) {
			printf("at loc %d, %f < %f \n", i, lyst[i], lyst[i-1]);
			return 0;
		}
	}
	return 1;
}

int isReSorted(double lyst[], int size)
{
	for (int i = 1; i < size; i ++) {
		if (lyst[i] > lyst[i-1]) {
			printf("at loc %d, %f > %f \n", i, lyst[i], lyst[i-1]);
			return 0;
		}
	}
	return 1;
}

//for the built-in qsort comparator
//from http://www.gnu.org/software/libc/manual/html_node/Comparison-Functions.html#Comparison-Functions
int compare_doubles (const void *a, const void *b)
{
	const double *da = (const double *) a;
	const double *db = (const double *) b;

	return (*da > *db) - (*da < *db);
}

int re_compare_doubles (const void *a, const void *b)
{
	const double *da = (const double *) a;
	const double *db = (const double *) b;

	return (*da < *db) - (*da > *db);
}

