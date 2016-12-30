#include"mpi.h"
#include<stdio.h>
#include<stdlib.h>

void interP2P(int bufsize,int rank,int membershipKey,MPI_Comm* comm)
{
	int myrank;
	int i,mysize,sb[bufsize],rb[bufsize];
	char outstr[500];
	MPI_Request sreq1,rreq1;
	MPI_Status sstatus1,rstatus1;

	if(membershipKey==0){
		MPI_Comm_rank(*comm,&myrank);
		MPI_Comm_size(*comm,&mysize);
		for(i=0;i<bufsize;i++)
			sb[i]=myrank*mysize+i;
		if(myrank==0){
			MPI_Isend(sb,bufsize,MPI_INT,1,123,*comm,&sreq1);
		}
		if(myrank==0){
			MPI_Wait(&sreq1,&sstatus1);
		}
	} else if(membershipKey==1){
		MPI_Comm_rank(*comm,&myrank);
		MPI_Comm_size(*comm,&mysize);
		if(myrank==1){
			MPI_Irecv(rb,bufsize,MPI_INT,0,123,*comm,&rreq1);
		}
		if(myrank==1){
			MPI_Wait(&rreq1,&rstatus1);
			sprintf(outstr,"proc G: %d, L: %d, received:\n",rank,myrank);
			
			for(i=0;i<bufsize;i++)
				sprintf(outstr,"%srb[%d]=%d,",outstr,i,rb[i]);
		}
		fprintf(stdout,"%s\n",outstr);
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

	interP2P(5,rank,membershipKey,&myFirstComm);
	
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





