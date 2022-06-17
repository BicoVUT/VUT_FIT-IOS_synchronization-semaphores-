//////////////////////////////////
/*		IOS		*/
/*	       proj2		*/
/*	      xbrnaf00		*/
/*	     Filip Brna		*/
/*	     2019/2020		*/
//////////////////////////////////


///			INCLUDES		///

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <inttypes.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h> 


/// 			DEFINES			///

#define DEBUG
#define MMAP(pointer) {(pointer) = mmap(NULL, sizeof(*(pointer)), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);}
#define UNMAP(pointer){munmap((pointer),sizeof((pointer)));}

int *NA = 0;
int *NE = 0;
int *NB = 0;
int *NC = 0;
int *NI = 0;
int *GC = 0;
int *AC = 0;


sem_t *imm_wait = NULL;
sem_t *judge_inside = NULL;
sem_t *write_a = NULL;
sem_t *imm_checkin = NULL;
sem_t *imm_checked = NULL;
sem_t *imm_certificate = NULL;
FILE *pfile;

///			INIT			///

int init()
{
	pfile = fopen("proj2.out","w");				// opening file for output

	if(pfile == NULL){
		fprintf(stderr,"FILE NOT OPEN");		// error handle
		return 1;
	}


	setbuf(pfile,NULL);					// function for writting to file with buffer


	MMAP(NA);						// MMAPING shared variables
	MMAP(NI);
	MMAP(NE);
	MMAP(NB);
	MMAP(NC);
	MMAP(GC);
	MMAP(AC);

				// opening semaphores, if sem didn't open -> return -1;

	if((imm_wait = sem_open("/xbrnaf00.ios.proj2.imm_wait", O_CREAT | O_EXCL, 0666, 1))== SEM_FAILED){
		return 1;
	}
	if((judge_inside = sem_open("/xbrnaf00.ios.proj2.judge_inside", O_CREAT | O_EXCL, 0666, 1))== SEM_FAILED){
		return 1;
	}
	if((write_a = sem_open("/xbrnaf00.ios.proj2.write_a", O_CREAT | O_EXCL, 0666, 1))== SEM_FAILED){
		return 1;
	}
	if((imm_checkin = sem_open("/xbrnaf00.ios.proj2.imm_checkin", O_CREAT | O_EXCL, 0666, 1))== SEM_FAILED){
		return 1;
	}
	if((imm_checked = sem_open("/xbrnaf00.ios.proj2.imm_checked", O_CREAT | O_EXCL, 0666, 0))== SEM_FAILED){
		return 1;
	}
	if((imm_certificate = sem_open("/xbrnaf00.ios.proj2.imm_certificate", O_CREAT | O_EXCL, 0666, 0))== SEM_FAILED){
		return 1;
	}



	return 0;
}

///			CLEANUP			///

void cleanup()
{
								//UNMAPING shared variables
	UNMAP(NA);
	UNMAP(NI);
	UNMAP(NE);
	UNMAP(NB);
	UNMAP(NC);
	UNMAP(GC);
	UNMAP(AC);
								// closing semaphores
	sem_close(imm_wait);
	sem_close(judge_inside);
	sem_close(imm_checkin);
	sem_close(imm_checked);
	sem_close(imm_certificate);
	sem_close(write_a);


								// semaphore unlinking
	sem_unlink("/xbrnaf00.ios.proj2.imm_wait");
	sem_unlink("/xbrnaf00.ios.proj2.judge_inside");
	sem_unlink("/xbrnaf00.ios.proj2.imm_checkin");
	sem_unlink("/xbrnaf00.ios.proj2.imm_checked");
	sem_unlink("/xbrnaf00.ios.proj2.imm_certificate");
	sem_unlink("/xbrnaf00.ios.proj2.write_a");
								// closing output file
	if(pfile != NULL){
		fclose(pfile);
	}

}

///			IMMIGRANT FUNCTIONS			///

