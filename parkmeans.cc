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
#include <cmath>

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
		int size = 0;
	}clusters[clusterCount];
	
	
	int means[clusterCount];
	for (int i=0; i<clusterCount; ++i){
		clusters[i].mean = (float)sendbuf[i];
		MPI_Bcast(&clusters[i].mean, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);
	}

	// Create buffer for each processor and scatter data
	int subArrSize = sendbuf.size() / size;
	std::vector<int> recvbuf(subArrSize);
	MPI_Scatter(sendbuf.data(), subArrSize, MPI_INT, recvbuf.data(), subArrSize, MPI_INT, 0, MPI_COMM_WORLD);
	

	// 2. Calculate distances for every other point
	// 3. Assign every point to the nearest cluster
	for (int x : recvbuf){
		// Find min distance from clusters means
		int cIndex = 0;
		float minDist = MAXFLOAT;
		for (int i=0; i<clusterCount; ++i){
			float dist = std::abs(x - clusters[i].mean);
			if (dist < minDist){
				minDist = dist;
				cIndex = i;
			}
		}
		// Assign point to the cluster
		clusters[cIndex].points.push_back(x);
		clusters[cIndex].size++;
	}

	// Merge clusters from each processor
	for (int c=0; c<clusterCount; ++c){
		std::vector<int> cSizes(size);
		int cluster_size = clusters[c].points.size();
		MPI_Gather(&cluster_size, 1, MPI_INT, cSizes.data(), 1, MPI_INT, 0, MPI_COMM_WORLD);

		if (rank==0){
			int total_size = 0;
			for (int s : cSizes)
				total_size += s;
			
			std::vector<int> joinedCluster(total_size);

			// Displacements
			std::vector<int> displs(size);
			for (int j=1; j<size; ++j)
				displs[j] = displs[j-1] + cSizes[j-1];

			// Join into a cluster
			MPI_Gatherv(clusters[c].points.data(), cluster_size, MPI_INT, joinedCluster.data(),
					cSizes.data(), displs.data(), MPI_INT, 0, MPI_COMM_WORLD);

			printf("Cluster[%d]: ", c);
			for (int x : joinedCluster)
				std::cout<< x << " ";
			std::cout<< std::endl;
		}
		else{
			// Send points to the root process
			MPI_Gatherv(clusters[c].points.data(), cluster_size, MPI_INT, 
					NULL, NULL, NULL, MPI_INT, 0, MPI_COMM_WORLD);
		}
	}


// if (rank==1){
	// std::cout << "Processor " << rank << std::endl;
//     // for (int x : recvbuf)
//     //     std::cout << x << " ";
//     // std::cout<<std::endl;
// 	// std::cout << "means: ";
// 	// for (int i = 0; i<clusterCount; ++i)
// 	// 	std::cout << clusters[i].mean << " ";
// 	// std::cout<<std::endl;
	// for (int i=0; i<clusterCount; ++i){
	// 	std::cout << "Cluster["<<i<<"]: mean: "<<clusters[i].mean<< " \tpoints: ";
	// 	for (int x : clusters[i].points)
	// 		std::cout << x << " ";
	// 	std::cout << "\tsize: "<<clusters[i].size;
	// 	std::cout<<std::endl;
	// }
// }

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