#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define DEVICE "/dev/mydevice"

int main(){

	int bytes_written, file_descriptor, process;
	char command_choice, write_buf[100], write_buf_two[100], read_buff[100];

	file_descriptor = open(DEVICE, O_RDWR);

	if(file_descriptor == -1){
		printf("file %s either does not exist or has been locked by another process\n", DEVICE);
		exit(-1);
	}

	printf("r = read from device\nw = write to device\nt = threaded processes\ns = save to file\nenter command: ");
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
		case 's':
			strcat(read_buff, "s");
			read(file_descriptor, read_buff, sizeof(read_buff));
			FILE *file = fopen("hash_data.txt", "w");
			int results = fputs(read_buff, file);
			if(results == EOF) {
				printf("failed to save file!\n");
			} else {
				printf("save successful.\n");
			}
			fclose(file);
			break;
		case 't':
			if(fork() == 0){
				process = 1;
				if (fork() == 0){
					process = 2;
					if (fork() == 0){
						process = 3;
						if (fork() == 0){
							process = 4;
							if (fork() == 0){
								process = 5;
								if (fork() == 0){
									process = 6;
									if (fork() == 0){
										process = 7;
									}
								}
							}
						}
					}
				}
			} else {
				process = 0;
			}
			printf("I am process %i\n", process);
			
			//write process:20
			char* snum;
			snprintf(snum, sizeof(snum), "%i", process);
			strcat(write_buf, snum);
			strcat(write_buf, ":20");
			write(file_descriptor, write_buf, sizeof(write_buf));
			printf("wrote: %s\n", write_buf);

			//read for key:process - 1
			snprintf(snum, sizeof(snum), "%i", process-1);
			strcat(read_buff, snum);
			printf("reading: %s\n", read_buff);
			read(file_descriptor, read_buff, sizeof(read_buff));
			printf("device: %s\n", read_buff);

			break;
		default:
			printf("unknown command\n");
	}

	close(file_descriptor);

	return 0;
}