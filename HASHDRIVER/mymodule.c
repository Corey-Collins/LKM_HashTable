#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/semaphore.h>
#include <linux/uaccess.h>
#include <linux/string.h>
#include <asm/uaccess.h>
#include <linux/vmalloc.h>


struct fake_device {
	struct semaphore sem;
} virtual_device;

struct cdev *mcdev;
int major_number;
int ret;

dev_t dev_num;

#define DEVICE_NAME "mydevice"


long key;
long value;

void updateKeyValuePair(char *input) {

	char *chkey;
	char *chvalue;
	char *ptr;

	chkey = strsep(&input, ":");

	printk(KERN_INFO "mydevice: chkey is %s\n", chkey);

	chvalue = strsep(&input, ":");

	printk(KERN_INFO "mydevice: chvalue is %s\n", chvalue);

	key = simple_strtol(chkey, &ptr, 10);
	value = simple_strtol(chvalue, &ptr, 10);

}

void updateKey(char *input) {

	char *chkey;
	char *ptr;

	chkey = strsep(&input, ":");

	key = simple_strtol(chkey, &ptr, 10);

}

/** HASH AREA */

#define SIZE 20
int items = 0;

struct DataItem {
   int data;  
   int key;
};

struct DataItem* hashArray[SIZE]; 
struct DataItem* dummyItem;
struct DataItem* item;

int hashCode(int key) {
   return key % SIZE;
}

bool isFull (void) {
	return (items >= SIZE);
}

struct DataItem *search(int key) {
   //get the hash 
   int hashIndex;
   hashIndex = hashCode(key);  

   //move in array until an empty 
   while(hashArray[hashIndex] != NULL) {
	
      if(hashArray[hashIndex]->key == key)
         return hashArray[hashIndex]; 
			
      //go to next cell
      ++hashIndex;
		
      //wrap around the table
      hashIndex %= SIZE;
   }        
	
   return NULL;        
}

void insert(int key, int data) {

   int hashIndex;
   struct DataItem *insertItem = (struct DataItem*) vmalloc(sizeof(struct DataItem));
   
   if(isFull()) {
   	printk(KERN_INFO "mydevice: failed to insert item. Table already full!\n");
   	return;
   }

   item = search(key);
	if(item != NULL) {
	  item->data = data;
	  return;
   	}

   insertItem->data = data;  
   insertItem->key = key;

   //get the hash 
   hashIndex = hashCode(key);

   //move in array until an empty or deleted cell
   while(hashArray[hashIndex] != NULL && hashArray[hashIndex]->key != -1) {
      //go to next cell
      ++hashIndex;
		
      //wrap around the table
      hashIndex %= SIZE;
   }
	
   hashArray[hashIndex] = insertItem;
   items++;
}

struct DataItem* delete(struct DataItem* item) {
   int hashIndex;
   int key;
   key = item->key;

   //get the hash 
   hashIndex = hashCode(key);

   //move in array until an empty
   while(hashArray[hashIndex] != NULL) {
	
      if(hashArray[hashIndex]->key == key) {
         struct DataItem* temp = hashArray[hashIndex]; 
			
         //assign a dummy item at deleted position
         hashArray[hashIndex] = dummyItem; 
         items--;
         return temp;
      }
		
      //go to next cell
      ++hashIndex;
		
      //wrap around the table
      hashIndex %= SIZE;
   }      
	
   return NULL;        
}

void display(void) {
   int i;
	
	printk(KERN_INFO "mydevice: items: %i\n", items);
   for(i = 0; i<SIZE; i++) {
	
      if(hashArray[i] != NULL)
         printk(KERN_INFO " (%d,%d)",hashArray[i]->key,hashArray[i]->data);
      else
         printk(KERN_INFO " ~~ ");
   }
	
   printk(KERN_INFO "\n");
}


