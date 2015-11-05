/******************************************************************* 
 *  Health.c : Model of the Colombian Health Care System           *
 *******************************************************************/ 
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <omp.h>
#include "health_hpx.h"

static hpx_action_t _health      = 0;
static hpx_action_t _health_main = 0;

/* global variables */
int sim_level;
int sim_cities;
int sim_population_ratio;
int sim_time;
int sim_assess_time;
int sim_convalescence_time;
long sim_seed;
float sim_get_sick_p;
float sim_convalescence_p;
float sim_realloc_p;
int sim_pid = 0;

int res_population;
int res_hospitals;
int res_personnel;
int res_checkin;
int res_village;
int res_waiting;
int res_assess;
int res_inside;
float res_avg_stay;

/**********************************************************
 * Handles math routines for health.c                     *
 **********************************************************/
float my_rand(long *seed) 
{
	long k;
	long idum = *seed;

	idum ^= MASK;
	k = idum / IQ;
	idum = IA * (idum - k * IQ) - IR * k;
	idum ^= MASK;
	if (idum < 0) idum  += IM;
	*seed = idum * IM;
	return (float) AM * idum;
}
/********************************************************************
 * Handles lists.                                                   *
 ********************************************************************/
void addList(struct Patient **list, struct Patient *patient)
{
	if (*list == NULL)
	{
		*list = patient;
		patient->back = NULL;
		patient->forward = NULL;
	}
	else
	{
		struct Patient *aux = *list;
		while (aux->forward != NULL) aux = aux->forward; 
		aux->forward = patient;
		patient->back = aux;
		patient->forward = NULL;
	}
} 
void removeList(struct Patient **list, struct Patient *patient) 
{
	if (patient->back != NULL) patient->back->forward = patient->forward;
	else *list = patient->forward;
	if (patient->forward != NULL) patient->forward->back = patient->back;
}
/**********************************************************************/
void allocate_village( struct Village **capital, struct Village *back, struct Village *next, int level, long vid)
{ 
	int i, population, personnel;
	struct Village *current, *inext;
	struct Patient *patient;

	if (level == 0) *capital = NULL;
	else
	{
		personnel = (int) pow(2, level);
		population = personnel * sim_population_ratio;
		/* Allocate Village */
		*capital = (struct Village *) malloc(sizeof(struct Village));
		/* Initialize Village */
		(*capital)->back  = back;
		(*capital)->next  = next;
		(*capital)->level = level;
		(*capital)->id    = vid;
		(*capital)->seed  = vid * (IQ + sim_seed);
		(*capital)->population = NULL;
		for(i=0;i<population;i++)
		{
			patient = (struct Patient *)malloc(sizeof(struct Patient));
			patient->id = sim_pid++;
			patient->seed = (*capital)->seed;
			// changes seed for capital:
			my_rand(&((*capital)->seed));
			patient->hosps_visited = 0;
			patient->time          = 0;
			patient->time_left     = 0;
			patient->home_village = *capital; 
			addList(&((*capital)->population), patient);
		}
		/* Initialize Hospital */
		(*capital)->hosp.personnel = personnel;
		(*capital)->hosp.free_personnel = personnel;
		(*capital)->hosp.assess = NULL;
		(*capital)->hosp.waiting = NULL;
		(*capital)->hosp.inside = NULL;
		(*capital)->hosp.realloc = NULL;
		omp_init_lock(&(*capital)->hosp.realloc_lock);
		//(*capital)->hosp.mutex = hpx_lco_sema_new(1);
		// Create Cities (lower level)
		inext = NULL;
		for (i = sim_cities; i>0; i--)
		{
			allocate_village(&current, *capital, inext, level-1, (vid * (long) sim_cities)+ (long) i);
			inext = current;
		}
		(*capital)->forward = current;
	}
}
/**********************************************************************/
struct Results get_results(struct Village *village)
{
	struct Village *vlist;
	struct Patient *p;
	struct Results t_res, p_res;

	t_res.hosps_number     = 0.0;
	t_res.hosps_personnel  = 0.0;
	t_res.total_patients   = 0.0;
	t_res.total_in_village = 0.0;
	t_res.total_waiting    = 0.0;
	t_res.total_assess     = 0.0;
	t_res.total_inside     = 0.0;
	t_res.total_hosps_v    = 0.0;
	t_res.total_time       = 0.0;

	if (village == NULL) return t_res;

