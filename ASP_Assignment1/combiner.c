=========================================================================
=													                    =
=				Name: Mohit Sathyaseelan								=
=				contact: Mohitsathyaseelan@gmail.com			        =
=													    	            =
=========================================================================


#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<math.h>
#include<string.h> 

int
main(int argc, char *argv[])
{
    int pfd[2];                                     /* Pipe file descriptors */

    if (pipe(pfd) == -1)                            /* Create pipe */
        perror("pipe");

    switch (fork()) {
    case -1:
        perror("fork");

    case 0:             /* First child: exec 'ls' to write to pipe */
        if (close(pfd[0]) == -1)                    /* Read end is unused */
            perror("close 1");

        /* Duplicate stdout on write end of pipe; close duplicated descriptor */

        if (pfd[1] != STDOUT_FILENO) {              /* Defensive check */
            if (dup2(pfd[1], STDOUT_FILENO) == -1)
                perror("dup2 1");
            if (close(pfd[1]) == -1)
                perror("close 2");
        }       
        execlp("./mapper","./mapper",NULL);		/* Writes to pipe */
	perror("mapper error");

    }

    switch (fork()) {
    case -1:
        perror("fork");

    case 0:             /* Second child: exec 'wc' to read from pipe */
        if (close(pfd[1]) == -1)                    /* Write end is unused */
            perror("close 3");

        /* Duplicate stdin on read end of pipe; close duplicated descriptor */

        if (pfd[0] != STDIN_FILENO) {               /* Defensive check */
            if (dup2(pfd[0], STDIN_FILENO) == -1)
                perror("dup2 2");
            if (close(pfd[0]) == -1)
                perror("close 4");
        }

        execlp("./reducer","./reducer","-l",NULL);
	perror("reducer error");

    default: /* Parent falls through */
        break;
    }

    /* Parent closes unused file descriptors for pipe, and waits for children */

	if(close(pfd[0]) == -1)
		perror("close 5 error");
	if(close(pfd[1]) == -1)
		perror("close 6 error");
	if(wait(NULL) == -1)
		perror("wait 1 error");

	if(wait(NULL) == -1)
		perror("wait 2 error");

	exit(EXIT_SUCCESS);

	return 0;
}
