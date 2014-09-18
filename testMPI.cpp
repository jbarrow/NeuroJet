#include "mpi.h"
#include <iostream>

using namespace std;

int main(int argc, char *argv[]) {
   // Initialize MPI
   MPI_Init(&argc, &argv);
   // Get the processor number of the current node
	int rank; // can't be unsigned
   MPI_Comm_rank(MPI_COMM_WORLD, &rank);
   // Get the total number of nodes
	int numNodes; // can't be unsigned
   MPI_Comm_size(MPI_COMM_WORLD, &numNodes);
	cout << "My rank is: " << rank << " and I think there are "
        << numNodes << " nodes to talk to." << endl;
}
