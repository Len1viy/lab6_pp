def read_data(filename):
    threads = []
    times = []
    with open(filename, 'r') as file:
        for line in file:
            parts = line.split()
            thread = int(parts[0])
            time = float(parts[1])
            threads.append(thread)
            times.append(time)

    return threads, times