#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

typedef struct child_data
{
	double start;
	double end;
	unsigned int range;
};

int main(int argc, const char **argv)
{
	// default number of processes
	int pid[8], processn = 1, is_parent = 1, child_num = 0;
	unsigned int calc_num = 10000;
	// max 8 processes for now, later we can dynamically allocate this
	pid_t process_ids[8];
	//memset(procedd_ids, 1, 8*sizeof(pid_t));
	char message[] = "My parent says hi...";
	char *message_buf;
	// now make the pipe handles, this will be dynamically allocated soon
	int p[8][2];

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
	if(0 >= (calc_num = atoi(argv[2])))
	{
		printf("Error: %s is not an integer greater than zero.\n");
		return -1;
	}
	else if(calc_num % processn != 0)
	{
		printf("Warning: The number of calculations is not divisible by the number of processes. This may lead to unexpected results.\n");
	}

	// Create the pipes
	for(int i = 0; i < 8; i++)
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
		for(int i = 0; i < processn; i++)
		{
			/* Set up child data */

			// Allocate enough memory for a child_data structure
			// and save a pointer to it
			struct child_data *c = (struct child_data *) malloc(sizeof(struct child_data));
			
			// Populate the child_data struct with numbers it needs
			// for calculations
			c->start = ((float)i)/processn;
			c->end = ((float)i+1)/processn - 1.0/calc_num;
			c->range = calc_num / processn;
			
			/* Write data for child into the pipe */

			// Write into the appropriate pipe
			// the address of the structure we created
			// and last the size of the structure we are passing
			write(p[i][1], c, sizeof(struct child_data));

			/* Free unused resources */
			// Release the write pipe
			close(p[i][1]);
			// Free the memory we allocated
			free(c);
		}
	}
	//This is what the children do
	else
	{
		// Allocate some memory for the data that will be read
		struct child_data *c = (struct child_data *) malloc(sizeof(struct child_data));

		// Attempt to read from pipe
		if(read(p[child_num][0], c, sizeof(struct child_data)) < 0)
			printf("Process %d couldn't read from parent!!!\n", child_num);

		// Print the results (for test purposes)
		printf("I am process %d and I do start: %f end: %f range: %d\n", child_num, c->start, c->end, c->range);
		
		/* TODO: add calculations */
		
		// Free unused resources
		free(c);
		close(p[child_num][0]);
	}

	// Program finished successfully. Return 0 to let calling program know.
	return 0;
	
}
