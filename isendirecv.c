#include"mpi.h"
#include<stdio.h>
#include<stdlib.h>

int main(int argc,char** argv)
{
	int myid,numprocs,left,right,buf[10],buf2[10];
	MPI_Request req,req2;
	MPI_Status status,status2;

	MPI_Init(&argc,&argv);
	MPI_Comm_size(MPI_COMM_WORLD,&numprocs);
	MPI_Comm_rank(MPI_COMM_WORLD,&myid);

	right=(myid+1)%numprocs;
	left=myid-1;
	if(left<0)
		left=numprocs-1;
	for(int i=0;i<10;i++)
		buf2[i]=myid*numprocs+i;
	
	MPI_Irecv(buf,10,MPI_INT,left,123,MPI_COMM_WORLD,&req);
	MPI_Isend(buf2,10,MPI_INT,right,123,MPI_COMM_WORLD,&req2);

	MPI_Wait(&req,&status);
	MPI_Wait(&req2,&status2);

	fprintf(stdout,"P: %d,i send and recv finished\n",myid);

	MPI_Finalize();
	return 0;
}
