#include <stdio.h>
#include <errno.h>
#include <sys/types.h>


main()
{
	extern int errno;
	//extern char *sys_errlist[];
	
	pid_t pid;
	int val = 0;
	int n = 0;
	int status
	pid = getpid();
	fprintf(stdout, "Avant le fork, pid = %d\n", pid);
	sleep(20);
	
	pid = fork();
	/* En cas de réussite du fork le pere et le fils poursuivent leur execution
	   à partir d'ici. La valeur de pid permet de distinguer le pere du fils.
	*/
	
	fprintf(stdout, "après le fork, pid = %d\n", pid);
	
	switch(pid) 
	{
		case -1:	/* erreur dans fork() */
			fprintf(stderr, "error %d in fork: %s\n", errno, strerror(errno));
			exit(errno);
			
		case 0: 	/* on est dans le fils */
			fprintf(stdout, "valeur de val dans le fils = %d\n", pid);
			fprintf(stdout, "Dans le fils, pid = %d\n", getpid());
			//sleep(20);
			exit(50);
			/* On suspend le processus pendant 20s. Cela permet d'utiliser
			   la commande ps (p.ex. ps -gux) pour visualiser la liste des processus
			*/
			break;
			
		default:	/* on est dans le pere */
			fprintf(stdout, "valeur de val dans le pere = %d\n", pid);
			fprintf(stdout, "Dans le pere, pid = %d\n", getpid());
			//sleep(20);
			wait(&status);
	}
}			