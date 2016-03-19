#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <omp.h>
#include <time.h>

//static double num_steps=1000;
static double num_steps;
double step, pi;

double pi_calculate( double counter, double sum );

static void _usage(FILE *f, int error) {
  fprintf(f, "Usage: ./omp NUMBER\n");
  fflush(f);
}

int main(int argc, char *argv[])
{
	int i,j;
	double x, sum=0;

	argc -= optind;
	argv += optind;

	switch (argc) {
		case 0:
			fprintf(stderr, "\nMissing pi number.\n"); // fall through
		default:
			_usage(stderr, 0);
		case 1:
			num_steps = atof(argv[0]);
			break;
	}

	step = 1.0/ (double)num_steps;

#pragma omp parallel
#pragma omp single
	{
#ifdef _OPENMP
	double start = omp_get_wtime();;
#else
	clock_t start = clock();
#endif
	sum = pi_calculate( 0, 0 );
	pi = step*sum;
#ifdef _OPENMP
	double time = omp_get_wtime() - start;
	printf( "Steps: %f; Pi: %f; Sec: %f\n", num_steps, pi, time );
#else
	double end = clock();
	float time = (float)(end-start) / CLOCKS_PER_SEC;
	printf( "Steps: %f; Pi: %f; Sec: %f\n", num_steps, pi, time );
#endif
	}

	return 0;
}

double pi_calculate( double counter, double sum )
{
	double x, result=0, result_2=0;

//printf("counter =%f/%f\n", counter, num_steps);
	if( counter < num_steps ) {
		x = (counter + 0.5) * step;
		result = sum + 4.0/(1.0+x*x);
		#pragma omp task shared(result_2) firstprivate( counter, result )
		{
		result_2 = pi_calculate( counter+1, result );
		}
		#pragma omp taskwait
		return result_2;
	}
	else
		return sum;
}


