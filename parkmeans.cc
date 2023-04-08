/********************
*   Martin Takacs   *
*     xtakac07      *
*    PRL project    *
*  Parallel K-Means *
********************/

#include <iostream>
#include <mpi.h>
#include <vector>
#include <fstream>
#include <string>

int main(int argc, char** argv){
    MPI_Init(&argc, &argv);

    // Read input from a file
	const std::string filename = "numbers";
	std::ifstream file(filename);
	std::string line;
	std::getline(file, line);

	// Init buffer
	std::vector<int> sendbuf;

	// Convert to numbers
	for (char c : line) {
		sendbuf.push_back(static_cast<unsigned char>(c));
	}

	// int n = sendbuf.size();
    for (int x : sendbuf)
        std::cout << x << " ";
    std::cout<<std::endl;

    // Get rank and size
	int rank, size;	
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);







    MPI_Finalize();

    return 0;
}