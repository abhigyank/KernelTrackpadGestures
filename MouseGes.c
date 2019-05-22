#include <linux/input.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/kmod.h>
#include <linux/time.h>

#define DEVICE_NAME "MouseGes"
#define PROCFS_MAX_SIZE 1000
#define gestureCount 5
#define positionBufferSize 35
MODULE_LICENSE("GPL"); 
  
///< The author -- visible when you use modinfo 
MODULE_AUTHOR("Abhigyan Khaund"); 
  
///< The description -- see modinfo 
MODULE_DESCRIPTION("Mouse Gestures!"); 
  
///< The version of the module 
MODULE_VERSION("0.1"); 
     
static struct input_dev *mouseges_dev;
static struct proc_dir_entry *proc_file;
static struct timespec t1,t2;
static int trigger[3] = {-1, -1, -1};
static int gesture_on = 0;
static int count_x = 0;
static int count_y = 0;
static int *position_x;
static int *position_y;
static int y_end;
char proc_buf[PROCFS_MAX_SIZE];
char *prefix = "/usr/bin/";
char* forInitialProc[] = {"/usr/bin/python", "/home/abhigyan/project/config.py", NULL};
char* command1[] = {"", NULL};
char* command2[] = {"", NULL};
char* command3[] = {"", NULL};
char* command4[] = {"", NULL};
char* command5[] = {"", NULL};
char** commandList[gestureCount] = {command1, command2, command3, command4, command5};

static char* envp[] = {
	"SHELL=/bin/bash",
	"HOME=/home/abhigyan",
	"PATH=/sbin:/bin:/usr/sbin:/usr/bin",
	"DISPLAY=:0",
	 NULL };


/**
* struct action - A structure to store the action to be executed corresponding to gesture.
* @execute:		application name string.
*/
static struct action {
	char execute[100];
} action_list[gestureCount];

/**
* get_max() - Get minimum value from one of the position arrays.
* @axis:	   	0 for x-axis and 1 for y-axis
*/
static int get_min(int axis) {
	if(axis ==0) {
		int i,m=INT_MAX;
		for(i=0;i<min(positionBufferSize,count_x);i++) m =min(m,position_x[i]);
		return m; 
	}
	else {
		int i,m=INT_MAX;
		for(i=0;i<min(positionBufferSize,count_y);i++) m =min(m,position_y[i]);
		return m; 
	}
}
/**
* get_max() - Get maximum value from one of the position arrays.
* @axis:	   	0 for x-axis and 1 for y-axis
*/
static int get_max(int axis) {
	if(axis ==0) {
		int i,m=0;
		for(i=0;i<min(positionBufferSize,count_x);i++) m =max(m,position_x[i]);
		return m; 
	}
	else {
		int i,m=0;
		for(i=0;i<min(positionBufferSize,count_y);i++) m =max(m,position_y[i]);
		return m; 
	}
}

/**
* determinePattern() - Called on when gesture recording stops. Determines which pattern gesture matches to and executes the corresponding action.
* @click: 	   Scancode of mouse button pressed / released
* @value:	   Status of key being pressed or released
*/
static void determinePattern(void) {
	int result;	
	int min_x = get_min(0);
	int max_x = get_max(0);
	int min_y = get_min(1);
	int max_y = get_max(1);
	printk("X range. %d %d", min_x, max_x);
	printk("Y range. %d %d %d %d", min_y, max_y, y_end, position_y[0]);
	if(max_x - min_x <=10000 && abs(y_end - position_y[0])>3000) {
		printk("Verical Line %s", commandList[0][0]);	
		 result = call_usermodehelper(commandList[0][0], commandList[0], envp, UMH_NO_WAIT);
		 printk("Result = %d", result);		
	}
	else if(max_y - min_y <=10000){
		printk("Horizonal Line %s", commandList[1][0]);
		result = call_usermodehelper(commandList[1][0], commandList[1], envp, UMH_NO_WAIT);
		printk("Result = %d", result);	
	}
	else if(abs(y_end - position_y[0])<3000) {
		printk("V shape %s", commandList[4][0]);
		result = call_usermodehelper(commandList[4][0], commandList[4], envp, UMH_NO_WAIT);
		printk("Result = %d", result);			
	}	
	else if(position_x[0] < position_x[1] ){
		result = call_usermodehelper(commandList[2][0], commandList[2], envp, UMH_NO_WAIT);
		printk("Result = %d", result);		
		printk("Right Diagonal %s", commandList[2][0]);
	}
	else {
		printk("Left Diagonal %s", commandList[3][0]);
		result = call_usermodehelper(commandList[3][0], commandList[3], envp, UMH_NO_WAIT);
		printk("Result = %d", result);			
	}
	return;
}
/**
* should_start() - Called on each mouse. This function is responsible for determine if gesture recording should start. Checks for 3 continuous mouse clicks. 
* @click: 	   Scancode of mouse button pressed / released
* @value:	   Status of key being pressed or released
*/
static void should_start(unsigned int click, unsigned int value) {
	if(value == 0) return;	
	if(trigger[0] !=-1 && trigger[1] !=-1 && trigger[2] !=-1) {
		trigger[0] = -1;
		trigger[1] = -1;
		trigger[2] = -1;
		gesture_on = 0;
		printk("Pattern Recording Complete. %d %d", count_x, count_y);
		determinePattern();
		count_x = 0;		
		count_y = 0;		
		
		return;
	}
	printk("%d %d %d %d", trigger[0], trigger[1], trigger[2], value);
	if(click == BTN_LEFT) {
		if(trigger[0] == -1) {
			trigger[0] = 1;
			return;
		}	
		else if (trigger[1] == -1) {
			trigger[1] = 1;
			return;
		}
		else if (trigger[2] == -1) {
			trigger[2] = 1;
			gesture_on = 1;
			printk("Pattern Recording Triggered.");
			return;
		}
		else {
			trigger[0] = -1;
			trigger[1] = -1;
			trigger[2] = -1;
			gesture_on = 0;
		}

	}
	else {
		if(gesture_on) {
			trigger[0] = -1;
			trigger[1] = -1;
			trigger[2] = -1;
			gesture_on = 0;
			printk("Pattern Recording Complete.");
			determinePattern();	
			return;		
		}
		else {
			trigger[0] = -1;
			trigger[1] = -1;
			trigger[2] = -1;
			return;
		}
	}
}