	/* Traverse village hierarchy (lower level first)*/
	vlist = village->forward;
	while(vlist)
	{
		p_res = get_results(vlist);
		t_res.hosps_number     += p_res.hosps_number;
		t_res.hosps_personnel  += p_res.hosps_personnel;
		t_res.total_patients   += p_res.total_patients;
		t_res.total_in_village += p_res.total_in_village;
		t_res.total_waiting    += p_res.total_waiting;
		t_res.total_assess     += p_res.total_assess;
		t_res.total_inside     += p_res.total_inside;
		t_res.total_hosps_v    += p_res.total_hosps_v;
		t_res.total_time       += p_res.total_time;
		vlist = vlist->next;
	}
	t_res.hosps_number     += 1.0;
	t_res.hosps_personnel  += village->hosp.personnel;

	// Patients in the village
	p = village->population;
	while (p != NULL) 
	{
		t_res.total_patients   += 1.0;
		t_res.total_in_village += 1.0;
		t_res.total_hosps_v    += (float)(p->hosps_visited);
		t_res.total_time       += (float)(p->time); 
		p = p->forward; 
	}
	// Patients in hospital: waiting
	p = village->hosp.waiting;
	while (p != NULL) 
	{
		t_res.total_patients += 1.0;
		t_res.total_waiting  += 1.0;
		t_res.total_hosps_v  += (float)(p->hosps_visited);
		t_res.total_time     += (float)(p->time); 
		p = p->forward; 
	}
	// Patients in hospital: assess
	p = village->hosp.assess;
	while (p != NULL) 
	{
		t_res.total_patients += 1.0;
		t_res.total_assess   += 1.0;
		t_res.total_hosps_v  += (float)(p->hosps_visited);
		t_res.total_time     += (float)(p->time); 
		p = p->forward; 
	}
	// Patients in hospital: inside
	p = village->hosp.inside;
	while (p != NULL) 
	{
		t_res.total_patients += 1.0;
		t_res.total_inside   += 1.0;
		t_res.total_hosps_v  += (float)(p->hosps_visited);
		t_res.total_time     += (float)(p->time); 
		p = p->forward; 
	}  

	return t_res; 
}
/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
void check_patients_inside(struct Village *village) 
{
	struct Patient *list = village->hosp.inside;
	struct Patient *p;

	while (list != NULL)
	{
		p = list;
		list = list->forward; 
		p->time_left--;
		if (p->time_left == 0) 
		{
			village->hosp.free_personnel++;
			removeList(&(village->hosp.inside), p); 
			addList(&(village->population), p); 
		}    
	}
}
/**********************************************************************/
void check_patients_assess_par(struct Village *village) 
{
	struct Patient *list = village->hosp.assess;
	float rand;
	struct Patient *p;

	while (list != NULL) 
	{
		p = list;
		list = list->forward; 
		p->time_left--;

		if (p->time_left == 0) 
		{ 
			rand = my_rand(&(p->seed));
			/* sim_covalescense_p % */
			if (rand < sim_convalescence_p)
			{
				rand = my_rand(&(p->seed));
				/* !sim_realloc_p % or root hospital */
				if (rand > sim_realloc_p || village->level == sim_level) 
				{
					removeList(&(village->hosp.assess), p);
					addList(&(village->hosp.inside), p);
					p->time_left = sim_convalescence_time;
					p->time += p->time_left;
				}
				else /* move to upper level hospital !!! */
				{
					village->hosp.free_personnel++;
					removeList(&(village->hosp.assess), p);
					omp_set_lock(&(village->hosp.realloc_lock));
					//hpx_lco_sema_p(village->hosp.mutex);
					addList(&(village->back->hosp.realloc), p); 
					//hpx_lco_sema_v_sync(village->hosp.mutex);
					omp_unset_lock(&(village->hosp.realloc_lock));
				} 
			}
			else /* move to village */
			{
				village->hosp.free_personnel++;
				removeList(&(village->hosp.assess), p);
				addList(&(village->population), p); 
			}
		}
	} 
}
/**********************************************************************/
void check_patients_waiting(struct Village *village) 
{
	struct Patient *list = village->hosp.waiting;
	struct Patient *p;

	while (list != NULL) 
	{
		p = list;
		list = list->forward; 
		if (village->hosp.free_personnel > 0) 
		{
			village->hosp.free_personnel--;
			p->time_left = sim_assess_time;
			p->time += p->time_left;
			removeList(&(village->hosp.waiting), p);
			addList(&(village->hosp.assess), p); 
		}
		else 
		{
			p->time++;
		}
	} 
}
/**********************************************************************/
void check_patients_realloc(struct Village *village)
{
	struct Patient *p, *s;

	while (village->hosp.realloc != NULL) 
	{
		p = s = village->hosp.realloc;
		while (p != NULL)
		{
			if (p->id < s->id) s = p;
			p = p->forward;
		}
		removeList(&(village->hosp.realloc), s);
		put_in_hosp(&(village->hosp), s);
	}
}
/**********************************************************************/
void check_patients_population(struct Village *village) 
{
	struct Patient *list = village->population;
	struct Patient *p;
	float rand;

	while (list != NULL) 
	{
		p = list;
		list = list->forward; 
		/* randomize in patient */
		rand = my_rand(&(p->seed));
		if (rand < sim_get_sick_p) 
		{
			removeList(&(village->population), p);
			put_in_hosp(&(village->hosp), p);
		}
	}

}
/**********************************************************************/
void put_in_hosp(struct Hosp *hosp, struct Patient *patient) 
{  
	(patient->hosps_visited)++;

	if (hosp->free_personnel > 0) 
	{
		hosp->free_personnel--;
		addList(&(hosp->assess), patient); 
		patient->time_left = sim_assess_time;
		patient->time += patient->time_left;
	} 
	else 
	{
		addList(&(hosp->waiting), patient); 
	}
}
/**********************************************************************/
static int _health_action(void *args, size_t size) 
{
	return HPX_SUCCESS;
}

