#!/bin/bash

# File to store PIDs
PID_FILE=".producer_consumer.pid"

# Function to cleanup
cleanup() {
    echo "Cleaning up..."
    if [ -f "$PID_FILE" ]; then
        while read pid; do
            if ps -p $pid > /dev/null; then
                kill $pid 2>/dev/null
            fi
        done < "$PID_FILE"
        rm "$PID_FILE"
    fi
    # No manual /dev/shm cleanup needed on macOS; handled by C code
}

# Handle script termination
trap cleanup EXIT INT TERM

# Compile the programs
make

# Start producer
./producer > producer.log &
echo $! > "$PID_FILE"

# Start consumers (adjust number as needed)
for i in {1..2}; do
    ./consumer > "consumer$i.log" &
    echo $! >> "$PID_FILE"
done

echo "All processes started. Press Ctrl+C to stop all processes."
echo "Logs are being written to producer.log and consumer*.log"

# Wait for user interrupt
wait 