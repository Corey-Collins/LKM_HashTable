#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define DEVICE "/dev/mydevice"

int main(){

	int bytes_written, file_descriptor;
	char command_choice, write_buf[100], write_buf_two[100], read_buff[100];

	file_descriptor = open(DEVICE, O_RDWR);

	if(file_descriptor == -1){
		printf("file %s either does not exist or has been locked by another process\n", DEVICE);
		exit(-1);
	}

	printf("r = read from device\nw = write to device\nenter command: ");
	scanf("%c", &command_choice);

	switch(command_choice){
		case 'w':
			printf("enter key:");
			scanf(" %[^\n]", write_buf);
			printf("enter data:");
			scanf(" %[^\n]", write_buf_two);
			strcat(write_buf, ":");
			strcat(write_buf, write_buf_two);
			printf("%s\n", write_buf);
			write(file_descriptor, write_buf, sizeof(write_buf));
			break;
		case 'r':
			printf("enter key:");
			scanf(" %[^\n]", read_buff);
			read(file_descriptor, read_buff, sizeof(read_buff));
			printf("device: %s\n", read_buff);
			break;
		default:
			printf("unknown command\n");
	}

	close(file_descriptor);

	return 0;
}