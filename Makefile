all:
	g++ lab6.cpp -I"C:\Program Files (x86)\Microsoft SDKs\MPI\Include" -L"C:\Program Files (x86)\Microsoft SDKs\MPI\Lib\x64" -lmsmpi -o lab6
	mpiexec -n 5 lab6.exe

test:
	./run_mpi.sh

clear:
	rm files/*.txt