#include<stdio.h>
#include<stdlib.h>
#include"mpi.h"
#include"MyMPI.h"

int get_size(MPI_Datatype t)
{
	if(t==MPI_BYTE)
		return sizeof(char);
	if(t==MPI_DOUBLE) 
		return sizeof(double);
	if(t==MPI_FLOAT)
		return sizeof(float);
	if(t==MPI_INT)
		return sizeof(int);
	printf("Error: Unrecognized argument to 'get_size'\n");
	fflush(stdout);
	MPI_Abort(MPI_COMM_WORLD,TYPE_ERROR);
}

void *my_malloc(int id,int bytes)
{
	void* buffer;
	if((buffer=malloc((size_t)bytes))==NULL)
	{
		printf("Error: Malloc failed for process %d\n",id);
		fflush(stdout);
		MPI_Abort(MPI_COMM_WORLD,MALLOC_ERROR);
	}
	return buffer;
}

void terminate(int id,char* error_message)
{
	if(!id)
	{
		printf("Error: %s\n",error_message);
		fflush(stdout);
	}
	MPI_Finalize();
	exit(-1);
}

void create_mixed_xfer_arrays(int id,int p,int n,int **count,int** disp)
{
	int i;
	*count=my_malloc(id,p*sizeof(int));
	*disp=my_malloc(id,p*sizeof(int));
	(*count)[0]=BLOCK_SIZE(0,p,n);
	(*disp)[0]=0;
	for(i=1;i<p;i++)
	{
		(*disp)[i]=(*disp)[i-1]+(*count)[i-1];
		(*count)[i]=BLOCK_SIZE(i,p,n);
	}
}

void create_uniform_xfer_arrays(int id,int p,int n,int **count,int **disp)
{
	int i;
	*count=my_malloc(id,p*sizeof(int));
	*disp=my_malloc(id,p*sizeof(int));
	(*count)[0]=BLOCK_SIZE(id,p,n);
	(*disp)[0]=0;
	for(i=1;i<p;i++)
	{
		(*disp)[i]=(*disp)[i-1]+(*count)[i-1];
		(*count)[i]=BLOCK_SIZE(id,p,n);
	}
}

void replicate_block_vector(void *ablock,int n,void *arep,MPI_Datatype dtype,MPI_Comm comm)
{
	int *cnt;
	int *disp;
	int id;
	int p;

	MPI_Comm_size(comm,&p);
	MPI_Comm_rank(comm,&id);
	create_mixed_xfer_arrays(id,p,n,&cnt,&disp);
	MPI_Allgatherv(ablock,cnt[id],dtype,arep,cnt,disp,dtype,comm);
	free(cnt);
	free(disp);
}

void read_checkerboard_matrix(char* s,void ***subs,void **storage,MPI_Datatype dtype,int *m,int* n,MPI_Comm grid_comm)
{
	void *buffer;
	int coords[2];
	int datnum_size;
	int dest_id;
	int grid_coord[2];
	int grid_id;
	int grid_period[2];
	int grid_size[2];
	int i,j,k;
	FILE *infileptr;
	void *laddr;
	int local_cols;
	int local_rows;
	void **lptr;
	int p;
	void *raddr;
	void *rptr;
	MPI_Status status;
	MPI_Comm_rank(grid_comm,&grid_id);
	MPI_Comm_size(grid_comm,&p);
	datnum_size=get_size(dtype);

	if(grid_id==0)
	{
		infileptr=fopen(s,"r");
		if(infileptr==NULL)
			*m=0;
		else
		{
			fread(m,sizeof(int),1,infileptr);
			fread(n,sizeof(int),1,infileptr);
		}
	}
	MPI_Bcast(m,1,MPI_INT,0,grid_comm);
	if(!(*m))
		MPI_Abort(MPI_COMM_WORLD,OPEN_FILE_ERROR);
	MPI_Bcast(n,1,MPI_INT,0,grid_comm);

	MPI_Cart_get(grid_comm,2,grid_size,grid_period,grid_coord);
	local_rows=BLOCK_SIZE(grid_coord[0],grid_size[0],*m);
	local_cols=BLOCK_SIZE(grid_coord[1],grid_size[1],*n);

	*storage=my_malloc(grid_id,local_rows*local_cols*datnum_size);
	*subs=(void**)my_malloc(grid_id,local_rows*PTR_SIZE);
	lptr=(void*)*subs;
	rptr=(void*)*storage;
	for(i=0;i<local_rows;i++)
	{
		*(lptr++)=(void*)rptr;
		rptr+=local_cols*datnum_size;
	}

	if(grid_id==0)
		buffer=my_malloc(grid_id,*n*datnum_size);

}

