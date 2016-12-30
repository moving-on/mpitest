#include"mpi.h"
#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<unistd.h>

int main(int argc,char** argv)
{
	int rank,value,nprocs;

	MPI_Init(&argc,&argv);
	MPI_Comm_size(MPI_COMM_WORLD,&nprocs);
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);

	do{
		if(rank==0){
			fprintf(stdout,"please enter a value, -1 meanse finish\n");
			scanf("%d",&value);

			//MPI_Bcast(&value,1,MPI_INT,0,MPI_COMM_WORLD);
		}

		MPI_Bcast(&value,1,MPI_INT,0,MPI_COMM_WORLD);
		//sleep(1);
		fprintf(stdout,"Process %d got %d\n",rank,value);
	}while(value>=0);

	MPI_Finalize();
	return 0;
}
