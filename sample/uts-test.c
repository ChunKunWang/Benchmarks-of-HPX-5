#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

typedef unsigned long long counter_t;

struct thread_data {
	int depth;
	int numChildren;
	int factor;
	bool subtree;
};

counter_t parTreeSearch( struct thread_data *args )
{
	counter_t subtreesize = 1, partialCount[args->numChildren];
	struct thread_data temp, input;
	int i;

	temp.depth        = args->depth;
	temp.numChildren  = args->numChildren;
	temp.factor       = args->factor;
	//printf("par: %d %d %d\n", temp.depth, temp.numChildren, temp.factor);

	if(!args->subtree) return subtreesize;
	if(args->depth == args->factor) return subtreesize;

	int pivot = 0;
	time_t t;
	srand((unsigned) time(&t));
	pivot = rand() % temp.numChildren;

	for (i = 0; i < temp.numChildren; i++) {
		input.depth       = temp.depth+1;
		input.numChildren = temp.numChildren;
		input.factor      = temp.factor;

		if(i == pivot) input.subtree = false;
		else input.subtree = true;

		partialCount[i] = parTreeSearch(&input);
		//printf("[%d] %llu\n", i, partialCount[i]);
	}

	for (i = 0; i < temp.numChildren; i++) {
		subtreesize += partialCount[i];
	}

	return subtreesize;
}

void parallel_uts( int children, int factor ) {
	struct thread_data input;
	counter_t num_nodes = 0;

	input.depth        = 1;
	input.numChildren  = children;
	input.factor       = factor;
	input.subtree      = true;

	//printf("main: %d %d %d\n", input.depth, input.numChildren, input.factor);
	printf("numChildren: %d, factor: %d, ", input.numChildren, input.factor);
	num_nodes = parTreeSearch(&input);

	printf("Tree size = %llu\n", (unsigned long long)num_nodes );
	
}

int main(int argc, char *argv[])
{
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
	parallel_uts( children, factor );

	return 0;
}

