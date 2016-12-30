#include"mpi.h"
#include<stdio.h>
#include<stdlib.h>

#define BUFSIZE 5

int main(int argc,char** argv)
{
	MPI_Request r;
	MPI_Status s;
	int flag;
	int buf[BUFSIZE];
	char pstr[BUFSIZE*(sizeof(int)+8)+50];
	int tag=123;
	int dest=0;
	int rank,size,i,j;

	MPI_Init(&argc,&argv);
	MPI_Comm_size(MPI_COMM_WORLD,&size);
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);

	for(int i=0;i<BUFSIZE;i++)
		buf[i]=BUFSIZE*rank+i;
	
	MPI_Send_init(buf,BUFSIZE,MPI_INT,dest,tag,MPI_COMM_WORLD,&r);

	if(rank==0)
	{
		int rbuf1[BUFSIZE*size];
		MPI_Request* rr=(MPI_Request*)malloc(size*sizeof(MPI_Request));

		for(int i=0;i<size;i++)
		{
			fprintf(stdout,"proc: %d,before Irecv...\n",rank);
			MPI_Irecv(rbuf1+i*BUFSIZE,BUFSIZE,MPI_INT,i,tag,MPI_COMM_WORLD,&rr[i]);
			fprintf(stdout,"proc: %d,after Irecv...\n",rank);
		}

		MPI_Start(&r);

		MPI_Wait(&r,&s);

		MPI_Waitall(size,rr,MPI_STATUSES_IGNORE);

		for(int i=0;i<size;i++)
		{
			sprintf(pstr,"proc: %d received massage from %d\n",rank,i);
			for(j=0;j<BUFSIZE;++j)
				sprintf(pstr,"%s rbuf1[%d]=%d,",pstr,i*BUFSIZE+j,rbuf1[i*BUFSIZE+j]);
			sprintf(pstr,"%s rbuf1[%d]=%d\n",pstr,i*BUFSIZE+j,rbuf1[i*BUFSIZE+j]);
			fprintf(stdout,"%s",pstr);
		}
		free(rr);
	}
	else
	{
		MPI_Start(&r);
		MPI_Wait(&r,&s);
	}

	MPI_Request_free(&r);
	
	MPI_Finalize();
	return 0;
}
