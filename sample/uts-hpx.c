#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include <hpx/hpx.h>

typedef unsigned long long counter_t;

static hpx_action_t _uts      = 0;
static hpx_action_t _uts_main = 0;

struct thread_data {
	int depth;
	int numChildren;
	int factor;
	bool subtree;
};

double spawns_counter= 1;
hpx_addr_t mutex;

static counter_t _uts_action(void *args, size_t size) 
{
	counter_t subtreesize = 1;
	struct thread_data *my_data;
	struct thread_data temp, input;
	int i;

	my_data = (struct thread_data *)args;	
	//printf("args: %d %d %d\n", my_data->depth, my_data->numChildren, my_data->factor);
	temp.depth        = my_data->depth;
	temp.numChildren  = my_data->numChildren;
	temp.factor       = my_data->factor;
	temp.subtree	  = my_data->subtree;
	//printf("par: %d %d %d\n", temp.depth, temp.numChildren, temp.factor);

	hpx_lco_sema_p (mutex);
	printf("D:%-4d; child:%d; spawns:%.0f\n", temp.depth, temp.numChildren, spawns_counter++);
	hpx_lco_sema_v_sync (mutex);

	if(!temp.subtree) HPX_THREAD_CONTINUE(subtreesize);
	if(temp.depth == temp.factor) HPX_THREAD_CONTINUE(subtreesize);

	counter_t partialCount[temp.numChildren];
	int pivot = 0;
	time_t t;
	srand((unsigned) time(&t));
	pivot = rand() % temp.numChildren;

	hpx_addr_t theThread = HPX_HERE;
	for (i = 0; i < temp.numChildren; i++) {
		input.depth       = temp.depth+1;
		input.numChildren = temp.numChildren;
		input.factor      = temp.factor;

		if(i == 0) input.subtree = false;
		else input.subtree = true;

		hpx_call_sync(theThread, _uts, &partialCount[i], sizeof(partialCount[i]), &input, sizeof(input));
		//partialCount[i] = parTreeSearch(&input);
		//printf("[%d] %llu\n", i, partialCount[i]);
	}

	for (i = 0; i < temp.numChildren; i++) {
		subtreesize += partialCount[i];
	}

	HPX_THREAD_CONTINUE(subtreesize);
	return HPX_SUCCESS;
}

void parallel_uts(struct thread_data *root ) {
	struct thread_data input;
	hpx_time_t start;
	double bots_time_program;
	hpx_addr_t theThread = HPX_HERE;
	counter_t num_nodes = 0;

	input.depth        = root->depth;
	input.numChildren  = root->numChildren;
	input.factor       = root->factor;
	input.subtree      = root->subtree;

	//printf("main: %d %d %d\n", input.depth, input.numChildren, input.factor);
	//num_nodes = parTreeSearch(&input);
	printf("numChildren: %d, factor: %d, ", input.numChildren, input.factor);
	start = hpx_time_now();
	hpx_call_sync(theThread, _uts, &num_nodes, sizeof(num_nodes), &input, sizeof(input));
	bots_time_program = hpx_time_elapsed_ms(start)/1e3;

	printf("Tree size = %llu\n", (unsigned long long)num_nodes );
	printf("Wallclock time = %.3f sec\n", bots_time_program );
}

static int _uts_main_action(void *args, size_t size)
{
	struct thread_data *root = (struct thread_data *)args;

	mutex = hpx_lco_sema_new(1);
	parallel_uts(root);

	hpx_exit(HPX_SUCCESS);
}

int main(int argc, char *argv[])
{
	struct thread_data root;
	int e = hpx_init(&argc, &argv);
	if (e) {
		fprintf(stderr, "HPX: failed to initialize.\n");
		return e;
	}

	int opt = 0;
	while ((opt = getopt(argc, argv, "h?")) != -1) {
		switch (opt) {
			case 'h':
				fprintf(stdout, "uts [# of children] [branching factor]\n");
				return 1;
			case '?':
			default:
				fprintf(stdout, "uts [# of children] [branching factor]\n");
				return 1;
		}
	}

	argc -= optind;
	argv += optind;

	int children = 0;
	int factor = 0;
	switch (argc) {
		case 0:
		case 1:
		default:
			fprintf(stderr, "\nMissing uts numbers.\n");
		case 2:
			children = atoi(argv[0]);
			factor = atoi(argv[1]);
			break;
	}
	//printf("%d %d\n", children, factor);
	//parallel_uts( children, factor );

	root.depth        = 1;
	root.numChildren  = children;
	root.factor       = factor;
	root.subtree      = true;

	HPX_REGISTER_ACTION(HPX_DEFAULT, HPX_MARSHALLED, _uts, _uts_action,
			HPX_POINTER, HPX_SIZE_T);
	HPX_REGISTER_ACTION(HPX_DEFAULT, HPX_MARSHALLED, _uts_main, _uts_main_action,
			HPX_POINTER, HPX_SIZE_T);

	int t = hpx_run(&_uts_main, &root, sizeof(struct thread_data));
	hpx_finalize();
	return t;
}