static int mouseges_open(struct input_dev *dev) 
{	
	printk("MouseGes Opened.");
	return 0; 
}

static void mouseges_close(struct input_dev *dev) {}

/**
* mouseges_filter() - called when a event occurs from an input device. We only process the event if it is mouse click or mouse movement when gesture is recording.
					
* @handle:			Pointer to input handler
* @type:			Type of event
* @code:			Scancode of key pressed
* @value:			Status of key being pressed or released
*/
static bool mouseges_filter(struct input_handle *handle, unsigned int type, unsigned int code, int value)
{

	if(type == EV_KEY) {
		if(code == BTN_LEFT || code == BTN_RIGHT) {
			getnstimeofday(&t1);
			if(trigger[0] == -1){
				printk(" clicked: %u %u", code, value);should_start(code, value);
			}
			else {
				if(t1.tv_sec - t2.tv_sec >1 && !gesture_on) {
					trigger[0] = -1;
					trigger[1] = -1;
					trigger[2] = -1;
					printk(" clicked: %u %u", code, value);should_start(code, value);
				}
				else {
					printk(" clicked: %u %u", code, value);should_start(code, value);				
				}
			}
			t2 = t1;
		}
		else {
			if(!gesture_on) {
				trigger[0] = -1;
				trigger[1] = -1;
				trigger[2] = -1;
			}
		}
	
	}
	
	if(type == EV_ABS) {
		if(!gesture_on) return 0;		
		if(code == ABS_X) { 
			printk(" Current X position: %u", value);	
			if(count_x<=positionBufferSize)
				position_x[count_x] = value;
			count_x+=1;
		}
		if(code == ABS_Y) { 
			printk(" Current Y position: %u", value);	
			if(count_y<=positionBufferSize)
				position_y[count_y] = value;
			count_y+=1;
			y_end = value;
		}
	}	

	return 0;
}

/**
* mouseges_connect() - Called when an input device connects to mouseges
* @handler: 			 Input handler of device
* @dev:				 Input device
* @id:				 Id of input device
*/
static int mouseges_connect(struct input_handler *handler, struct input_dev *dev, const struct input_device_id *id)
{
	struct input_handle *handle;
	int error;
 
	handle = kzalloc(sizeof(struct input_handle), GFP_KERNEL);
	if (!handle)
		return -ENOMEM;
 
	handle->dev = dev;
	handle->handler = handler;
	handle->name = "mouseges_handle";
 
	error = input_register_handle(handle);
	if (error)
		goto err_free_handle;
 
	error = input_open_device(handle);
	if (error)
		goto err_unregister_handle;
 
	printk(KERN_INFO "Connected device: (%s  %s)\n",dev->name ,  dev->phys);
 
	return 0;
 
 err_unregister_handle:
	input_unregister_handle(handle);
 err_free_handle:
	kfree(handle);
	return error;
}

/**
* mouseges_disconnect() - called when input device disconnects
* @handle: 				Input handler of device
*/
static void mouseges_disconnect(struct input_handle *handle)
{
	printk(KERN_INFO "Disconnect %s\n", handle->dev->name);
 
	input_close_device(handle);
	input_unregister_handle(handle);
	kfree(handle);
}


