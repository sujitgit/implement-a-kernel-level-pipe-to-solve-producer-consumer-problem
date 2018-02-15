#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>

#define MAXLEN 10

int main(int argc, char *argv[])
{
	int fd;
	char numstr[MAXLEN];
	int count = 0;
	int num_to_write;

	//if( argc != 2) {
	//	printf("Usage: %s <numpipe_name>\n", argv[0]);
	//	exit(1);
	//}	

	if ( (fd = open("/dev/numpipe", O_WRONLY)) < 0) {
		printf("There was an error opening the file, fd is %d\n", fd);
		exit(1);
	} 
	
	// Prevent producer from dying due to SIGPIPE when last consumer quits
	signal(SIGPIPE, SIG_IGN); 

	while(1) {
		bzero(numstr, MAXLEN);
		sprintf(numstr, "%d%d\n", getpid(), count++);
		num_to_write = atoi(numstr);
		printf("Writing: %d", num_to_write);

		// write to pipe
		ssize_t ret = write(fd, &num_to_write, sizeof(int));
		if ( ret < 0) {
			fprintf(stderr, "error writing ret=%ld errno=%d perror: ", ret, errno);
			perror("");
		} else {
			printf("Bytes written: %ld\n", ret);
		}
		sleep(1);
	}

	close(fd);

	return 0;
}

