#include<stdio.h>
#include"mpi.h"
#include"MyMPI.h"
typedef int dtype;
#define MPI_TYPE MPI_INT
#define MAX_VALUE 1000000

void createMatrixFile(FILE* outfile)
{
	if(outfile==NULL)
		exit(-1);
	dtype a[5][5]={{0,2,5,MAX_VALUE,MAX_VALUE,MAX_VALUE},{MAX_VALUE,0,7,1,MAX_VALUE,8},\
			{MAX_VALUE,MAX_VALUE,0,4,MAX_VALUE,MAX_VALUE},{MAX_VALUE,MAX_VALUE,MAX_VALUE,0,3,MAX_VALUE},{MAX_VALUE,MAX_VALUE,2,MAX_VALUE,0,3},{MAX_VALUE,5,MAX_VALUE,2,4,0}};
	int i,j;
	int m=5,n=5;
	fwrite(&m,sizeof(int),1,outfile);
	fwrite(&m,sizeof(int),1,outfile);
	for(i=0;i<5;i++)
	{
		for(j=0;j<5;j++)
		{
			fwrite(&a[i][j],sizeof(int),1,outfile);
		}
	}
	fclose(outfile);
}


int main(int argc,char** argv)
{
	/*
	FILE *outfile;
	outfile=fopen("floydarray","wb");
	createMatrixFile(outfile);
	*/
	dtype** a;
	dtype* storage;
	int i,j,k;
	int id;
	int m,n;
	int p;
	void compute_shortest_paths(int,int,dtype**,int);
	
	MPI_Init(&argc,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD,&id);
	MPI_Comm_size(MPI_COMM_WORLD,&p);
	read_row_striped_matrix(argv[1],(void*)&a,(void*)&storage,MPI_TYPE,&m,&n,MPI_COMM_WORLD);
	if(m!=n)
		terminate(id,"Matrix must be square\n");
	print_row_striped_matrix((void**)a,MPI_TYPE,m,n,MPI_COMM_WORLD);
	compute_shortest_paths(id,p,(dtype**)a,n);
	print_row_striped_matrix((void**)a,MPI_TYPE,m,n,MPI_COMM_WORLD);

	MPI_Finalize();
}

void compute_shortest_paths(int id,int p,dtype** a,int n)
{
	int i,j,k;
	int offset;
	int root;
	int *tmp;
	tmp=(dtype*)malloc(n*sizeof(dtype));
	for(k=0;k<n;k++)
	{
		root=BLOCK_OWNER(k,p,n);
		if(root==id)
		{
			offset=k-BLOCK_LOW(id,p,n);
			for(j=0;j<n;j++)
				tmp[j]=a[offset][j];
		}
		MPI_Bcast(tmp,n,MPI_TYPE,root,MPI_COMM_WORLD);
		for(i=0;i<BLOCK_SIZE(id,p,n);i++)
			for(j=0;j<n;j++)
				a[i][j]=MIN(a[i][j],a[i][k]+tmp[j]);
	}
	free(tmp);
}
