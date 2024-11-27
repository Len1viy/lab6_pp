from read_file import read_data

import matplotlib

matplotlib.use('TkAgg')
import matplotlib.pyplot as plt
import numpy as np
from read_file import read_data


variations = ['OpenMP', 'MPI']
measurements = ['time', 'acc', 'eff']
suptitles = ['Зависимость времени от количества потоков', 'Зависимость коэффициента ускорения от количества потоков', 'Зависимость эффективности от количества потоков']
directory_first = 'files/'
directory_second = 'lab3_files/'
y_labels = ['Время (сек)', 'Коэффициент ускорения', 'Эффективность']
index_labels = 0
for measure in measurements:
    threads_all_pos = []
    points_all_pos = []
    threads_all_theory = []
    points_all_theory = []
    filenames = (directory_first + measure + ".txt", directory_second + measure + ".txt")

    threads_openmp, points_openmp = read_data(filenames[0])
    threads_mpi, points_mpi = read_data(filenames[1])

    threads_openmp = np.array(threads_openmp)
    points_openmp = np.array(points_openmp)

    threads_mpi = np.array(threads_mpi)
    points_mpi = np.array(points_mpi)

    plt.figure(figsize=(10, 6))
    plt.title(suptitles[measurements.index(measure)])
    plt.ylabel(y_labels[index_labels])
    plt.xlabel('Количество потоков')
    plt.plot(threads_mpi, points_mpi)
    plt.plot(threads_openmp, points_openmp)

    plt.legend(labels=[*variations])


    index_labels += 1
    plt.tight_layout()
    plt.show()