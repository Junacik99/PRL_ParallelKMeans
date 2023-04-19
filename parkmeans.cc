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

// TODO: prerobit classu, aby zodpovedala c++ praktikam
class Cluster
{
public:
	float mean;
	std::vector<int> points;
	float prevMean;
};

std::vector<int> getDisplacements(int size, std::vector<int> cSizes);
int getTotalSize(std::vector<int> cSizes);
void points2cluster(std::vector<int> buffer, int clusterCount, Cluster clusters[]);

int main(int argc, char **argv)
{
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
	for (char c : line)
	{
		sendbuf.push_back(static_cast<unsigned char>(c));
	}

	// K-means
	// 1. Select 4 random distinct numbers (cluster means)
	// We know that input is an array of random numbers (at least 4)
	// so we can just take the first 4 numbers as cluster means
	Cluster clusters[clusterCount];

	// Assign and broadcast means
	for (int i = 0; i < clusterCount; ++i)
	{
		clusters[i].mean = (float)sendbuf[i];
		clusters[i].prevMean = clusters[i].mean;
		MPI_Bcast(&clusters[i].mean, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);
		MPI_Bcast(&clusters[i].prevMean, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);
	}

	// Create buffer for each processor and scatter data
	int subArrSize = sendbuf.size() / size;
	std::vector<int> recvbuf(subArrSize);
	MPI_Scatter(sendbuf.data(), subArrSize, MPI_INT, recvbuf.data(), subArrSize, MPI_INT, 0, MPI_COMM_WORLD);

	// 2. Calculate distances for every other point
	// 3. Assign every point to the nearest cluster
	points2cluster(recvbuf, clusterCount, clusters);

	int iterations = 0;
	int sameMeans = 0;
	do
	{
		iterations++;

		// Merge clusters from each processor
		for (int c = 0; c < clusterCount; ++c)
		{
			// Get sizes of the cluster in each processor
			std::vector<int> cSizes(size);
			int cluster_size = clusters[c].points.size();
			MPI_Gather(&cluster_size, 1, MPI_INT, cSizes.data(), 1, MPI_INT, 0, MPI_COMM_WORLD);

			if (rank == 0)
			{
				// Reset same means counter
				sameMeans = 0;

				// Calculate total size of a cluster
				int total_size = getTotalSize(cSizes);

				// Displacements
				std::vector<int> displs = getDisplacements(size, cSizes);

				// Join into a cluster
				std::vector<int> joinedCluster(total_size);
				MPI_Gatherv(clusters[c].points.data(), cluster_size, MPI_INT, joinedCluster.data(),
							cSizes.data(), displs.data(), MPI_INT, 0, MPI_COMM_WORLD);

				// Update previous mean
				clusters[c].prevMean = clusters[c].mean;

				// 4. Calculate new mean for every cluster
				int sum = 0;
				for (int x : joinedCluster)
					sum += x;
				clusters[c].mean = (float)sum / joinedCluster.size();
			}
			else
			{
				// Send points to the root process
				MPI_Gatherv(clusters[c].points.data(), cluster_size, MPI_INT,
							NULL, NULL, NULL, MPI_INT, 0, MPI_COMM_WORLD);
			}
		}

		for (int i = 0; i < clusterCount; ++i)
		{
			// Broadcast new means
			MPI_Bcast(&clusters[i].mean, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);
			MPI_Bcast(&clusters[i].prevMean, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);
			// Clear current cluster vectors
			clusters[i].points.clear();
			// Calculate same means
			if (rank == 0 && clusters[i].mean == clusters[i].prevMean)
			{
				sameMeans++;
			}
		}

		// 5. Calculate distances for each point to every cluster
		// 6. Reassign points to their respective cluster
		points2cluster(recvbuf, clusterCount, clusters);

		// 7. Repeat from step 4. until means remain the same
		MPI_Bcast(&sameMeans, 1, MPI_INT, 0, MPI_COMM_WORLD);
	} while (sameMeans < clusterCount);

	// Finish
	// A little repetitive code to gather clusters once more to print them out
	// But algorithm works even without this just fine (Calculating means)
	if (rank == 0)
		printf("Convergence has been achieved after %d iterations!\n", iterations);
		
	for (Cluster c : clusters)
	{
		std::vector<int> cSizes(size);
		int cluster_size = c.points.size();
		MPI_Gather(&cluster_size, 1, MPI_INT, cSizes.data(), 1, MPI_INT, 0, MPI_COMM_WORLD);

		if (rank == 0)
		{

			// Calculate total size of a cluster
			int total_size = getTotalSize(cSizes);

			// Displacements
			std::vector<int> displs = getDisplacements(size, cSizes);

			// Join into a cluster
			std::vector<int> joinedCluster(total_size);
			MPI_Gatherv(c.points.data(), cluster_size, MPI_INT, joinedCluster.data(),
						cSizes.data(), displs.data(), MPI_INT, 0, MPI_COMM_WORLD);

			// Print out results
			printf("[%.2f] ", c.mean);
			for (int point : joinedCluster)
				printf("%d ", point);
			std::cout << std::endl;
		}
		else
		{
			// Send points to the root process
			MPI_Gatherv(c.points.data(), cluster_size, MPI_INT,
						NULL, NULL, NULL, MPI_INT, 0, MPI_COMM_WORLD);
		}
	}

	MPI_Finalize();

	return 0;
}

// Calculate displacements
std::vector<int> getDisplacements(int size, std::vector<int> cSizes)
{
	std::vector<int> displs(size);
	for (int j = 1; j < size; ++j)
		displs[j] = displs[j - 1] + cSizes[j - 1];
	return displs;
}

// Calculate total size of a cluster
int getTotalSize(std::vector<int> cSizes)
{
	int total_size = 0;
	for (int s : cSizes)
		total_size += s;
	return total_size;
}

void points2cluster(std::vector<int> buffer, int clusterCount, Cluster clusters[])
{
	for (int x : buffer)
	{
		// Find min distance from clusters means
		int cIndex = 0;
		float minDist = MAXFLOAT;
		for (int i = 0; i < clusterCount; ++i)
		{
			float dist = std::abs(x - clusters[i].mean);
			if (dist < minDist)
			{
				minDist = dist;
				cIndex = i;
			}
		}
		// Assign point to the cluster
		clusters[cIndex].points.push_back(x);
	}
}