void immigrant_leaves(int immigrant_number)
{
	(*NB)--;	

	sem_wait(write_a);
	(*NA)++;
	fprintf(pfile,"%d :	 IMM %d 		: leaves			%d : %d : %d\n",*NA,immigrant_number,*NE,*NC,*NB);
	sem_post(write_a);
}


void immigrant_certificate(int immigrant_number, int IT)
{	

	sem_wait(write_a);
	(*NA)++;
	fprintf(pfile,"%d :	 IMM %d 		: wants certificate		%d : %d : %d\n",*NA,immigrant_number,*NE,*NC,*NB);
	sem_post(write_a);

	if( IT != 0){
		usleep((rand()%IT)*1000);
	}

	sem_wait(write_a);
	(*NA)++;
	fprintf(pfile,"%d :	 IMM %d 		: got certificate		%d : %d : %d\n",*NA,immigrant_number,*NE,*NC,*NB);
	sem_post(write_a);

}


void immigrant_check(int immigrant_number)
{
	sem_wait(imm_checkin);
	(*NC)++;

	sem_wait(write_a);
	(*NA)++;
	fprintf(pfile,"%d :	 IMM %d		: checks 			%d : %d : %d\n",*NA,immigrant_number,*NE,*NC,*NB);
	sem_post(write_a);

	sem_post(imm_checkin);
	if ((*NE) == (*NC)){
		sem_post(imm_checked);
	}
}

void immigrant_enter(int immigrant_number)
{
	sem_wait(write_a);
	(*NA)++;
	fprintf(pfile,"%d :	 IMM %d		: enters 			%d : %d : %d\n",*NA,immigrant_number,*NE,*NC,*NB);
	sem_post(write_a);
}


void immigrant_start(int immigrant_number)
{
	(*NI)++;

	sem_wait(write_a);
	(*NA)++;
	fprintf(pfile,"%d :	 IMM %d		: starts\n",*NA,immigrant_number);
       	sem_post(write_a);	
}

///			IMMIGRANT PROCESS			///

void immigrant_process(int immigrant_number, int IT)
{
	sem_wait(imm_wait);
	sem_wait(judge_inside);

	(*NE)++;
	(*NB)++;

	immigrant_enter(immigrant_number);			// immigrant entered to judgment hall

	sem_post(imm_wait);
	sem_post(judge_inside);

	immigrant_check(immigrant_number);			// immigrant was checked print

	sem_wait(imm_certificate);
	sem_post(imm_certificate);

	(*GC)++;
	immigrant_certificate(immigrant_number,IT);		// immigrant got certificate form judge

	sem_wait(judge_inside);
	sem_post(judge_inside);

	immigrant_leaves(immigrant_number);			// immigrant leaves judgment hall
}


///			JUDGE FUNCTIONS				///

void judge_enter()
{
	sem_wait(write_a);
	(*NA)++;
	fprintf(pfile,"%d :	 JUDGE		: wants to enter\n",*NA);
	sem_post(write_a);

	sem_wait(judge_inside);

	sem_wait(write_a);
	(*NA)++;
	fprintf(pfile,"%d : 	 JUDGE	 	: enters			%d : %d : %d\n",*NA,*NE,*NC,*NB);
	sem_post(write_a);
}

void judge_wait()
{
	sem_wait(write_a);
	(*NA)++;
	fprintf(pfile,"%d :	 JUDGE	 	: waits for imm			%d : %d : %d\n",*NA,*NE,*NC,*NB);
	sem_post(write_a);
}

void judge_leaves()
{
	sem_wait(write_a);
	(*NA)++;
	fprintf(pfile,"%d :	 JUDGE		: leaves			%d : %d : %d\n",*NA,*NE,*NC,*NB);
	sem_post(write_a);
}


