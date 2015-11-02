#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <omp.h>
#include <sys/time.h>
#include <hpx/hpx.h>
#include <unistd.h>

#include "app-desc.h"
#include "bots.h"
#include "uts.h"

static hpx_action_t _uts      = 0;
static hpx_action_t _uts_main = 0;

struct thread_data {
	int depth;
	Node parent;
	int numChildren;
}; 

double spawns_counter= 0;
//hpx_addr_t mutex;

double bots_time_program;
int    bots_number_of_tasks;
char bots_resources[128];
bots_verbose_mode_t bots_verbose_mode;

/***********************************************************
 *  Global state                                           *
 ***********************************************************/
counter_t nLeaves = 0;
int maxTreeDepth = 0;
/***********************************************************
 *  tree generation and search parameters                  *
 *                                                         *
 *  Tree generation strategy is controlled via various     *
 *  parameters set from the command line.  The parameters  *
 *  and their default values are given below.              *
 ***********************************************************/
char * uts_trees_str[]     = { "Binomial" };
/***********************************************************
 *  Tree type
 *    Trees are generated using a Galton-Watson process, in 
 *    which the branching factor of each node is a random 
 *    variable.
 *     
 *  The random variable follow a binomial distribution.
 ***********************************************************/
tree_t type  = BIN; // Default tree type
double b_0   = 4.0; // default branching factor at the root
int   rootId = 0;   // default seed for RNG state at root
/***********************************************************
 *  Tree type BIN (BINOMIAL)
 *  The branching factor at the root is specified by b_0.
 *  The branching factor below the root follows an 
 *     identical binomial distribution at all nodes.
 *  A node has m children with prob q, or no children with 
 *     prob (1-q).  The expected branching factor is q * m.
 *  
 *  Default parameter values 
 ***********************************************************/
int    nonLeafBF   = 4;            // m
double nonLeafProb = 15.0 / 64.0;  // q
/***********************************************************
 *  compute granularity - number of rng evaluations per
 *  tree node
 ***********************************************************/
int computeGranularity = 1;
/***********************************************************
 *  expected results for execution
 ***********************************************************/
counter_t  exp_tree_size = 0;
int        exp_tree_depth = 0;
counter_t  exp_num_leaves = 0;
/***********************************************************
 *  FUNCTIONS                                              *
 ***********************************************************/

// Interpret 32 bit positive integer as value on [0,1)
double rng_toProb(int n)
{
	if (n < 0) {
		printf("*** toProb: rand n = %d out of range\n",n);
	}
	return ((n<0)? 0.0 : ((double) n)/2147483648.0);
}

void uts_initRoot(Node * root, int type)
{
	root->height = 0;
	root->numChildren = -1;      // means not yet determined
	rng_init(root->state.state, rootId);

	//printf("Root node of type %d at %p\n",type, root);
}

int uts_numChildren_bin(Node * parent)
{
	// distribution is identical everywhere below root
	int    v = rng_rand(parent->state.state);	
	double d = rng_toProb(v);

	return (d < nonLeafProb) ? nonLeafBF : 0;
}

int uts_numChildren(Node *parent)
{
	int numChildren = 0;

	/* Determine the number of children */
	if (parent->height == 0) numChildren = (int) floor(b_0);
	else numChildren = uts_numChildren_bin(parent);

	// limit number of children
	// only a BIN root can have more than MAXNUMCHILDREN
	if (parent->height == 0) {
		int rootBF = (int) ceil(b_0);
		if (numChildren > rootBF) {
			bots_debug("*** Number of children of root truncated from %d to %d\n", numChildren, rootBF);
			numChildren = rootBF;
		}
	}
	else {
		if (numChildren > MAXNUMCHILDREN) {
			bots_debug("*** Number of children truncated from %d to %d\n", numChildren, MAXNUMCHILDREN);
			numChildren = MAXNUMCHILDREN;
		}
	}

	return numChildren;
}

/***********************************************************
 * Recursive depth-first implementation                    *
 ***********************************************************/
int getNumRootChildren(Node *root)
{
	int numChildren;

	numChildren = uts_numChildren(root);
	root->numChildren = numChildren;

	return numChildren;
}

