# MyMonitor_systemcall
Adds System call( called mymonitor) to  kernel to record system calls that are executed by a process  
Adding a new feature to the UML Linux kernel which allows for the monitoring of the system calls of a given user are logged in the given file. 

This feature must be accessible by a system tool monitor, with the following synopsis: 
	monitor -a file   //Activates the monitoring 
	monitor -d       //Deactivates the monitoring 
