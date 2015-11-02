/* ------------------------------------------------------ */
/* PROGRAM knight tour :                                  */
/*    Given a n*n chess board and a starting position,    */
/* this program will find a knight tour path passing      */
/* through each square on the chess board exactly once by */
/* using backtrack technique without recursion.  For      */
/* larger n, for example n > 6, it will take very long    */
/* time to find such path.  Therefore be patience please. */
/*                                                        */
/* Copyright Ching-Kuang Shene               July/20/1989 */
/* ------------------------------------------------------ */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <hpx/hpx.h>

#define   MAXSIZE     10      /* max. board size          */
#define   MAX_STACK  1000     /* stack size = board-size^2*/
//#define   SUCCESS      1      /* return value for a succ. */
#define   FAILURE      0      /* for a failure.           */
#define   EMPTY       -1      /* value to indicate empty  */

static int SUCCESS = 1;

/* ------------------------------------------------------ */
/* FUNCTION try :                                         */
/*    The main non-recursive backtrack working routine.   */
/* ------------------------------------------------------ */

#define  YES          1
#define  NO           0
#define  BOARD(x,y)   (1<=x) && (x<=n) && (1<=y) && (y<=n)
#define  CHECK(x,y)   board[x][y] == EMPTY
//??
#define  PUSH(x,y)    { top++;             \
	path_x[top] = x; path_y[top] = y;  \
	direction[top] = 0;                \
	board[x][y] = top;                 \
}

/* ----------------- external variables ----------------- */

int  board[MAXSIZE+1][MAXSIZE+1];   /* chess board        */
int  n;                       /* working board size       */
int  offset_x[] = { 2,  1, -1, -2, -2, -1,  1,  2};
int  offset_y[] = { 1,  2,  2,  1, -1, -2, -2, -1};

int  path_x[MAX_STACK+1];     /* stack for x coordinate   */
int  path_y[MAX_STACK+1];     /* stack for y coordinate   */
int  direction[MAX_STACK+1];  /* stack for direction      */
int  top;                     /* stack pointer            */

/* ------------------- HPX-5 variables ------------------ */

static hpx_action_t _knight      = 0;
static hpx_action_t _knight_main = 0;

static void _usage(FILE *f, int error) {
	fprintf(f, "Usage: hpx-5 [options] NUMBER\n"
			"\t-h, show help\n");
	//hpx_print_help();
	fflush(f);
	exit(error);
}

/* ------------------------------------------------------ */
/* FUNCTION initial :                                     */
/*    initialize the chess board to EMPTY.                */
/* ------------------------------------------------------ */

void  initial(void)
{
	int  i, j;

	for (i = 1; i <= n; i++)
		for (j = 1; j <= n; j++)
			board[i][j] = EMPTY;
}

void  display(void)
{
	int  r,i;

	printf("\n\nHere is One Possible Solution :\n");
	printf("\n+");
	for (i = 1; i <= n; i++)
		printf("--+");

	for (r = 1; r <= n; r++) {
		printf("\n|");
		for (i = 1; i <= n; i++)
			printf("%2d|",board[r][i]);

		printf("\n+");
		for (i = 1; i <= n; i++)
			printf("--+");
	}
}


/* ----------------- function prototype ----------------- */

//void  initial(void);
//void  display(void);
//int   trygo(int temp);

/* -------------------- main program -------------------- */

int _knight_main_action(int *args, size_t size) {
	int   row, column, top, i;
	int   result=0;

	printf("\nRecursive Knight Tour Problem");
	printf("\n=============================");

	n = *args;
	printf("\n\nBoard Size ----> %d\n", n );

	row = 0;
	//printf( "Start Row -----> %d\n", row);

	column = 0;
	//printf( "Start Column --> %d\n", column);

	initial();
	top = -1;                /* initial to empty         */
	PUSH(row, column);       /* push first pos. and dir. */

	hpx_time_t now = hpx_time_now();
	//result = trygo(top);
	hpx_call_sync(HPX_HERE, _knight, &result, sizeof(result), &top, sizeof(top));
	double elapsed = hpx_time_elapsed_ms(now)/1e3;

	if (result == FAILURE)
		printf("\nNO SOLUTION AT ALL.");
	else
		;//display();

	printf("\n");
	printf("seconds: %.7f\n", elapsed);
	printf("localities: %d\n", HPX_LOCALITIES);
	printf("threads/locality: %d\n", HPX_THREADS);

	hpx_exit(HPX_SUCCESS);
}


int  _knight_action(int *args, size_t size) 
{
	int  top = *args;
	int  new_x, new_y;
	int  found;
	int  found_2;
	hpx_addr_t done = hpx_lco_future_new(sizeof(int));

	if( top < n*n-1 ) {
		found = NO;         
		while (direction[top] < 8) { /* try all 8 pos.  */
			new_x = path_x[top] + offset_x[direction[top]];
			new_y = path_y[top] + offset_y[direction[top]];
			if (BOARD(new_x,new_y) && CHECK(new_x,new_y)) {
				PUSH(new_x, new_y); /* a new pos. PUSH*/
				found = YES;        /* set flag       */
				break;              /* try next pos.  */
			}
			else
				direction[top]++;   /* OR try next dir*/
		}
		if (!found)         /* if no new pos. is found  */
			if (top > 0) { /* do we have prev. item?   */
				board[path_x[top]][path_y[top]] = EMPTY;
				direction[--top]++; /* YES, backtrack */
			}
			else
				return FAILURE; /* otherwise, FAILURE */
		//#pragma omp task shared(found_2) firstprivate(top)
		//{
		hpx_call(HPX_HERE, _knight, done, &top, sizeof(top));
		hpx_lco_wait(done);
		hpx_lco_get(done, sizeof(int), &found_2);
		hpx_lco_delete(done, HPX_NULL);
		//found_2 = trygo( top );
		//}
		//#pragma omp taskwait
		//return found_2;
		HPX_THREAD_CONTINUE(found_2);
	}

	//return SUCCESS;          /* all pos. visited. DONE   */
	HPX_THREAD_CONTINUE(SUCCESS);
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
				hpx_print_help();
			case '?':
			default:
				_usage(stderr, EXIT_FAILURE);
		}
	}

	argc -= optind;
	argv += optind;

	int input = 0;
	switch (argc) {
		case 0:
			fprintf(stderr, "\nMissing knight number.\n"); // fall through
		default:
			_usage(stderr, EXIT_FAILURE);
		case 1:
			input = atoi(argv[0]);
			break;
	}

	HPX_REGISTER_ACTION(HPX_DEFAULT, HPX_MARSHALLED, _knight, _knight_action, HPX_POINTER, HPX_SIZE_T);
	HPX_REGISTER_ACTION(HPX_DEFAULT, HPX_MARSHALLED, _knight_main, _knight_main_action, HPX_POINTER, HPX_SIZE_T);

	int e = hpx_run(&_knight_main, &input, sizeof(input));
	hpx_finalize();
	return e;
}