counter_t parallel_uts ( Node *root )
{
	struct thread_data input;
	hpx_time_t start;
	hpx_addr_t theThread = HPX_HERE;
	counter_t num_nodes;

	input.depth = 0;
	memcpy(&input.parent, root, sizeof(Node));
	input.numChildren = getNumRootChildren(root);

	printf("Computing Unbalance Tree Search algorithm ");

	hpx_addr_t done = hpx_lco_future_new(sizeof(uint64_t));

	start = hpx_time_now();
	hpx_call_sync(theThread, _uts, &num_nodes, sizeof(num_nodes), &input, sizeof(input));
	bots_time_program = hpx_time_elapsed_ms(start)/1e3;

	printf(" completed!");

	return num_nodes;
}

static counter_t _uts_action(void *args, size_t size) 
{
	int i, j;
	struct thread_data *my_data;
	struct thread_data temp, input;
	my_data = (struct thread_data *)args;	

	Node n[my_data->numChildren], *nodePtr;
	counter_t subtreesize = 1, partialCount[my_data->numChildren];

	temp.depth = my_data->depth;
	memcpy(&temp.parent, &my_data->parent, sizeof(Node));
	temp.numChildren = my_data->numChildren;

	//hpx_lco_sema_p (mutex);
	//printf("D: %d; child: %d; spawns:%.0f\n", temp.depth, temp.numChildren, spawns_counter++);
	//hpx_lco_sema_v_sync (mutex);

	/*
	   printf("\n[Node] height = %d; numChildren = %d\n"
	   , temp.parent.height
	   , temp.parent.numChildren);
	   */

	hpx_addr_t theThread = HPX_HERE;
	hpx_addr_t done = hpx_lco_future_new(sizeof(uint64_t));
	// Recurse on the children
	for (i = 0; i < temp.numChildren; i++) {
		nodePtr = &n[i];

		nodePtr->height = temp.parent.height + 1;

		// The following line is the work (one or more SHA-1 ops)
		for (j = 0; j < computeGranularity; j++) {
			rng_spawn(temp.parent.state.state, nodePtr->state.state, i);
		}

		nodePtr->numChildren = uts_numChildren(nodePtr);

		input.depth = temp.depth+1;
		memcpy(&input.parent, nodePtr, sizeof(Node));
		input.numChildren = nodePtr->numChildren;
		//partialCount[i] = parTreeSearch(depth+1, nodePtr, nodePtr->numChildren);
		hpx_call_sync(theThread, _uts, &partialCount[i], sizeof(partialCount[i]), &input, sizeof(input));
	}

	for (i = 0; i < temp.numChildren; i++) {
		subtreesize += partialCount[i];
	}

	HPX_THREAD_CONTINUE(subtreesize);
	return HPX_SUCCESS;
}

counter_t parTreeSearch(int depth, Node *parent, int numChildren) 
{
	Node n[numChildren], *nodePtr;
	int i, j;
	counter_t subtreesize = 1, partialCount[numChildren];


	//printf("[p] *** depth         = %d ***\n", depth);
	//printf("[p] *** height      = %d ***\n", parent->height);
	//printf("[p] *** numChildren = %d ***\n", parent->numChildren);

	// Recurse on the children
	for (i = 0; i < numChildren; i++) {
		nodePtr = &n[i];

		nodePtr->height = parent->height + 1;

		// The following line is the work (one or more SHA-1 ops)
		for (j = 0; j < computeGranularity; j++) {
			rng_spawn(parent->state.state, nodePtr->state.state, i);
		}

		nodePtr->numChildren = uts_numChildren(nodePtr);

		//#pragma omp task firstprivate(i, nodePtr) shared(partialCount) untied
		partialCount[i] = parTreeSearch(depth+1, nodePtr, nodePtr->numChildren);
	}

	//#pragma omp taskwait

	for (i = 0; i < numChildren; i++) {
		subtreesize += partialCount[i];
	}

	return subtreesize;
}

