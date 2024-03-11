#!/bin/sh
#
# DON'T EDIT THIS!
#
# CodeCrafters uses this file to test your code. Don't make any changes here!
#
# DON'T EDIT THIS!
set -e

# Record the start time
start_time=$(date +%s)

# Run CMake to generate build files (suppress output)
cmake . >/dev/null

# Build the project with make using parallel compilation
make -j$(nproc) >/dev/null

# Record the end time
end_time=$(date +%s)

# Calculate the total execution time
execution_time=$((end_time - start_time))

# Print the execution time
echo "Total execution time: $execution_time seconds"

# Execute the server binary with any command-line arguments passed to this script
exec ./server "$@"
