#include"mpi.h"
#include<stdio.h>
#include<stdlib.h>

void interAllgather(int bufsize,int rank,int root,int membershipKey,MPI_Comm* comm)
{
	int i,myrank;
	int sb[bufsize];
	int rb[3*bufsize];
	char outstr[500];

	for(i=0;i<bufsize;i++)
		rb[i]=-111;

	int baseA=77;
	int baseB=111;
	if(membershipKey==0){
		MPI_Comm_rank(*comm,&myrank);
		for(i=0;i<bufsize;i++)
			sb[i]=baseA*(i+1)+rank;

		MPI_Allgather(sb,bufsize,MPI_INT,rb,bufsize,MPI_INT,*comm);
		sprintf(outstr,"gp: %d,lp: %d",rank,myrank);
		for(i=0;i<3*bufsize;i++)
			sprintf(outstr,"%s,[%d]=%d",outstr,i,rb[i]);
		fprintf(stdout,"%s\n",outstr);
	} else if(membershipKey==1){
		MPI_Comm_rank(*comm,&myrank);
		for(i=0;i<bufsize;i++)
			sb[i]=baseB*(i+1)+rank;

		MPI_Allgather(sb,bufsize,MPI_INT,rb,bufsize,MPI_INT,*comm);
		sprintf(outstr,"gp: %d,lp: %d",rank,myrank);
		for(i=0;i<bufsize*3;i++)
			sprintf(outstr,"%s,[%d]=%d",outstr,i,rb[i]);

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

	interAllgather(4,rank,2,membershipKey,&myFirstComm);
	
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





