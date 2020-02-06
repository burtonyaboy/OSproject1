#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, const char **argv)
{
	// default number of processes
	int pid[8], processn = 1, is_parent = 1;
	// max 8 processes for now, later we can dynamically allocate this
	pid_t process_ids[8];
	//memset(procedd_ids, 1, 8*sizeof(pid_t));

	// now make the pipe handles, this will be dynamically allocated soon
	int p[8][2];


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

	// Create the pipes
	for(int i = 0; i < 8; i++)
	{
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
			process_ids[i] = fork();
		
		// Tell the child that they are in fact a child.
		if(process_ids[i] == 0)
		{
			is_parent = 0;
			break;
		}
	}

	// A bit of testing
	printf("Hi! I am process number %d", getpid());
	if(is_parent)
	{
		printf(", I am the parent.\n");
	}
	else
	{
		printf(", I am a child.\n");
	}

	return 0;
	
}
