#include<stdio.h>
#include<dirent.h>
#include<stdlib.h>
#include<sys/stat.h>
#include<string.h>
#include"mpi.h"
#include"MyMPI.h"
#include<ftw.h>

#define DICT_SIZE_MSG 0
#define FILE_NAME_MSG 1
#define VECTOR_MSG 2
#define EMPTY_MSG 3

#define DIR_ARG 1
#define DICT_ARG 2
#define RES_ARG 3

typedef unsigned char uchar;

int main(int argc,char** argv)
{
	int id;
	int p;
	MPI_Comm worker_comm;
	void manager(int,char**,int);
	void worker(int,char**,MPI_Comm);

	MPI_Init(&argc,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD,&id);
	MPI_Comm_size(MPI_COMM_WORLD,&p);
	if(argc!=4)
	{
		if(!id)
		{
			printf("Program need three arguments:\n");
			printf("%s<dir><dict><result>\n",argv[0]);
		}
	}
	else if(p<2)
	{
		printf("Program needs at least two processes\n");
	}
	else
	{
		if(!id)
		{
			MPI_Comm_split(MPI_COMM_WORLD,MPI_UNDEFINED,id,&worker_comm);
			manager(argc,argv,p);
		}
		else
		{
			MPI_Comm_split(MPI_COMM_WORLD,0,id,&worker_comm);
			worker(argc,argv,worker_comm);
		}
	}

	MPI_Finalize();
	return 0;
}

void manager(int argc,char** argv,int p)
{
	int assign_cnt;
	int *assigned;
	uchar* buffer;
	int dict_size;
	int file_cnt;
	char** file_name;
	int i;
	MPI_Request pending;
	int src;
	MPI_Status status;
	int tag;
	int terminated;
	uchar** vector;

	void build_2d_array(int,int,uchar***);
	void get_names(char*,char***,int*);
	void write_profiles(char*,int,int,char**,uchar**);

	MPI_Irecv(&dict_size,1,MPI_INT,MPI_ANY_SOURCE,DICT_SIZE_MSG,MPI_COMM_WORLD,&pending);

	get_names(argv[DIR_ARG],&file_name,&file_cnt);

	MPI_Wait(&pending,&status);
	
	buffer=(uchar*)malloc(dict_size*sizeof(MPI_UNSIGNED_CHAR));

	build_2d_array(file_cnt,dict_size,&vector);

	terminated=0;
	assign_cnt=0;
	assigned=(int*)malloc(p*sizeof(int));

	do{
		MPI_Recv(buffer,dict_size,MPI_UNSIGNED_CHAR,MPI_ANY_SOURCE,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
		src=status.MPI_SOURCE;
		tag=status.MPI_TAG;
		if(tag==VECTOR_MSG)
		{
			for(i=0;i<dict_size;++i)
				vector[assigned[src]][i]=buffer[i];
		}

		if(assign_cnt<file_cnt)
		{
			MPI_Send(file_name[assign_cnt],strlen(file_name[assign_cnt])+1,MPI_CHAR,src,FILE_NAME_MSG,MPI_COMM_WORLD);
			assigned[src]=assign_cnt;
			assign_cnt++;
		}
		else
		{
			MPI_Send(NULL,0,MPI_CHAR,src,FILE_NAME_MSG,MPI_COMM_WORLD);
			terminated++;
		}
	} while(terminated<(p-1));

	write_profiles(argv[RES_ARG],file_cnt,dict_size,file_name,vector);
}

void get_names(char* dir_path,char***file_name,int* file_cnt)
{
	DIR *pDir;
	struct dirent* ent=NULL;
	char dir[512];
	struct stat statbuf;
	int count=0;
	int i;
	fprintf(stdout,"Open dir %s\n",dir_path);
	if((pDir=opendir(dir_path))==NULL)
	{
		fprintf(stderr,"Cannot open dir\n",dir_path);
		exit(-1);
	}

	while((ent=readdir(pDir))!=NULL)
	{
		snprintf(dir,512,"%s/%s",dir_path,ent->d_name);
		lstat(dir,&statbuf);
		if(!S_ISDIR(statbuf.st_mode))
			count++;
	}
	closedir(pDir);
	

	if(((*file_name)=(char**)malloc(sizeof(char*)*count))==NULL)
	{
		fprintf(stderr,"malloc failed\n");
		exit(-1);
	}
	
	if((pDir=opendir(dir_path))==NULL)
	{
		fprintf(stderr,"Cannot open dir\n",dir_path);
		exit(-1);
	}
	
	fprintf(stdout,"filenum: %d\n",count);
	for(i=0;(ent=readdir(pDir))!=NULL&&i<count;)
	{
		if(strlen(ent->d_name)<=0)
			continue;
		snprintf(dir,512,"%s/%s",dir_path,ent->d_name);
		lstat(dir,&statbuf);

		if(!S_ISDIR(statbuf.st_mode))
		{
			if(((*file_name)[i]=(char*)malloc(strlen(ent->d_name)+1))==NULL)
			{
				fprintf(stderr,"Malloc failed\n");
				exit(-1);
			}
			
			memset((*file_name)[i],0,strlen(ent->d_name)+1);
			strcpy((*file_name)[i],ent->d_name);
			fprintf(stdout,"num%d: %s\n",i,ent->d_name);
			fflush(stdout);
			i++;
		}
	}

	closedir(pDir);
	*file_cnt=count;
}

void build_2d_array(int file_cnt,int dict_size,uchar*** vector)
{
	int i;
	if(((*vector)=(uchar**)malloc(sizeof(uchar*)*file_cnt))==NULL)
	{
		MPI_Abort(MPI_COMM_WORLD,MALLOC_ERROR);
	}
	for(i=0;i<file_cnt;++i)
	{
		if(((*vector)[i]=(uchar*)malloc(sizeof(uchar)*dict_size))==NULL)
			MPI_Abort(MPI_COMM_WORLD,MALLOC_ERROR);
	}
}

void write_profiles(char* resfile,int file_cnt,int dict_size,char** file_name,uchar** vector)
{
	int i,j;
	FILE* outfile=fopen(resfile,"wb");
	if(outfile==NULL)
	{
		fprintf(stderr,"Cannot open result file %s\n",resfile);
		fflush(stderr);
	}
	
	fwrite((char*)&file_cnt,sizeof(int),1,outfile);
	fwrite((char*)&dict_size,sizeof(int),1,outfile);
	for(i=0;i<file_cnt;++i)
	{
		int file_len=strlen(file_name[i]);
		fwrite((char*)&file_len,sizeof(int),1,outfile);
		fwrite(file_name[i],sizeof(char),strlen(file_name[i]),outfile);
		for(j=0;j<dict_size;++j)
		{
			fwrite((char*)&vector[i][j],sizeof(uchar),1,outfile);
		}
	}
}

void worker(int argc,char** argv,MPI_Comm comm)
{
	char* buffer;
	hash
}
