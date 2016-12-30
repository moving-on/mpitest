all: floyd mpi_mv createMatrix createVector doc_classify

createMatrix: createMatrix.c
	gcc -o createMatrix createMatrix.c
createVector: createVector.c
	gcc -o createVector createVector.c
floyd: floyd.c MyMPI.h MyMPI.c
	mpicc -o floyd floyd.c MyMPI.c
mpi_mv: mpi_mv.c MyMPI.h MyMPI.c
	mpicc -o mpi_mv mpi_mv.c MyMPI.c
doc_classify: doc_classify.c MyMPI.h MyMPI.c
	mpicc -o doc_classify doc_classify.c MyMPI.c