void sim_village_par(struct Village *village)
{
	struct Village *vlist;
	int counter = 0;

	// lowest level returns nothing
	// only for sim_village first call with village = NULL
	// recursive call cannot occurs
	if (village == NULL) return;

	vlist = village->forward;
	while(vlist) {
		counter++;
		vlist = vlist->next;
	}

	/* Traverse village hierarchy (lower level first)*/
	vlist = village->forward;
	while(vlist)
	{
//#pragma omp task untied
		sim_village_par(vlist);
		vlist = vlist->next;
	}

	/* Uses lists v->hosp->inside, and v->return */
	check_patients_inside(village);

	/* Uses lists v->hosp->assess, v->hosp->inside, v->population and (v->back->hosp->realloc) !!! */
	check_patients_assess_par(village);

	/* Uses lists v->hosp->waiting, and v->hosp->assess */
	check_patients_waiting(village);

//#pragma omp taskwait
	printf("counter=%d; level=%d\n", counter, village->level);

	/* Uses lists v->hosp->realloc, v->hosp->asses and v->hosp->waiting */
	check_patients_realloc(village);

	/* Uses list v->population, v->hosp->asses and v->h->waiting */
	check_patients_population(village);
}
/**********************************************************************/
void my_print(struct Village *village)
{
	struct Village *vlist;
	struct Patient *plist;

	if (village == NULL) return;

	/* Traverse village hierarchy (lower level first)*/
	vlist = village->forward;
	while(vlist) {
		my_print(vlist);
		vlist = vlist->next;
	}

	plist = village->population;

	while (plist != NULL) {
		printf("[pid:%d]",plist->id);
		plist = plist->forward; 
	}
	printf("[vid:%d]\n",village->id);

}
/**********************************************************************/
void read_input_data(char *filename)
{
	FILE *fin;
	int res;

	if ((fin = fopen(filename, "r")) == NULL) {
		printf("Could not open sequence file (%s)\n", filename);
		exit (-1);
	}
	res = fscanf(fin,"%d %d %d %d %d %d %ld %f %f %f %d %d %d %d %d %d %d %d %f", 
			&sim_level,
			&sim_cities,
			&sim_population_ratio,
			&sim_time, 
			&sim_assess_time,
			&sim_convalescence_time,
			&sim_seed, 
			&sim_get_sick_p,
			&sim_convalescence_p,
			&sim_realloc_p,
			&res_population,
			&res_hospitals,
			&res_personnel,
			&res_checkin,
			&res_village,
			&res_waiting,
			&res_assess,
			&res_inside,
			&res_avg_stay
		    );
	if ( res == EOF ) {
		printf("Bogus input file (%s)\n", filename);
		exit(-1);
	}
	fclose(fin);

	// Printing input data
	printf("\n");
	printf("Number of levels    = %d\n", (int) sim_level);
	printf("Cities per level    = %d\n", (int) sim_cities);
	printf("Population ratio    = %d\n", (int) sim_population_ratio);
	printf("Simulation time     = %d\n", (int) sim_time);
	printf("Assess time         = %d\n", (int) sim_assess_time);
	printf("Convalescence time  = %d\n", (int) sim_convalescence_time);
	printf("Initial seed        = %d\n", (int) sim_seed);
	printf("Get sick prob.      = %f\n", (float) sim_get_sick_p);
	printf("Convalescence prob. = %f\n", (float) sim_convalescence_p);
	printf("Realloc prob.       = %f\n", (float) sim_realloc_p);
}
int check_village(struct Village *top)
{
	struct Results result = get_results(top);
	int answer = 1;

	if (res_population != result.total_patients) answer = 2;
	if (res_hospitals != result.hosps_number) answer = 2;
	if (res_personnel != result.hosps_personnel) answer = 2;
	if (res_checkin != result.total_hosps_v) answer = 2;
	if (res_village != result.total_in_village) answer = 2;
	if (res_waiting != result.total_waiting) answer = 2;
	if (res_assess != result.total_assess) answer = 2;
	if (res_inside != result.total_inside) answer = 2;

	printf("\n");
	printf("Sim. Variables      = expect / result\n");
	printf("Total population    = %6d / %6d people\n", (int)   res_population, (int) result.total_patients);
	printf("Hospitals           = %6d / %6d people\n", (int)   res_hospitals, (int) result.hosps_number);
	printf("Personnel           = %6d / %6d people\n", (int)   res_personnel, (int) result.hosps_personnel);
	printf("Check-in's          = %6d / %6d people\n", (int)   res_checkin, (int) result.total_hosps_v);
	printf("In Villages         = %6d / %6d people\n", (int)   res_village, (int) result.total_in_village);
	printf("In Waiting List     = %6d / %6d people\n", (int)   res_waiting, (int) result.total_waiting);
	printf("In Assess           = %6d / %6d people\n", (int)   res_assess, (int) result.total_assess);
	printf("Inside Hospital     = %6d / %6d people\n", (int)   res_inside, (int) result.total_inside);
	printf("Average Stay        = %6f / %6f u/time\n", (float) res_avg_stay,(float) result.total_time/result.total_patients);

	if( answer == 1 ) printf( "RESULT_SUCCESSFUL!\n" );
	else {
		//my_print(top);
		printf( "RESULT_UNSUCCESSFUL!\n" );
	}


	return 0;
}
/**********************************************************************/
void sim_village_main_par(struct Village *top)
{
	long i;

#ifdef _OPENMP
		double start = omp_get_wtime();;
#endif

//#pragma omp parallel
//#pragma omp single
//#pragma omp task untied
	for (i = 0; i < sim_time; i++) sim_village_par(top);   

#ifdef _OPENMP
		double time = omp_get_wtime() - start;
		printf("OpenMP Work took %f sec.\n", time);
#endif
}

