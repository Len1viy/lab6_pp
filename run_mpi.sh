#!/bin/bash

EXECUTABLE="./lab5.exe"

if [ ! -x "$EXECUTABLE" ]; then
    echo "Error: Executable file $EXECUTABLE does not exist or is not executable."
    exit 1
fi

for PROCESSES in {2..16}; do
    echo "Running with $PROCESSES processes"
    mpiexec -n $PROCESSES $EXECUTABLE
done

echo "All runs completed."