void read_col_striped_matrix(char* s,void ***subs,void **storage,MPI_Datatype dtype,int *m,int*n,MPI_Comm comm)
{
	void *buffer;
	int datnum_size;
	int i,j;
	int id;
	FILE* infileptr;
	int local_cols;
	void **lptr;
	void *rptr;
	int p;
	int *send_count;
	int *send_disp;

	MPI_Comm_size(comm,&p);
	MPI_Comm_rank(comm,&id);
	datnum_size=get_size(dtype);

	if(id==(p-1))
	{
		infileptr=fopen(s,"r");
		if(infileptr==NULL)
			*m=0;
		else
		{
			fread(m,sizeof(int),1,infileptr);
			fread(n,sizeof(int),1,infileptr);
		}
	}
	MPI_Bcast(m,1,MPI_INT,p-1,comm);

	if(!(*m))
		MPI_Abort(comm,OPEN_FILE_ERROR);
	
	MPI_Bcast(n,1,MPI_INT,p-1,comm);
	local_cols=BLOCK_SIZE(id,p,*n);

	*storage=my_malloc(id,*m*local_cols*datnum_size);
	*subs=(void**)my_malloc(id,*m*PTR_SIZE);
	lptr=(void*)*subs;
	rptr=(void*)*storage;
	for(i=0;i<*m;i++)
	{
		*(lptr++)=(void*)rptr;
		rptr+=local_cols*datnum_size;
	}

	if(id==(p-1))
		buffer=my_malloc(id,*n*datnum_size);
	create_mixed_xfer_arrays(id,p,*n,&send_count,&send_disp);
	for(i=0;i<*m;i++)
	{
		if(id==(p-1))
			fread(buffer,datnum_size,*n,infileptr);
		MPI_Scatterv(buffer,send_count,send_disp,dtype,(*storage)+i*local_cols*datnum_size,local_cols,dtype,p-1,comm);
	}
	free(send_count);
	free(send_disp);
	if(id==(p-1))
		free(buffer);
}

void read_row_striped_matrix(char* s,void ***subs,void **storage,MPI_Datatype dtype,int *m,int *n,MPI_Comm comm)
{
	int datnum_size;
	int i;
	int id;
	FILE* infileptr;
	int local_rows;
	void **lptr;
	int p;
	void *rptr;
	MPI_Status status;
	int x;

	MPI_Comm_size(comm,&p);
	MPI_Comm_rank(comm,&id);
	datnum_size=get_size(dtype);

	if(id==(p-1))
	{
		infileptr=fopen(s,"r");
		if(infileptr==NULL)
			*m=0;
		else
		{
			fread(m,sizeof(int),1,infileptr);
			fread(n,sizeof(int),1,infileptr);
		}
	}
	MPI_Bcast(m,1,MPI_INT,p-1,comm);
	if(!(*m))
		MPI_Abort(MPI_COMM_WORLD,OPEN_FILE_ERROR);
	MPI_Bcast(n,1,MPI_INT,p-1,comm);
	local_rows=BLOCK_SIZE(id,p,*m);

	*storage=(void*)my_malloc(id,local_rows**n*datnum_size);
	*subs=(void**)my_malloc(id,local_rows*PTR_SIZE);
	lptr=(void*)&(*subs[0]);
	rptr=(void*)*storage;
	for(i=0;i<local_rows;i++)
	{
		*(lptr++)=(void*)rptr;
		rptr+=*n*datnum_size;
	}

	if(id==(p-1))
	{
		for(i=0;i<p-1;i++)
		{
			x=fread(*storage,datnum_size,BLOCK_SIZE(i,p,*m)**n,infileptr);
			MPI_Send(*storage,BLOCK_SIZE(i,p,*m)**n,dtype,i,DATA_MSG,comm);
		}
		x=fread(*storage,datnum_size,local_rows**n,infileptr);
		fclose(infileptr);
	}
	else
		MPI_Recv(*storage,local_rows**n,dtype,p-1,DATA_MSG,comm,&status);
}

