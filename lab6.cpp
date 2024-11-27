#include <iostream>
#include <vector>
#include <mpi.h>
#include <random>
#include <fstream>

void write_results_to_file(int size, double time_ans, double acc_ans, double eff_ans) {
    std::string filename = "files/results.txt";
    std::string time_filename = "files/times.txt";
    std::string acc_filename = "files/acc.txt";
    std::string eff_filename = "files/eff.txt";

    std::ofstream file;
    std::ofstream time_file;
    std::ofstream acc_file;
    std::ofstream eff_file;

    if (size == 1) {
        time_file.open(time_filename, std::ios::out);
        acc_file.open(acc_filename, std::ios::out);
        eff_file.open(eff_filename, std::ios::out);
        file.open(filename, std::ios::out);  
    } else {
        time_file.open(time_filename, std::ios::app);
        acc_file.open(acc_filename, std::ios::app);
        eff_file.open(eff_filename, std::ios::app);
        file.open(filename, std::ios::app);  
    }
    file << size << " " << time_ans << " " << acc_ans << " " << eff_ans << "\n";
    time_file << size << " " << time_ans << "\n";
    acc_file << size << " " << acc_ans << "\n";
    eff_file << size << " " << eff_ans << "\n";
}

void create_vector(std::vector<int> &array, const int array_size) {
    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_int_distribution<> dis(-1e7, 1e7);
    
    for (int i = 0; i < array_size; i++) {
        array[i] = dis(gen);
    }
}

void shell_sort(std::vector<int>& array) {
    int n = array.size();
    for (int gap = n / 2; gap > 0; gap /= 2) {
        for (int i = gap; i < n; i++) {
            int temp = array[i];
            int j;
            for (j = i; j >= gap && array[j - gap] > temp; j -= gap) {
                array[j] = array[j - gap];
            }
            array[j] = temp;
        }
    }
}

std::vector<int> merge_sorted_segments(const std::vector<int>& left, const std::vector<int>& right) {
    std::vector<int> merged;
    size_t i = 0, j = 0;
    while (i < left.size() && j < right.size()) {
        if (left[i] < right[j]) {
            merged.push_back(left[i]);
            i++;
        } else {
            merged.push_back(right[j]);
            j++;
        }
    }
    while (i < left.size()) {
        merged.push_back(left[i]);
        i++;
    }
    while (j < right.size()) {
        merged.push_back(right[j]);
        j++;
    }
    return merged;
}

void print_array(std::vector<int> &array, int array_size) {
    std::cout << "[";
    for (int i = 0; i < array_size - 1; i++) {
        std::cout << array[i] << " ";
    }
    std::cout << array[array_size - 1] << "]";
}

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    double global_time = 0;
    double time_first = 0.781917;

    int global_cool_size = 10;
    int global_size = global_cool_size - global_cool_size % size;
    int local_size = (int)(global_size / size);
    int last_size = global_size % size;
    std::vector<int> global_data(global_size), local_data(local_size);

    if (rank == 0) { 
        create_vector(global_data, global_size);
    }

    
    MPI_Scatter(global_data.data(), local_size, MPI_INT, local_data.data(), local_size, MPI_INT, 0, MPI_COMM_WORLD);

	double start_time = MPI_Wtime();

    shell_sort(local_data);

    for (int step = 0; step < size; ++step) {
        if ((rank % 2 == 0 && step % 2 == 0) || (rank % 2 != 0 && step % 2 != 0)) {
            if (rank < size - 1) {
                int neighbour_rank = rank + 1;
                std::vector<int> neighbour_data(local_size);
                MPI_Sendrecv(local_data.data(), local_size, MPI_INT, neighbour_rank, 0,
                             neighbour_data.data(), local_size, MPI_INT, neighbour_rank, 0,
                             MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                std::vector<int> merged = merge_sorted_segments(local_data, neighbour_data);
                std::copy(merged.begin(), merged.begin() + local_size, local_data.begin());
            }
        } else {
            if (rank > 0) {
                int neighbour_rank = rank - 1;
                std::vector<int> neighbour_data(local_size);
                MPI_Sendrecv(local_data.data(), local_size, MPI_INT, neighbour_rank, 0,
                             neighbour_data.data(), local_size, MPI_INT, neighbour_rank, 0,
MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                std::vector<int> merged = merge_sorted_segments(neighbour_data, local_data);
                std::copy(merged.begin() + local_size, merged.end(), local_data.begin());
            }
        }
    }
    MPI_Gather(local_data.data(), local_size, MPI_INT, global_data.data(), local_size, MPI_INT, 0, MPI_COMM_WORLD);

	double end_time = MPI_Wtime();
    global_time = end_time - start_time;

    if (rank == 0) {
        double global_acc, global_eff;

        if (size == 1) { global_acc = 1; global_eff = 1; }
        else { global_acc = time_first / global_time; global_eff = global_acc / size; }
        
        write_results_to_file(size, global_time, global_acc, global_eff);
    }


    MPI_Finalize();
    return 0;
}
