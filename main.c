#include <stdio.h>

int main(int argc, const char **argv)
{
	int processn = 1;
	
	if(argc < 2)
	{
		printf("Error: not enough arguments supplied\n");
		return -1;
	}

	if(0 >= (processn = atoi(argv[1])))
	{
		printf("Error: %s is not an integer", argv[1]);
	}
}
