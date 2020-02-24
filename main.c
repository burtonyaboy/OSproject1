#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

struct child_in
{
	double start;
	double end;
	unsigned long range;
};

typedef struct child_out 
{
	double result;
	clock_t cc;
} Cout;

long ms_cpu_time(clock_t cc)
{
	return (unsigned long) (cc*1000) / CLOCKS_PER_SEC;
}

int main(int argc, const char **argv)
{
	// default number of processes
	int processn = 1, is_parent = 1, child_num = 0;
	unsigned long calc_num = 10000;
	// max 8 processes for now, later we can dynamically allocate this
	pid_t process_ids[8];

	// now make the pipe handles, this will be dynamically allocated soon
	int p[16][2];

	clock_t start_t = clock();
	
	struct timespec spec;
	clock_gettime(CLOCK_MONOTONIC,&spec);

	/* Begin validating input */

	// Make sure we have all of the inputs we need
	if(argc < 3)
	{
		printf("Usage: %s <process-count> <calculations>\n", argv[0]);
		return -1;
	}

	/* Check the process count */
	// Make sure that the process count is actually a positive integer
	if(0 >= (processn = atoi(argv[1])))
	{
		printf("Error: %s is not an integer greater than zero\n", argv[1]);
		return -1;
	}
	else if(processn > 8)
	{
		printf("Warning: Program does not currently accept %d processes. The program will execute with a single process.\n", processn);
		processn = 1;
	}

	/* Check the calculation count */
	// Make sure calculation count is actually a positive integer
	// TODO: change atoi to something that returns a long
	if(0 >= (calc_num = (unsigned long) atoi(argv[2])))
	{
		printf("Error: %s is not an integer greater than zero.\n", argv[2]);
		return -1;
	}
	else if(calc_num % processn != 0)
	{
		printf("Warning: The number of calculations is not divisible by the number of processes. This may lead to unexpected results.\n");
	}

	// Create the pipes
	for(int i = 0; i < 16; i++)
	{
		// Make the pipes. The function returns < 0 if something fails
		// so we check that.
		if(pipe(p[i]) < 0)
		{
			printf("Error: Couldn't create pipe file handles.\n");
			return -1;
		}
	}

	// Create the child processes
	for(int i = 0; i < processn; i++)
	{
		// We only want the parent to be creating children
		if(is_parent)
			if((process_ids[i] = fork()) < 0)
			{
				printf("Error: Couldn't create child process.\n");
				return -1;
			}
		
		// Tell the child that they are in fact a child.
		if(process_ids[i] == 0)
		{
			is_parent = 0;
			child_num = i;
			break;
		}
	}

	//This is what the parent does
	if(is_parent)
	{
		double result = 0.0;
		clock_t total_cc;
		Cout tmp;

		for(int i = 0; i < processn; i++)
		{
			// Allocate enough memory for a child_in structure
			// and save a pointer to it
			struct child_in *c = (struct child_in *) malloc(sizeof(struct child_in));
			
			// Populate the child_in struct with numbers it needs
			// for calculations
			c->start = ((float)i)/processn;
			c->end = ((float)i+1)/processn - 1.0/calc_num;
			c->range = calc_num / processn;
			
			// Write into the appropriate pipe
			// the address of the structure we created
			// and last the size of the structure we are passing
			write(p[i][1], c, sizeof(struct child_in));

			// Release the write pipe
			close(p[i][1]);

			// Free the memory we allocated
			free(c);
		}
		
		//Now read the results :)
		for(int i = 0; i < processn; i++)
		{
			read(p[i+8][0], &tmp, sizeof(Cout));
			
			result += tmp.result;
			total_cc += tmp.cc;
			
			printf("Child %d took %lu ms CPU time\n", i, ms_cpu_time(tmp.cc));
			
			close(p[i+8][0]);
		}

		printf("Result is: %f\n",result);
		
		long total_t, ms_start = spec.tv_nsec;
		int s_start = spec.tv_sec;
		clock_gettime(CLOCK_MONOTONIC, &spec);
		
		//totalt_t = time(0) - startt_t;
		
		if(spec.tv_sec - s_start >= 1)
			total_t = 1000000000 * (spec.tv_sec - s_start) + (spec.tv_nsec - ms_start);
		else
			total_t = spec.tv_nsec - ms_start;


		printf("The program took %lu ms CPU time to execute.\n", ms_cpu_time(total_cc));
		printf("The program took %lu milliseconds to execute.\n", (total_t)/1000000);
	}
	//This is what the children do
	else
	{
		start_t = clock();
		// Allocate some memory for the data that will be read
		struct child_in *c = (struct child_in *) malloc(sizeof(struct child_in));
		Cout child_out;
		// Attempt to read from pipe
		if(read(p[child_num][0], c, sizeof(struct child_in)) < 0)
			printf("Process %d couldn't read from parent!!!\n", child_num);

		// Print the results (for test purposes)
		printf("I am process %d and I do start: %f end: %f range: %lu\n", child_num, c->start, c->end, c->range);
		
		/* Do the calculations */
		double result = 0.0;
		// Start and end at values given by parent, increment one step each time.
		for(double n = c->start; n <= c->end; n += 1.0/c->range)
		{
			// C has a function for hyperbolic tangents defined in math.h
			result += tanh(n);
		}

		child_out.result = result;
		child_out.cc = clock() - start_t; 

		write(p[child_num + 8][1], &child_out, sizeof(Cout));
		// Free unused resources
		free(c);

		close(p[child_num][0]);
		close(p[child_num+8][1]);
	}

	// Program finished successfully. Return 0 to let calling program know.
	return 0;
	
}
