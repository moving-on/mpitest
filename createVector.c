#include<stdio.h>
#include<stdlib.h>
#include<time.h>

int main(int argc,char** argv)
{
	int i,j;
	int n;
	if(argc<2)
	{
		fprintf(stderr,"not given enough param\n");
		exit(-1);
	}
	if(argc==2)
	{
		n=atoi(argv[1]);
	}
	FILE* outfile=fopen("vector","wb");
	if(outfile==NULL)
	{
		fprintf(stderr,"open file error\n");
		fflush(stderr);
		exit(-1);
	}
	fwrite((char*)&n,sizeof(int),1,outfile);
	srand((unsigned int)(time(NULL)));
	for(i=0;i<n;i++)
	{
		double num=(double)(rand()%20);
		fwrite((char*)&num,sizeof(double),1,outfile);
		fprintf(stdout,"%6.3f ",num);
	}
	fprintf(stdout,"\n");
	fclose(outfile);
}
