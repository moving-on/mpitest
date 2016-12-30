#include"mpi.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define MSG1LEN 7
#define MSG2LEN 2
#define MSG3LEN 17
#define RMSG1LEN 64
#define RMSG2LEN 64
#define RMSG3LEN 64

int main(int argc,char** argv)
{
	MPI_Comm comm=MPI_COMM_WORLD;
	int dest=1,src=0,tag=1,s1,s2,s3;
	char* buf,*bbuf,msg1[7],msg3[17],rmsg1[64],rmsg3[64];
	double msg2[2],rmsg2[64];
	int errs=0,rank,bufsize,bsize;

	MPI_Init(&argc,&argv);
	MPI_Comm_rank(comm,&rank);

	MPI_Pack_size(7,MPI_CHAR,comm,&s1);
	MPI_Pack_size(2,MPI_DOUBLE,comm,&s2);
	MPI_Pack_size(17,MPI_CHAR,comm,&s3);

	bufsize=3*MPI_BSEND_OVERHEAD+s1+s2+s3;
	buf=(char*)malloc(bufsize);

	MPI_Buffer_attach(buf,bufsize);

	strncpy(msg1,"0123456",7);
	strncpy(msg3,"0123401234012341",17);
	msg2[0]=1.23;
	msg2[1]=3.21;

	if(rank==src){
		MPI_Bsend(msg1,7,MPI_CHAR,dest,tag,comm);
		MPI_Bsend(msg2,2,MPI_DOUBLE,dest,tag,comm);
		MPI_Bsend(msg3,17,MPI_CHAR,dest,tag,comm);
	}

	if(rank==dest){
		MPI_Recv(rmsg1,7,MPI_CHAR,src,tag,comm,MPI_STATUS_IGNORE);
		MPI_Recv(rmsg2,7,MPI_DOUBLE,src,tag,comm,MPI_STATUS_IGNORE);
		MPI_Recv(rmsg3,17,MPI_CHAR,src,tag,comm,MPI_STATUS_IGNORE);
		if(strcmp(rmsg1,msg1)!=0){
			errs++;
			fprintf(stderr,"message 1 (%s) should be %s\n",rmsg1,msg1);
			fflush(stderr);
		}

		if(rmsg2[0]!=msg2[0]||rmsg2[1]!=msg2[1]){
			errs++;
			fprintf(stderr,"message 2 incorrect, values are (%f,%f) but should be (%f,%f)\n",rmsg2[0],rmsg2[1],msg2[0],msg2[1]);
			fflush(stderr);
		}

		if(strcmp(rmsg3,msg3)!=0){
			errs++;
			fprintf(stderr,"message 3 (%s) should be %s\n",rmsg3,msg3);
			fflush(stderr);
		}
		fprintf(stderr,"rmsg1:%s\n",rmsg1);
		fprintf(stderr,"rmsg2:%f,%f\n",rmsg2[0],rmsg2[1]);
		fprintf(stderr,"rmsg3:%s\n",rmsg3);
	}

	MPI_Buffer_detach(&buf,&bufsize);
	free(buf);
	MPI_Finalize();
	return 0;
}
