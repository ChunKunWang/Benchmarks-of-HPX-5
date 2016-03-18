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
#include <time.h>
#include <cilk/cilk.h>

#define   MAXSIZE     10      /* max. board size          */
#define   MAX_STACK  1000     /* stack size = board-size^2*/
#define   SUCCESS      1      /* return value for a succ. */
#define   FAILURE      0      /* for a failure.           */
#define   EMPTY       -1      /* value to indicate empty  */

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

/* ----------------- function prototype ----------------- */

void  initial(void);
void  display(void);
int   trygo(int temp);

/* -------------------- main program -------------------- */

int main(int input, char **argv)
{
	int   row, column, top, i;
	int   result;

	__cilkrts_set_param("stack size", "2000000");

	if (input <= 1 || (input = atoi(argv[1])) <= 0) input = 3;
	//char  line[256];
	//printf("\nRecursive Knight Tour Problem");
	//printf("\n=============================");
	n = input;
	printf("Size: %d; ", n );
	//gets(line);
	//n = atoi(line);
	row = 0;
	//printf( "Start Row -----> %d\n", row);
	//gets(line);
	//row = atoi(line);
	column = 0;
	//printf( "Start Column --> %d\n", column);
	//gets(line);
	//column = atoi(line);

	initial();
	top = -1;                /* initial to empty         */
	PUSH(row, column);       /* push first pos. and dir. */

	clock_t start = clock();
	result = trygo(top);
	clock_t end = clock();

	double duration = (double)(end - start) / CLOCKS_PER_SEC;
	printf("%f sec\n", duration);

	if (result == FAILURE)
		printf("NO SOLUTION AT ALL.\n");
	else
		;//display();

	//printf("test\n");

	return 0;
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

int  trygo(int temp)
{
	int  new_x, new_y;
	int  top = temp;
	int  found;
	int  found_2;

	//top = -1;                /* initial to empty         */
	//PUSH(x, y);              /* push first pos. and dir. */	

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

		found_2 = cilk_spawn trygo( top );

		cilk_sync;
		return found_2;
	}

	return SUCCESS;          /* all pos. visited. DONE   */
}






