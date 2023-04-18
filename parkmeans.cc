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

    // Get rank and size
	int rank, size;	
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	// Init buffers
	const int clusterCount = 4;
	std::vector<int> sendbuf;


    // Read input from a file
	const std::string filename = "numbers";
	std::ifstream file(filename);
	std::string line;
	std::getline(file, line);

	// Convert ASCII to numbers
	for (char c : line) {
		sendbuf.push_back(static_cast<unsigned char>(c));
	}

	

	// K-means
	// 1. Select 4 random distinct numbers (cluster means)
	// We know that input is an array of random numbers (at least 4)
	// so we can just take the first 4 numbers as cluster means
	struct 
	{
		float mean;
		std::vector<int> points;
	}clusters[clusterCount];
	
	
	int means[clusterCount];
	for (int i=0; i<4; ++i){
		clusters[i].mean = (float)sendbuf[i];
		MPI_Bcast(&clusters[i].mean, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);
	}

	// Create buffer for each processor and scatter data
	int subArrSize = sendbuf.size() / size;
	std::vector<int> recvbuf(subArrSize);
	MPI_Scatter(sendbuf.data(), subArrSize, MPI_INT, recvbuf.data(), subArrSize, MPI_INT, 0, MPI_COMM_WORLD);
	
	std::cout << "Processor " << rank << std::endl;
    for (int x : recvbuf)
        std::cout << x << " ";
    std::cout<<std::endl;
	std::cout << "means: ";
	for (int i; i<clusterCount; ++i)
		std::cout << clusters[i].mean << " ";
	std::cout<<std::endl;

	// 2. Calculate distances for every other point



	// 3. Assign every point to the nearest cluster
	// 4. Calculate new mean for every cluster
	// 5. Calculate distances for each point to every cluster
	// 6. Reassign points to their respective cluster
	// 7. Repeat from step 4. until means remain the same



	// if (rank == 0){
	// 	for (int i : means)
	// 		std::cout<<i<<" ";
	// 	std::cout<<std::endl;
	// }

    MPI_Finalize();

    return 0;
}