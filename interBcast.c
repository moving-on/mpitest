#include"mpi.h"
#include<stdio.h>
#include<stdlib.h>

void interBcast(int *buf,int bufsize,int rank,int root,int membershipKey,MPI_Comm* comm)
{
	int i;
	int base=77;
	int myrank;
	char printb[200];
	for(i=0;i<bufsize;i++)
		printb[i]=-111;

	if(membershipKey==2){
		MPI_Comm_rank(*comm,&myrank);
		if(myrank==root){
			for(i=0;i<bufsize;i++)
				buf[i]=base+i*11;
			fprintf(stdout,"global %d, inter %d prepare to cast data\n",rank,myrank);
			MPI_Bcast(buf,bufsize,MPI_INT,MPI_ROOT,*comm);
			fprintf(stdout,"global %d, inter %d cast data finished\n",rank,myrank);
		} else{
			fprintf(stdout,"global %d, inter %d prepare to cast data\n",rank,myrank);
			MPI_Bcast(buf,bufsize,MPI_INT,MPI_PROC_NULL,*comm);
			sprintf(printb,"%s","");
			for(i=0;i<bufsize;i++)
				sprintf(printb,"%s,buf[%d]=%d",printb,i,buf[i]);

			sprintf(printb,"%s\n",printb);
			fprintf(stdout,"global %d, local p: %d cast data finished: %s\n",rank,myrank,printb);
		}
	} else if(membershipKey==0){
		MPI_Comm_rank(*comm,&myrank);
		fprintf(stdout,"global %d, p: %d, prepare to receive cast data\n",rank,myrank);
		MPI_Bcast(buf,bufsize,MPI_INT,root,*comm);
		sprintf(printb,"%s","");
		for(i=0;i<bufsize;++i)
			sprintf(printb,"%s,buf[%d]=%d",printb,i,buf[i]);

		sprintf(printb,"%s\n",printb);
		fprintf(stdout,"global %d, p: %d, received cast data: %s\n",rank,myrank,printb);
	}

}

			

int main(int argc,char** argv)
{
	MPI_Comm mycomm;
	MPI_Comm myFirstComm;
	MPI_Comm mySecondComm;
	MPI_Comm myThirdComm;

	int membershipKey;
	int rank;
	int buf[4];

	MPI_Init(&argc,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);

	for(int i=0;i<4;i++)
		buf[i]=-1;

	membershipKey=rank%3;

	MPI_Comm_split(MPI_COMM_WORLD,membershipKey,rank,&mycomm);

	if(membershipKey==0){
		MPI_Intercomm_create(mycomm,0,MPI_COMM_WORLD,1,1,&myFirstComm);
		fprintf(stdout,"p: %d,first 1 call end\n",rank);

		MPI_Intercomm_create(mycomm,0,MPI_COMM_WORLD,2,2,&myThirdComm);
		fprintf(stdout,"p: %d,first 2 call end\n",rank);
	} else if(membershipKey==1){
		MPI_Intercomm_create(mycomm,0,MPI_COMM_WORLD,0,1,&myFirstComm);
		fprintf(stdout,"p: %d,second 1 call end\n",rank);

		MPI_Intercomm_create(mycomm,0,MPI_COMM_WORLD,2,12,&mySecondComm);
		fprintf(stdout,"p: %d,second 2 call end\n",rank);
	} else if(membershipKey==2){
		MPI_Intercomm_create(mycomm,0,MPI_COMM_WORLD,0,2,&myThirdComm);
		fprintf(stdout,"p: %d,third 1 call end\n",rank);

		MPI_Intercomm_create(mycomm,0,MPI_COMM_WORLD,1,12,&mySecondComm);
		fprintf(stdout,"p: %d,third 2 call end\n",rank);
	}

	interBcast(buf,4,rank,2,membershipKey,&myThirdComm);
	
	switch(membershipKey)
	{
		case 1:
			MPI_Comm_free(&mySecondComm);
			MPI_Comm_free(&myFirstComm);
		break;
		case 0:
			MPI_Comm_free(&myFirstComm);
			MPI_Comm_free(&myThirdComm);
		break;
		case 2:
			MPI_Comm_free(&myThirdComm);
			MPI_Comm_free(&mySecondComm);
		break;
	}

	MPI_Finalize();
}