/**
* proc_write() - called when /dev/proc_file file is written from the user-space. It updates the actions stored in the buffer.				
*
* @fp:					file pointer
* @buf:					buffer in which data was written
* @len:					length of data
* @off:					offset
*
* Return - length of buffer
*/
static ssize_t proc_write(struct file *fp, const char *buf, size_t len, loff_t * off) {
	int i, index=0, idx=0;
	if(len > PROCFS_MAX_SIZE) 
	{ 
		return -EFAULT; 
	}

	if(copy_from_user(proc_buf, buf, len)) 
	{ 
		return -EFAULT;
	}
	//printk(KERN_INFO "%s", proc_buf);
	for(i=0;i<len;i++) {
		if(proc_buf[i] == '\n') {
			action_list[index].execute[idx++] = '\0';
			idx = 0;
			index++;
		}
		else {
			action_list[index].execute[idx++] = proc_buf[i];
		}
	}
	for(i=0; i<gestureCount;i++) {
		printk("%s", action_list[i].execute);
		commandList[i][0] = (char*)kmalloc(100,GFP_KERNEL);
		strcpy(commandList[i][0], prefix);
		strcat(commandList[i][0], action_list[i].execute);
		printk("%s", commandList[i][0]);
	}
	return len;
}

/*
* initialiseProc() - called at init to copy values into procfile from user space.
*				  
*/
static void initialiseProc(void) {
	 int result = call_usermodehelper(forInitialProc[0], forInitialProc, envp, UMH_NO_WAIT);
	 printk("Result = %d", result);	
}
     
static const struct input_device_id mouseges_ids[] = {
    	{ .driver_info = 1 },			/* Matches all devices */
    	{ },					
};
     
static struct input_handler mouseges_handler = {
    	.filter 	=	mouseges_filter,
	.connect	=	mouseges_connect,
	.disconnect	=	mouseges_disconnect,
    	.name 		=	DEVICE_NAME,
    	.id_table 	=	mouseges_ids,
};
     
static struct file_operations mouseges_fops = {
	.write		=	proc_write,
	.owner		=	THIS_MODULE
}; 
     
/*
* mouseges_init() - called when module is loaded into kernel. It registers the input device drivers, input handler and proc file.
*				  
*/
static int __init mouseges_init(void) {
	int error;

	mouseges_dev = input_allocate_device();

	if (!mouseges_dev) {
		printk(KERN_ERR "mouseges.c: Not enough memory. Registering device failed\n");
		error = -ENOMEM;
		goto err_exit;
	}
	else {
		printk(KERN_INFO "Device Allocated.\n");
	}

	mouseges_dev->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_REL);
	mouseges_dev->relbit[0] = BIT_MASK(REL_X) | BIT_MASK(REL_Y);
	mouseges_dev->keybit[BIT_WORD(BTN_LEFT)] = BIT_MASK(BTN_LEFT);

	mouseges_dev->open  = mouseges_open;
	mouseges_dev->close = mouseges_close;

	error = input_register_device(mouseges_dev);
	if (error) {
	    printk(KERN_ERR "mouseges.c: Failed to register device\n");
	    goto err_free_dev;
	}
	else {
	    printk(KERN_ERR "mouseges.c: register device complete.\n");
	}

	error = input_register_handler(&mouseges_handler);

	if (error)
	{
		// DPRINTK("Registering input handler failed with (%d)\n",error);
	    printk(KERN_ERR "Registering input handler failed with (%d)\n",error);
	 	goto err_unregister_dev;
	}
	else {
		printk(KERN_INFO " MouseGes Handler registerd");
	}
	position_x = kzalloc(30*sizeof(position_x), GFP_KERNEL);
	position_y = kzalloc(30*sizeof(position_y), GFP_KERNEL);

	proc_file = proc_create("mouseges_proc_file", 0666, NULL, &mouseges_fops);
	if(!proc_file) {
		printk ("Couldn't create proc_file.");	
		return -ENOMEM;
	}
	initialiseProc();
	return 0;

err_free_dev:
	input_free_device(mouseges_dev);
err_unregister_dev:
	input_unregister_device(mouseges_dev);
err_exit:
	return error;
}


/**
* mouseges_exit() - called when module is removed from kernel.
*/
static void __exit mouseges_exit(void)
{
	input_unregister_handler(&mouseges_handler);
	input_unregister_device(mouseges_dev);
	input_free_device(mouseges_dev);
	remove_proc_entry("mouseges_proc_file", NULL);
}
     
module_init(mouseges_init);
module_exit(mouseges_exit);
