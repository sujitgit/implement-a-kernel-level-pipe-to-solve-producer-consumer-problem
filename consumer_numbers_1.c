#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAXLEN 10

int main(int argc, char *argv[])
{

	int fd;
	char numstr[MAXLEN];
	int num_read;

	//if( argc != 2) {
		//printf("Usage: %s <numpipe_name>\n", argv[0]);
		//exit(1);
	//}
	//fd = open("/dev/mytime", O_RDONLY);
	if ( (fd = open("/dev/numpipe", O_RDONLY)) < 0) {
		printf("There was an error opening the file, fd is %d\n", fd);
		exit(1);
	}

	while(1) {
		// read a line
		ssize_t ret = read(fd, &num_read, sizeof(int));
		if( ret > 0) {
			printf("\nNumber read: %d ", num_read);
			printf("Bytes read: %ld\n", ret);
			sleep(1);
		} else {
			fprintf(stderr, "\nerror reading ret=%ld errno=%d perror: ", ret, errno);
			perror("");
			//sleep(1);
		}
	}
	close(fd);

	return 0;
}

