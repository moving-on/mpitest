#include<stdio.h>
#include<stdlib.h>
#include<time.h>

int main(int argc,char** argv)
{
	int i,j;
	int m,n;
	if(argc<2)
	{
		fprintf(stderr,"not given enough param\n");
		exit(-1);
	}
	if(argc==2)
	{
		m=n=atoi(argv[1]);
	}
	if(argc==3)
	{
		m=atoi(argv[1]);
		n=atoi(argv[2]);
	}
	FILE* outfile=fopen("matrix","wb");
	if(outfile==NULL)
	{
		fprintf(stderr,"open file error\n");
		fflush(stderr);
		exit(-1);
	}
	fwrite((char*)&m,sizeof(int),1,outfile);
	fwrite((char*)&n,sizeof(int),1,outfile);
	srand((unsigned int)(time(NULL)));
	//buffer=(double*)malloc(sizeof(double)*n);
	for(i=0;i<m;i++)
	{
		for(j=0;j<n;j++)
		{
			double num=(double)(rand()%20);
			fwrite((char*)&num,sizeof(double),1,outfile);
			fprintf(stdout,"%6.3f ",num);
		}
		putchar('\n');
	}
	fclose(outfile);
}
