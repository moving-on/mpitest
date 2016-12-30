#include"mpi.h"
#include<stdio.h>
static int SIZE=5;

int main(int argc,char** argv)
{
	int rank,gsize,sb[SIZE],root,i;

	MPI_Comm comm=MPI_COMM_WORLD;
	root=0;

	MPI_Init(&argc,&argv);
	MPI_Comm_rank(comm,&rank);
	MPI_Comm_size(comm,&gsize);

	int buf[gsize*SIZE];
	char outstr[gsize*SIZE*10];

	for(i=0;i<SIZE;i++)
		sb[i]=gsize*rank+i;
	
	MPI_Allgather(sb,SIZE,MPI_INT,buf,SIZE,MPI_INT,MPI_COMM_WORLD);
	sprintf(outstr,"p: %d gathered",rank);
	for(i=0;i<gsize*SIZE;i++)
		sprintf(outstr,"%s,buf[%d]=%d",outstr,i,buf[i]);
	fprintf(stdout,"%s\n",outstr);
	MPI_Finalize();
	return 0;
}
