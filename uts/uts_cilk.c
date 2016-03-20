#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <cilk/cilk.h>
#include <sys/time.h>
#include <sys/resource.h>

#include "app-desc.h"
#include "bots.h"
#include "uts.h"


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
	long bots_t_start;
	long bots_t_end;
	counter_t num_nodes;

	//printf("Computing Unbalance Tree Search algorithm ");

	bots_t_start = bots_usecs();

	num_nodes = parTreeSearch( 0, root, getNumRootChildren(root) );

	bots_t_end = bots_usecs();
	bots_time_program = ((double)(bots_t_end-bots_t_start))/1000000;
	printf("%.3f sec\n", bots_time_program );
	//printf(" completed!");

	return num_nodes;
}

counter_t parTreeSearch(int depth, Node *parent, int numChildren) 
{
	Node n[numChildren], *nodePtr;
	int i, j;
	counter_t subtreesize = 1, partialCount[numChildren];

	// Recurse on the children
	for (i = 0; i < numChildren; i++) {
		nodePtr = &n[i];

		nodePtr->height = parent->height + 1;

		// The following line is the work (one or more SHA-1 ops)
		for (j = 0; j < computeGranularity; j++) {
			rng_spawn(parent->state.state, nodePtr->state.state, i);
		}

		nodePtr->numChildren = uts_numChildren(nodePtr);

		//task firstprivate(i, nodePtr) shared(partialCount) untied
		partialCount[i] = cilk_spawn parTreeSearch(depth+1, nodePtr, nodePtr->numChildren);
	}

	cilk_sync;
	//#pragma omp taskwait

	for (i = 0; i < numChildren; i++) {
		subtreesize += partialCount[i];
	}

	return subtreesize;
}

void uts_read_file ( char *filename )
{
	FILE *fin;

	printf("%s, ", filename);
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
	/*
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
	   */
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
	printf("Wallclock time                       = %f sec\n", bots_time_program );
	printf("Overall performance                  = %.0f nodes/sec\n", (bots_number_of_tasks / bots_time_program) );
	printf("Performance per PE                   = %.0f nodes/sec\n", (bots_number_of_tasks / bots_time_program / nPes) );
}

int uts_check_result ( void )
{
	//printf("\nbots_number_of_tasks = %d\nexp_tree_size = %llu\n",  bots_number_of_tasks, exp_tree_size); 
	if ( bots_number_of_tasks != exp_tree_size ) {
		printf("Tree size value is non valid.\n");
		printf( "RESULT_UNSUCCESSFUL!\n" );
	}
	else
		;//printf( "RESULT_SUCCESSFUL!\n" );

	return 0;
}

long bots_usecs (void)
{
	struct timeval t;
	gettimeofday(&t,((void *)0));
	return t.tv_sec*1000000+t.tv_usec;
}

int main(int argc, char *argv[])
{
	long bots_t_start;
	long bots_t_end;
	Node root; 
	char bots_arg_file[255];//"./input/large.input"

	if( argc < 1 ) {
		printf("No input file.\n");
		return 0;
	}
	else {
		strcpy(bots_arg_file, argv[1] );
	}

	__cilkrts_set_param("stack size", "536870912");

	//setting up stack size
	const rlim_t kStackSize = 512 * 1024 * 1024;
	struct rlimit rl;
	int result;

	result = getrlimit(RLIMIT_STACK, &rl);
	//printf ("Default Limit = %ld\n", rl.rlim_cur);
	if (result == 0)
	{
		if (rl.rlim_cur < kStackSize)
		{
			rl.rlim_cur = kStackSize;
			result = setrlimit(RLIMIT_STACK, &rl);
			if (result != 0)
			{
				fprintf(stderr, "setrlimit returned result = %d\n", result);
			}
		}
	}
	//printf ("Stack Limit = %ld\n", rl.rlim_cur);
	//end of setting

	
	//end of setting
	uts_read_file(bots_arg_file);;

	uts_initRoot(&root, type);;

	//bots_t_start = bots_usecs();
	bots_number_of_tasks = parallel_uts(&root);;
	//bots_t_end = bots_usecs();
	//bots_time_program = ((double)(bots_t_end-bots_t_start))/1000000;

	//uts_show_stats();;

	uts_check_result();;

	return 0;
}