char saveData[100];
void getSaveData (void) {
	int i;
	char sKey[5], sData[5];
	strncpy(saveData, "", sizeof(saveData));

	for(i = 0; i<SIZE; i++) {
	
      if(hashArray[i] != NULL) {
      	snprintf(sKey, sizeof(sKey), "%i", hashArray[i]->key);
      	snprintf(sData, sizeof(sData), "%i", hashArray[i]->data);
      	strcat(saveData, sKey);
      	strcat(saveData, ",");
      	strcat(saveData, sData);
      	strcat(saveData, "\n");
      }
   }

}


/** END HASH AREA */


int device_open(struct inode *inode, struct file *filp){

	printk(KERN_INFO "mydevice: opened device\n");
	return 0;
}


int searchKey;
char searchValue[100];
char failString[3] = "NaN";
char* saveString = "s";
ssize_t device_read(struct file* filp, char* bufStoreData, size_t bufCount, loff_t* curOffset){
	printk(KERN_INFO "mydevice: Reading from device\n");

	if(strcmp(bufStoreData, saveString) == 0){
		printk(KERN_INFO "mydevice: Sending save data\n");
		getSaveData();
		ret = copy_to_user(bufStoreData, saveData, sizeof(saveData));
		return ret;
	}

	printk(KERN_INFO "mydevice: reached this far...............\n");

	updateKey(bufStoreData);

	searchKey = (int) key;

	item = search(searchKey);

   	if(item != NULL) {
    	//printk(KERN_INFO "mydevice: Element found: %d\n", item->data);
   		snprintf(searchValue, sizeof(searchValue), "%i", item->data);
    	printk(KERN_INFO "mydevice: Element with value '%i' found for key '%i'\n", item->data, searchKey);
   		ret = copy_to_user(bufStoreData, searchValue, sizeof(searchValue));
  	} else {
     	printk(KERN_INFO "mydevice: Element with key '%i' not found\n", searchKey);
     	ret = copy_to_user(bufStoreData, failString, sizeof(failString));
   	}

   	display();

	return ret;
}

char rawInput[100];
ssize_t device_write(struct file* filp, const char* bufSourceData, size_t bufCount, loff_t* curOffset){
	
	printk(KERN_INFO "mydevice: locking device\n");

	if(down_interruptible(&virtual_device.sem) != 0){
		printk(KERN_ALERT "mydevice: could not lock device during open\n");
		return -1;
	}

	printk(KERN_INFO "mydevice: writing to device\n");

	ret = copy_from_user(rawInput, bufSourceData, bufCount);
	
	updateKeyValuePair(rawInput);
	
	insert((int)key, (int)value);

	display();

	up(&virtual_device.sem);

	printk(KERN_INFO "mydevice: unlocked device\n");

	return ret;
}

int device_close(struct inode *inode, struct file *filp){
	printk(KERN_INFO "mydevice: closed device\n");
	return 0;
}


struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = device_open,
	.release = device_close,
	.write = device_write,
	.read = device_read
};

static int driver_entry(void){
	ret = alloc_chrdev_region(&dev_num,0,1,DEVICE_NAME);
	if(ret < 0) {
		printk(KERN_ALERT "mydevice: failed to allocate a major number.\n");
		return ret;
	}
	
	major_number = MAJOR(dev_num);
	printk(KERN_INFO "mydevice: major number is %d\n", major_number);
	printk(KERN_INFO "\tuse \"mknod /dev/%s c %d 0\" for device file\n", DEVICE_NAME, major_number);
	
	mcdev = cdev_alloc();
	mcdev->ops = &fops;
	mcdev->owner = THIS_MODULE;

	ret = cdev_add(mcdev, dev_num, 1);

	if(ret < 0) {
		printk(KERN_ALERT "mydevice: unable to add cdev to kernel\n");
		return ret;
	}

	sema_init(&virtual_device.sem, 1);

	dummyItem = (struct DataItem*) vmalloc(sizeof(struct DataItem));
   	dummyItem->data = -1;  
   	dummyItem->key = -1;

	return 0;
}

static void driver_exit(void){
	cdev_del(mcdev);

	unregister_chrdev_region(dev_num, 1);
	printk(KERN_ALERT "mydevice: unloaded module\n");

}

module_init(driver_entry);
module_exit(driver_exit);