void judge_confirmation(int JT)
{
	sem_wait(write_a);
	(*NA)++;
	fprintf(pfile,"%d : 	 JUDGE	 	: starts confirmation		%d : %d : %d\n",*NA,*NE,*NC,*NB);
	sem_post(write_a);

	if( JT != 0 ){
		usleep((rand()%JT)*1000);
	}

	(*NC)=0;						// all entered have been confirmated
	(*NE)=0;						// all checked have been confirmated
		
	sem_wait(write_a);
	(*NA)++;
	fprintf(pfile,"%d :	 JUDGE 		: ends confirmation		%d : %d : %d\n",*NA,*NE,*NC,*NB);
	sem_post(write_a);
}

void judge_finish()
{
	sem_wait(write_a);
	(*NA)++;
	fprintf(pfile,"%d :	 JUDGE		: finishes\n",*NA);
	sem_post(write_a);
}

//			JUDGE PROCESS				///

void process_judge(int JG,int PI,int JT)
{
	while (PI != *GC){					// get certificate != number of immigrants from argument
		if( JG != 0){
			usleep((rand()%JG)*1000);
		}

		judge_enter();					// judge enter print

		if ((*NC) != (*NE)){	
			judge_wait();				// judge wait for immigrant print
		}
		if ((*NC) != (*NE) && (*NB) != 0){
			sem_wait(imm_checked);
			sem_post(imm_checked);
			sem_wait(imm_checked);
		}
		
		judge_confirmation(JT);				// judge confirmation print
		
		sem_post(imm_certificate);
		if( JT != 0){
			usleep((rand()%JT)*1000);
		}
		sem_wait(imm_certificate);

		judge_leaves();					// judge leaves print

		sem_post(judge_inside);
	}
	judge_finish();						// judge finish print
}

//			GENERATE IMMIGRANTS			//

void gen_immigrants(int PI, int IG, int IT)
{
	pid_t IMMIGRANT;  					// datatype pid_t for variable 
	int immigrant_number = 0; 				// imm ID
	for (int i = 0;i<PI;i++)   				//  for with PI iterations
	{
		if( IG != 0){
			usleep((rand()%IG)*1000);	
		}
		IMMIGRANT = fork(); 				// split for child and parent
		if (IMMIGRANT == 0){ 				// if for child
			//child
			immigrant_number = i+1;
			immigrant_start(immigrant_number);	// immigrant process started
			break;					// child dont create new processes anymore
		}
		if (IMMIGRANT < 0){
			fprintf(stderr,"ERROR\n");		// error handle
			exit(1);
	       	}
	}
	if (IMMIGRANT != 0){
		while((IMMIGRANT = waitpid(-1,NULL,0))){	// synchronization 
			if(errno == ECHILD){
				break;
			}
		}

	}
	else{
		immigrant_process(immigrant_number,IT);		// immigrant process
		exit(0);
	}
}

//				MAIN				//
	
int main(int argc,char *argv[])
{
	
	if (argc != 6){
		fprintf(stderr, "INVALID INPUT\n");		// cheking valid input
		return 1;
	}

	int PI= atoi(argv[1]);					// saving arguments to variables
	int IG= atoi(argv[2]);
	int JG= atoi(argv[3]);
	int IT= atoi(argv[4]);
	int JT= atoi(argv[5]);

	if ( PI < 1 || IG < 0 || IG > 2000 || JG < 0 || JG > 2000 || IT < 0 || IT > 2000 || JT < 0 || JT > 2000)  {
		fprintf(stderr, "INVALID INPUT\n" );
		return 1;
	}
	

	if (init() == -1){					// error handle for not correct init
		//printf("INIT == -1 \n");
		cleanup();
		return 1;
	}
	pid_t PROCESS = fork();					// split for child and parent
	if ( PROCESS == 0 ){
		//child
		gen_immigrants(PI,IG,IT);			// generator of immigrants
		exit(0);
		}
	else{
		pid_t JUDGE = fork();				// process judge
		if ( JUDGE == 0 ){
			//child
			process_judge(JG,PI,JT);	
			exit(0);	
		}
		waitpid(JUDGE,NULL,0);				// synchronization
	}
	waitpid(PROCESS,NULL,0);				// synchronization
	cleanup();						// cleanup function
	exit(0);
	return 0;						// valid return
}
