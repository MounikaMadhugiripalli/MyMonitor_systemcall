
/*---Start of mymonitor.c----*/

#include<linux/linkage.h>
#include<linux/kernel.h>
#include<linux/sched.h>

#include <linux/slab.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/semaphore.h>
#include <linux/cdev.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/ptrace.h>
#include <asm/unistd.h>
#include <asm/ptrace.h> 
#include <linux/syscalls.h>
#include <asm/syscall.h>
#include <linux/delay.h>
#define FILE_PERM 0666

#if BITS_PER_LONG != 32
        #define FILE_FLAGS O_CREAT | O_APPEND  | O_LARGEFILE;
#else
        #define FILE_FLAGS O_CREAT | O_APPEND 
#endif

#define SUCCESS 0
#define FAILURE -1

char *MYFILE_PATH;
int mymonitor_flag=0;
int reverse(int mypid)	{
	int rev_mypid=0;
	while(mypid)	{

		rev_mypid*=10;
		rev_mypid+=(mypid%10);
		mypid/=10;
	}
	return rev_mypid;
}
int myfile_operation(struct task_struct *p)   { 			//Creation of file from Kernel Space into Userspace
	int mypid=p->pid;
	int myreg=0;
	char char_myreg[10]={0};
	char char_mypid[10]={0};
	sprintf(char_myreg,"%d",myreg);
        printk("*****start of file creation******\n");
        struct file *kfd = filp_open(MYFILE_PATH,FILE_FLAGS,FILE_PERM);
	if(IS_ERR(kfd))
	{
		printk(KERN_INFO "Error in opening file\n");
		return FAILURE;
	}	 
	
   	do{
	myreg=syscall_get_nr(p,task_pt_regs(p));
	//printk("current regs:%d\n",myreg);
	//printk("current mypid:%d\n",mypid);
	
	unsigned long int rev_mypid=reverse((int)mypid);
	unsigned long int rev_myreg=reverse((int)myreg);
	unsigned short int i=0;
	while(rev_mypid)	{

		char_mypid[i]=(char)((rev_mypid%10)+'0');
		rev_mypid/=10;
		i++;
	}
	char_mypid[i]='\0';	
	unsigned short int j=0;
	while(rev_myreg)	{

		char_myreg[j]=(char)((rev_myreg%10)+'0');
		rev_myreg/=10;
		j++;
	}
	char_myreg[j]='\0';

	//printk("mypid:%s",char_mypid);
	if(sys_ptrace(PTRACE_GETREGS,mypid,NULL,NULL)== -1)
	{
		printk("error in sys_ptrace\n");
		return -1;	
	}
      
	
        if((kfd->f_op->write(kfd, char_mypid,strlen(char_mypid), &kfd->f_pos))<0)
        {
                printk("Copying to File Was Failed\n");
          
                return FAILURE;
        }
        
	if((kfd->f_op->write(kfd, "\t",strlen("\t"), &kfd->f_pos))<0)
        {
                printk("Copying to File Was Failed\n");
          
                return FAILURE;
        }
	
        if((kfd->f_op->write(kfd, char_myreg,strlen(char_myreg), &kfd->f_pos))<0)
        {
                printk("Copying to File Was Failed\n");
                return FAILURE;
        }
   

	if((kfd->f_op->write(kfd, "\n",strlen("\n"), &kfd->f_pos))<0)
        {
                printk("Copying to File Was Failed\n");
          
                return FAILURE;
        }
	
	}while(myreg!=(syscall_get_nr(p,task_pt_regs(p))));

        filp_close(kfd,NULL);
        printk("*****end of file creation******\n");
	return SUCCESS;	
}

	
asmlinkage long sys_mymonitor(int mUid, char* mFile)			//Mymonitor function
{
	
	mymonitor_flag=mUid;
	MYFILE_PATH=mFile;
	int status;
	printk("*****start in mymonitor******\n");
        printk("The current process_pid:%d",current->pid);  
	if(mymonitor_flag == 0)
	{
		if(sys_kill(current->pid,9)== -1)
		{
			printk("error in killing daemon\n");
			return -1;	
		}
	}
	struct task_struct *p = current;
	pid_t  curr =task_pid_nr(p);
	pid_t  mychildpid,w;
	
	mychildpid = sys_fork();
	if(mychildpid < 0) {
		printk("fork failed\n");
		sys_exit(FAILURE);
	}

	if(mychildpid > 0)
	{
		printk("fork SUCCESS\n");
	}
	w = sys_waitpid(mychildpid, &status, WUNTRACED | WCONTINUED);
                   if (w == -1) {
                       printk("error in waitpid\n");
                       sys_exit(FAILURE);
                   }


	
	sys_umask(0);
	
	sys_close(0);	
	sys_close(1);	
	sys_close(2);	

	sys_getpgrp();

	int pre;
	while(1)
	{
	
		for_each_process(p)
		{
			pre = p->pid;
			
			if(pre > curr){
				myfile_operation(p);
				curr = pre;
			}
		}
		

	
	}
	

	printk("*****end of mymonitor ******\n");
	return SUCCESS;
}

