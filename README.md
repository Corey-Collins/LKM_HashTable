# Hash Table implemented with Linux Kernel Module

## Instructions

Hash Table is contained in /HASHDRIVER/

User synchronization program is contained in /APP/

To insert hashtable into kernel:
    $ make
    $ (su) insmod mymodule.ko

To check hashtable output and status:
	$ dmesg

To remove hashtable from kernel:
	$ (su) remmod mymodule.ko

To run user program:
	$ make
	$ ./userapp

To clean any project files:
	$ make clean

## How it works

The Linux Kernel Module (LKM) was implemented using a character driver. This seemed to be the obvious choice since it provides file operations and synchronization. The user program runs in user space, performs file operations consisting of open, write, read, close to a device file. This file is an object created in the system and transfers these file operations to the kernel space where the device driver (LKM hashtable character driver) is stored. The device driver will parse the request modify physical space accordingly.

The LKM device driver has a semaphore lock (size 1) that is activated upon write operations. You can test a basic read / write in the userapp by running commands 'r' or 'w' and followig directions. You can test the synchronization portion by running the 't' command. This command forks 8 process that all write the value 20 in the key of the process they are in (e.g. process 0 stores 0,20; process 7 stores 7;20) then tries to read from the key of the process before it (e.g process 0 attempts to read key: -1; process 6 makes a read from key: 5). Notice in the kernel log '$dmesg' that reads are performed in between writes, since the locking mechanism is in place only when data is being added to the hashtable. 

The user program can save current hashtable in memory to persistent storage by utilizing the 's' command. Upon receiving this command, the kernel will parse the hashtable into char array format and return it to user space. In user space the userapp will store that array as a file under 'hash_data.txt' which you can view with '$cat hash_data.txt'.

## The Good

There is a lot that I liked about this project. The best was learning how to implement a device driver. In previous courses I’ve used microcontroller RX/TX serializer pins to connect to Linux Systems via /dev/TTY#. Now I understand more what the purpose of these major / minor numbers are for and why they are needed. I also understand how most people provide additional functionality to linux kernels which is a huge plus for me in my future as a software developer. Overall I felt as if I was learning something that is important and will be of use for me in the future.

## The Bad (and conclusion)

While most things I did like about this project, there were some I didn’t like so much. The first was knowing where to begin. I started by trying to understand an LKM, which I learned through a series of tutorials online. Luckily as I advanced in those tutorials they provided much more insight as to how you can use an LKM to create drivers in kernel that connect to devices that user’s create to access these drivers in kernel space. I figured the best way to provide synchronization and Hash storage in a descent way was over a character driver, since the other version ‘block driver’ behaves asynchronously in comparison. After getting a basic character driver working the toughest part was using it’s file operation interface to adapt to my integer-based Hash storage. This requires a lot of C String parsing and string < -- > int casting which is my least favorite by far. What made it worse was that I did not have high level C libraries to my disposal for this. Along with that, much debugging was needed which required to constantly remove the module from kernel, re-compile, and re-insert the module back into the kernel which got frustrating after a thousand tries. 		

But these difficulties are what I saw as challenges, and these challenges felt great to overcome. As I finished up the project I realized I had basically used every topic learned from this course in the final software implementation. I’m glad it turned out better than I expected and I hope to see myself writing an LKM in the real world beyond academics. 