void uts_read_file ( char *filename )
{
	FILE *fin;

	if ((fin = fopen(filename, "r")) == NULL) {
		printf( "Could not open input file (%s)\n", filename);
		exit (-1);
	}
	fscanf(fin,"%lf %lf %d %d %d %llu %d %llu",
			&b_0,
			&nonLeafProb,
			&nonLeafBF,
			&rootId,
			&computeGranularity,
			&exp_tree_size,
			&exp_tree_depth,
			&exp_num_leaves
	      );
	fclose(fin);

	computeGranularity = max(1,computeGranularity);

	// Printing input data

	printf("\n");
	printf("Root branching factor                = %f\n", b_0);
	printf("Root seed (0 <= 2^31)                = %d\n", rootId);
	printf("Probability of non-leaf node         = %f\n", nonLeafProb);
	printf("Number of children for non-leaf node = %d\n", nonLeafBF);
	printf("E(n)                                 = %f\n", (double) ( nonLeafProb * nonLeafBF ) );
	printf("E(s)                                 = %f\n", (double) ( 1.0 / (1.0 - nonLeafProb * nonLeafBF) ) );
	printf("Compute granularity                  = %d\n", computeGranularity);
	printf("Tree type                            = %d (%s)\n", type, uts_trees_str[type]);
	printf("Random number generator              = "); rng_showtype();

}

void uts_show_stats( void )
{
	int nPes = atoi(bots_resources);
	int chunkSize = 0;

	printf("\n");
	printf("Tree size                            = %llu\n", (unsigned long long)  bots_number_of_tasks );
	printf("Maximum tree depth                   = %d\n", maxTreeDepth );
	printf("Chunk size                           = %d\n", chunkSize );
	printf("Number of leaves                     = %llu (%.2f%%)\n", nLeaves, nLeaves/(float)bots_number_of_tasks*100.0 ); 
	printf("Number of PE's                       = %.4d threads\n", nPes );
	printf("Wallclock time                       = %.3f sec\n", bots_time_program );
	printf("Overall performance                  = %.0f nodes/sec\n", (bots_number_of_tasks / bots_time_program) );
	printf("Performance per PE                   = %.0f nodes/sec\n", (bots_number_of_tasks / bots_time_program / nPes) );
}

int uts_check_result ( void )
{
	printf("\nbots_number_of_tasks = %d\nexp_tree_size = %llu\n",  bots_number_of_tasks, exp_tree_size); 
	if ( bots_number_of_tasks != exp_tree_size ) {
		printf("Tree size value is non valid.\n");
		printf( "RESULT_UNSUCCESSFUL!\n" );
	}
	else
		printf( "RESULT_SUCCESSFUL!\n" );

	return 0;
}

long bots_usecs (void)
{
	struct timeval t;
	gettimeofday(&t,((void *)0));
	return t.tv_sec*1000000+t.tv_usec;
}

static int _uts_main_action(void *args, size_t size)
{
	Node *root = (Node *)args;
	Node temp;

	uts_initRoot(&temp, type);

	//mutex = hpx_lco_sema_new(1);
	bots_number_of_tasks = parallel_uts(&temp);

	uts_show_stats();
	uts_check_result();

	hpx_exit(HPX_SUCCESS);
}

int main(int argc, char *argv[])
{
	long bots_t_start;
	long bots_t_end;
	Node root; 
	char bots_arg_file[255];

	int e = hpx_init(&argc, &argv);
	if (e) {
		fprintf(stderr, "HPX: failed to initialize.\n");
		return e;
	}

	int opt = 0;
	while ((opt = getopt(argc, argv, "h?")) != -1) {
		switch (opt) {
			case 'h':
				printf("./hpx-5 [file]");
			case '?':
			default:
				printf("./hpx-5 [file]");
		}
	}

	argc -= optind;
	argv += optind;

	switch (argc) {
		case 0:
			fprintf(stderr, "Missing uts file.\n"); // fall through
		default:
			printf("./hpx-5 [file]");
		case 1:
			strcpy(bots_arg_file, argv[0]);
			break;
	}

	uts_read_file(bots_arg_file);


	HPX_REGISTER_ACTION(HPX_DEFAULT, HPX_MARSHALLED, _uts, _uts_action,
			HPX_POINTER, HPX_SIZE_T);
	HPX_REGISTER_ACTION(HPX_DEFAULT, HPX_MARSHALLED, _uts_main, _uts_main_action,
			HPX_POINTER, HPX_SIZE_T);

	int t = hpx_run(&_uts_main, &root, sizeof(root));
	hpx_finalize();
	return t;
}

