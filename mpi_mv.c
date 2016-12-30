#include<stdio.h>
#include"mpi.h"
#include"MyMPI.h"

typedef double dtype;
#define MPI_TYPE MPI_DOUBLE

int main(int argc,char** argv)
{
	dtype** a;
	dtype* b;
	dtype* c_block;
	dtype* c;
	dtype* storage;
	int i,j;
	int id;
	int m,n;
	int nprime;
	int p;
	int rows;

	MPI_Init(&argc,&argv);
	MPI_Comm_size(MPI_COMM_WORLD,&p);
	MPI_Comm_rank(MPI_COMM_WORLD,&id);

	read_row_striped_matrix(argv[1],(void*)&a,(void*)&storage,MPI_TYPE,&m,&n,MPI_COMM_WORLD);
	rows=BLOCK_SIZE(id,p,m);
	print_row_striped_matrix((void**)a,MPI_TYPE,m,n,MPI_COMM_WORLD);
	read_replicated_vector(argv[2],(void*)&b,MPI_TYPE,&nprime,MPI_COMM_WORLD);
	print_replicated_vector(b,MPI_TYPE,nprime,MPI_COMM_WORLD);
	c_block=(dtype*)malloc(rows*sizeof(dtype));
	c=(dtype*)malloc(n*sizeof(dtype));
	for(i=0;i<rows;i++)
	{
		c_block[i]=0.0;
		for(j=0;j<n;j++)
			c_block[i]+=a[i][j]*b[j];
	}
	replicated_block_vector(c_block,n,(void*)c,MPI_TYPE,MPI_COMM_WORLD);
	print_replicated_vector(c,MPI_TYPE,n,MPI_COMM_WORLD);

	MPI_Finalize();
}