void print_submatrix(void **a,MPI_Datatype dtype,int rows,int cols)
{
	int i,j;
	for(i=0;i<rows;i++)
	{
		for(j=0;j<cols;j++)
		{
			if(dtype==MPI_DOUBLE)
				printf("%6.3f ",((double**)a)[i][j]);
			else
			{
				if(dtype==MPI_FLOAT)
					printf("%6.3f ",((float**)a)[i][j]);
				else if(dtype==MPI_INT)
					printf("%6d ",((int**)a)[i][j]);
			}
		}
		putchar('\n');
	}
}

void print_row_striped_matrix(void **a,MPI_Datatype dtype,int m,int n,MPI_Comm comm)
{
	MPI_Status status;
	void* bstorage;
	void **b;
	int datnum_size;
	int i;
	int id;
	int local_rows;
	int max_block_size;
	int prompt;
	int p;

	MPI_Comm_rank(comm,&id);
	MPI_Comm_size(comm,&p);
	local_rows=BLOCK_SIZE(id,p,m);

	if(!id)
	{
		print_submatrix(a,dtype,local_rows,n);
		if(p>1)
		{
			datnum_size=get_size(dtype);
			max_block_size=BLOCK_SIZE(p-1,p,m);
			bstorage=my_malloc(id,max_block_size*n*datnum_size);
			b=(void**)my_malloc(id,max_block_size*datnum_size);
			b[0]=bstorage;
			for(i=1;i<max_block_size;i++)
				b[i]=b[i-1]+n*datnum_size;
			for(i=1;i<p;i++)
			{
				MPI_Send(&prompt,1,MPI_INT,i,PROMPT_MSG,MPI_COMM_WORLD);
				MPI_Recv(bstorage,BLOCK_SIZE(i,p,m)*n,dtype,i,RESPONSE_MSG,MPI_COMM_WORLD,&status);
				print_submatrix(b,dtype,BLOCK_SIZE(i,p,m),n);
			}
			free(b);
			free(bstorage);
		}
		putchar('\n');
	}
	else
	{
		MPI_Recv(&prompt,1,MPI_INT,0,PROMPT_MSG,MPI_COMM_WORLD,&status);
		MPI_Send(*a,local_rows*n,dtype,0,RESPONSE_MSG,MPI_COMM_WORLD);
	}
}

void read_replicated_vector(char* s,void **v,MPI_Datatype dtype,int *n,MPI_Comm comm)
{
	int datnum_size;
	int i;
	int id;
	FILE* infileptr;
	int p;

	MPI_Comm_rank(comm,&id);
	MPI_Comm_size(comm,&p);
	datnum_size=get_size(dtype);
	if(id==(p-1))
	{
		infileptr=fopen(s,"r");
		if(infileptr==NULL)
			*n=0;
		else
			fread(n,sizeof(int),1,infileptr);
	}
	MPI_Bcast(n,1,MPI_INT,p-1,MPI_COMM_WORLD);
	if(!*n)
		terminate(id,"Cannot open vector file");
	*v=my_malloc(id,*n*datnum_size);
	if(id==(p-1))
	{
		fread(*v,datnum_size,*n,infileptr);
		fclose(infileptr);
	}
	MPI_Bcast(*v,*n,dtype,p-1,MPI_COMM_WORLD);
}

void print_subvector(void* a,MPI_Datatype dtype,int n)
{
	int i;
	for(i=0;i<n;i++)
	{
		if(dtype==MPI_DOUBLE)
			printf("%6.3f ",((double*)a)[i]);
		else if(dtype==MPI_FLOAT)
			printf("%6.3f ",((float*)a)[i]);
		else if(dtype==MPI_INT)
			printf("%6d ",((int*)a)[i]);
	}
}

void print_replicated_vector(void *v,MPI_Datatype dtype,int n,MPI_Comm comm)
{
	int id;
	MPI_Comm_rank(comm,&id);
	if(!id)
	{
		print_subvector(v,dtype,n);
		printf("\n\n");
	}
}

void replicated_block_vector(void *ablock,int n,void *arep,MPI_Datatype dtype,MPI_Comm comm)
{
	int *cnt;
	int *disp;
	int id;
	int p;

	MPI_Comm_rank(comm,&id);
	MPI_Comm_size(comm,&p);
	create_mixed_xfer_arrays(id,p,n,&cnt,&disp);
	MPI_Allgatherv(ablock,cnt[id],dtype,arep,cnt,disp,dtype,comm);
	free(cnt);
	free(disp);
}
