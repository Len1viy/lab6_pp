#include <iostream>
#include <vector>
#include <mpi.h>
#include <random>

void create_vector(std::vector<int> &array, const int array_size) {
    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_int_distribution<> dis(-1e7, 1e7);
    
    for (int i = 0; i < array_size; i++) {
        array[i] = dis(gen);
    }
}

// Функция для локальной сортировки Шелла
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

// Функция для слияния двух отсортированных подмассивов
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

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int global_size = 19;
    int local_size = (int)(global_size / size);
    int last_size = global_size % size;
    std::vector<int> global_data(global_size), local_data(local_size);

    if (rank == 0) { 
        create_vector(global_data, global_size);
    }

    

    // Распределяем данные между процессами
    MPI_Scatter(global_data.data(), local_size, MPI_INT, local_data.data(), local_size, MPI_INT, 0, MPI_COMM_WORLD);

    // Выполняем локальную сортировку Шелла
    shell_sort(local_data);

    // Процессы обмениваются крайними элементами с соседними процессами для слияния
    for (int step = 0; step < size; ++step) {
        // Четно-нечетное слияние
        if ((rank % 2 == 0 && step % 2 == 0) || (rank % 2 != 0 && step % 2 != 0)) {
            // Четная фаза: обмен с правым соседом
            if (rank < size - 1) {
                int neighbor_rank = rank + 1;
                std::vector<int> neighbor_data(local_size);
                MPI_Sendrecv(local_data.data(), local_size, MPI_INT, neighbor_rank, 0,
                             neighbor_data.data(), local_size, MPI_INT, neighbor_rank, 0,
                             MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                // Слияние и выборка нужной части
                std::vector<int> merged = merge_sorted_segments(local_data, neighbor_data);
                std::copy(merged.begin(), merged.begin() + local_size, local_data.begin());
            }
        } else {
            // Нечетная фаза: обмен с левым соседом
            if (rank > 0) {
                int neighbor_rank = rank - 1;
                std::vector<int> neighbor_data(local_size);
                MPI_Sendrecv(local_data.data(), local_size, MPI_INT, neighbor_rank, 0,
                             neighbor_data.data(), local_size, MPI_INT, neighbor_rank, 0,
MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                // Слияние и выборка нужной части
                std::vector<int> merged = merge_sorted_segments(neighbor_data, local_data);
                std::copy(merged.begin() + local_size, merged.end(), local_data.begin());
            }
        }
    }

    // Сбор всех данных обратно на корневом процессе
    MPI_Gather(local_data.data(), local_size, MPI_INT, global_data.data(), local_size, MPI_INT, 0, MPI_COMM_WORLD);


    // Вывод окончательной отсортированной последовательности
    if (rank == 0) {
        std::cout << "Fully sorted array: ";
        for (int num : global_data) {
            std::cout << num << " ";
        }
        std::cout << std::endl;
    }

    MPI_Finalize();
    return 0;
}