static void _usage(FILE *f, int error) {
	fprintf(f, "Usage: hpx-5 [options] INPUT_FILE\n");
	//hpx_print_help();
	fflush(f);
	exit(error);
}

static int _health_main_action(void *args, size_t size)
{
	char *ptr = (char *)args;
	struct Village *top; 

	read_input_data(ptr);

	allocate_village(&top, ((void *)0), ((void *)0), sim_level, 0);;

	sim_village_main_par(top);;

	check_village(top);

	hpx_exit(HPX_SUCCESS);
}

int main(int argc, char *argv[])
{
	char bots_arg_file[255]="./input/small.input";

	if (hpx_init(&argc, &argv) != 0) {
                fprintf(stderr, "HPX: failed to initialize.\n");
                return -1;
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
			fprintf(stderr, "\nMissing health input.\n"); // fall through
		default:
			_usage(stderr, EXIT_FAILURE);
		case 1:
			strcpy(bots_arg_file, argv[0] );
			break;
	}

	HPX_REGISTER_ACTION(HPX_DEFAULT, HPX_MARSHALLED, _health, _health_action,
			HPX_POINTER, HPX_SIZE_T);
	HPX_REGISTER_ACTION(HPX_DEFAULT, HPX_MARSHALLED, _health_main, _health_main_action,
			HPX_POINTER, HPX_SIZE_T);

	int t = hpx_run(&_health_main, &bots_arg_file, sizeof(bots_arg_file));
	hpx_finalize();
	return t;
}


