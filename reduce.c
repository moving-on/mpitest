#include"mpi.h"
#include<stdio.h>
#include<stdlib.h>
#include<math.h>

double f(double x)
{
	return (4.0/(1.0+x*x));
}

int main(int argc,char** argv)
{
	int done=0,n,myid,numprocs,i,namelen;
	double PI25DT=3.141592653589793238462643;
	double mypi,pi,h,sum,x,startwtime=0.0,endwtime;
	char processor_name[MPI_MAX_PROCESSOR_NAME];

	MPI_Init(&argc,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD,&myid);
	MPI_Comm_size(MPI_COMM_WORLD,&numprocs);
	MPI_Get_processor_name(processor_name,&namelen);
	fprintf(stdout,"process %d of %d on %s\n",myid,numprocs,processor_name);

	n=0;
	if(myid==0){
		printf("please enter a value,\nN= ");
		scanf("%d",&n);
		startwtime=MPI_Wtime();
	}
	MPI_Bcast(&n,1,MPI_INT,0,MPI_COMM_WORLD);
	h=1.0/(double)n;
	sum=0.0;
	for(i=myid+1;i<n;i+=numprocs)
	{
		x=h*((double)i-0.5);
		sum+=f(x);
	}
	mypi=h*sum;
	MPI_Reduce(&mypi,&pi,1,MPI_DOUBLE,MPI_SUM,0,MPI_COMM_WORLD);
	if(myid==0){
		printf("processor %d,pi is approximately %.16f,Error is %.16f\n",myid,pi,fabs(pi-PI25DT));
		endwtime=MPI_Wtime();
		printf("wall clock time=%f\n",endwtime-startwtime);
		fflush(stdout);
	}

	MPI_Finalize();
	return 0;
}
