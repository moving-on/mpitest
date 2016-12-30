/******************************************************************************
* FILE: mpi_array.c
* DESCRIPTION: 
*   MPI Example - Array Assignment - C Version
*   This program demonstrates a simple data decomposition. The master task
*   first initializes an array and then distributes an equal portion that
*   array to the other tasks. After the other tasks receive their portion
*   of the array, they perform an addition operation to each array element.
*   They also maintain a sum for their portion of the array. The master task 
*   does likewise with its portion of the array. As each of the non-master
*   tasks finish, they send their updated portion of the array to the master.
*   An MPI collective communication call is used to collect the sums 
*   maintained by each task.  Finally, the master task displays selected 
*   parts of the final array and the global sum of all array elements. 
*   NOTE: the number of MPI tasks must be evenly divided by 4.
* AUTHOR: Blaise Barney
* LAST REVISED: 04/13/05
****************************************************************************/
#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#define  ARRAYSIZE	16000000
#define  MASTER		0

float  data[ARRAYSIZE];

int main (int argc, char *argv[])
{
int   numtasks, taskid, rc, dest, offset, i, j, tag1,
      tag2, source, chunksize; 
float mysum, sum;
float update(int myoffset, int chunk, int myid);
MPI_Status status;

/***** Initializations *****/
MPI_Init(&argc, &argv);
MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
if (numtasks % 4 != 0) {
   printf("Quitting. Number of MPI tasks must be divisible by 4.\n");
   MPI_Abort(MPI_COMM_WORLD, rc);
   exit(0);
   }
//进程号
MPI_Comm_rank(MPI_COMM_WORLD,&taskid);
printf ("MPI task %d has started...\n", taskid);
//每个进程分到的块大小
chunksize = (ARRAYSIZE / numtasks);
tag2 = 1;
tag1 = 2;

/***** Master task only ******/
//进程号为0
if (taskid == MASTER){

  /* Initialize the array */
  //初始化array数据
  sum = 0;
  for(i=0; i<ARRAYSIZE; i++) {
    data[i] =  i * 1.0;
    sum = sum + data[i];
    }
  printf("Initialized array sum = %e\n",sum);

  /* Send each task its portion of the array - master keeps 1st part */
  offset = chunksize;
  //给每个进程发送数据
  for (dest=1; dest<numtasks; dest++) {
    //发送进程处理array数据的偏移位置
    MPI_Send(&offset, 1, MPI_INT, dest, tag1, MPI_COMM_WORLD);
    //发送需要进程处理的array数据
    MPI_Send(&data[offset], chunksize, MPI_FLOAT, dest, tag2, MPI_COMM_WORLD);
    printf("Sent %d elements to task %d offset= %d\n",chunksize,dest,offset);
    offset = offset + chunksize;
    }

  /* Master does its part of the work */
  //0进程自身的处理
  offset = 0;
  mysum = update(offset, chunksize, taskid);

  /* Wait to receive results from each task */
  //从其他进程接收处理结果
  for (i=1; i<numtasks; i++) {
    source = i;
    //接收各个进程的数据偏移
    MPI_Recv(&offset, 1, MPI_INT, source, tag1, MPI_COMM_WORLD, &status);
    //接收各个进程处理的数据
    MPI_Recv(&data[offset], chunksize, MPI_FLOAT, source, tag2,
      MPI_COMM_WORLD, &status);
    }

  /* Get final sum and print sample results */  
  //归约操作，求和
  MPI_Reduce(&mysum, &sum, 1, MPI_FLOAT, MPI_SUM, MASTER, MPI_COMM_WORLD);
  printf("Sample results: \n");
  offset = 0;
  for (i=0; i<numtasks; i++) {
    for (j=0; j<5; j++) 
      printf("  %e",data[offset+j]);
    printf("\n");
    offset = offset + chunksize;
    }
  printf("*** Final sum= %e ***\n",sum);

  }  /* end of master section */



/***** Non-master tasks only *****/
//其它非master的进程
if (taskid > MASTER) {

  /* Receive my portion of array from the master task */
  source = MASTER;
  //从master进程接收数据
  MPI_Recv(&offset, 1, MPI_INT, source, tag1, MPI_COMM_WORLD, &status);
  MPI_Recv(&data[offset], chunksize, MPI_FLOAT, source, tag2, 
    MPI_COMM_WORLD, &status);

  mysum = update(offset, chunksize, taskid);

  /* Send my results back to the master task */
  //将处理完的数据发送给master进程
  dest = MASTER;
  MPI_Send(&offset, 1, MPI_INT, dest, tag1, MPI_COMM_WORLD);
  MPI_Send(&data[offset], chunksize, MPI_FLOAT, MASTER, tag2, MPI_COMM_WORLD);

  //归约操作
  MPI_Reduce(&mysum, &sum, 1, MPI_FLOAT, MPI_SUM, MASTER, MPI_COMM_WORLD);

  } /* end of non-master */


MPI_Finalize();

}   /* end of main */


float update(int myoffset, int chunk, int myid) {
  int i; 
  float mysum;
  /* Perform addition to each of my array elements and keep my sum */
  mysum = 0;
  for(i=myoffset; i < myoffset + chunk; i++) {
    data[i] = data[i] + i * 1.0;
    mysum = mysum + data[i];
    }
  printf("Task %d mysum = %e\n",myid,mysum);
  return(mysum);
  }
