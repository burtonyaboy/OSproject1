#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, const char **argv)
{
	// default number of processes
	int pid, processn = 1;
	// max 8 processes for now, later we can dynamically allocate this
	pid_t process_ids[8];

	// now make the pipe handles, this will be dynamicall allocated soon
	int p[2];

	if(pipe(p) < 0)
	{
		printf("Error: Couldn't create pipe file handles.\n");
		return -1;
	}

	// lets get the number of processes from program input
	if(argc < 2)
	{
		printf("Error: not enough arguments supplied\n");
		return -1;
	}

	// Make sure that the process count is actually a positive integer
	if(0 >= (processn = atoi(argv[1])))
	{
		printf("Error: %s is not an integer\n", argv[1]);
		return -1;
	}
	else if(processn > 8)
	{
		printf("Warning: Program does not currently accept %d processes. The program will execute with a single process.", processn);
		processn = 1;
	}

	for(int i = 1; i < processn; i++)
	{
		// This is where the child process operates
		if((pid = fork()) > 0)
		{
			printf("I'm a child process\n");
		}
		// This is where the parent process operates
		else
		{
			printf("I'm the parent process\n");
		}
	}

	return 0;
	
}
