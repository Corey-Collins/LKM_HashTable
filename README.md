# Hash Table implemented with Linux Kernel Module


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
