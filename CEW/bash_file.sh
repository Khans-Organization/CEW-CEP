#!/bin/bash

# Function to handle the termination of the loop on Ctrl + C
trap 'echo -e "\nProcess has been terminated."; break' SIGINT

# Inform the user that Ctrl + C will stop the loop
echo "Press Ctrl + C to stop the repeating beep process."

# Loop to print a beep after each iteration
while true; do
 # Run the weather program
    ./run_file
    # Send the bell character to terminal to generate a beep sound
    echo -e "\a"
    sleep 2  # Wait for 2 second between beeps (adjust as needed)
done


