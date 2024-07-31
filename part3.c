#include <stdio.h>
#include <stdlib.h>

// Function to read sequence of logical addresses from a binary file
int* read_sequence(const char* filename, int* count) {
    // Open the file in binary read mode
    FILE* file = fopen(filename, "rb");
    if (!file) {
        perror("Error opening sequence file");
        exit(EXIT_FAILURE);
    }

    int* sequence = NULL; // Pointer to store the sequence of addresses
    int addr; // Variable to store each address read from the file
    *count = 0; // Initialize the count of addresses to 0

    // Read addresses from the file until the end of the file is reached
    while (fread(&addr, sizeof(int), 1, file) == 1) {
        // Reallocate memory to store the new address
        sequence = realloc(sequence, (*count + 1) * sizeof(int));
        if (sequence == NULL) {
            perror("Error reallocating memory");
            exit(EXIT_FAILURE);
        }
        sequence[*count] = addr; // Store the address in the sequence
        (*count)++; // Increment the count of addresses
    }

    fclose(file); // Close the file
    return sequence; // Return the sequence of addresses
}

// Function to translate logical addresses to physical addresses and track page faults using FIFO
int translate_addresses(int* sequence, int count, int bytesPerPage, int physicalSize, const char* outputFile) {
    // Open the output file in write mode
    FILE* file = fopen(outputFile, "w");
    if (!file) {
        perror("Error opening output file");
        exit(EXIT_FAILURE);
    }

    int numFrames = physicalSize / bytesPerPage; // Calculate the number of frames
    int* pageTable = (int*)malloc(numFrames * sizeof(int)); // Allocate memory for the page table
    int* frameQueue = (int*)malloc(numFrames * sizeof(int)); // Allocate memory for the FIFO queue

    // Initialize the page table and frame queue
    for (int i = 0; i < numFrames; i++) {
        pageTable[i] = -1;
        frameQueue[i] = -1;
    }

    int pageFaults = 0; // Counter for page faults
    int queueHead = 0; // Head of the FIFO queue
    int queueTail = 0; // Tail of the FIFO queue
    int pagesInMemory = 0; // Counter for pages in memory
    int mask = bytesPerPage - 1; // Mask to extract the offset

    // Iterate through the sequence of logical addresses
    for (int i = 0; i < count; i++) {
        int logicalAddress = sequence[i]; // Get the logical address
        int pageNumber = logicalAddress / bytesPerPage; // Calculate the page number
        int offset = logicalAddress & mask; // Calculate the offset
        int physicalAddress; // Variable to store the physical address

        int frameNumber = -1; // Variable to store the frame number
        // Check if the page is already in memory
        for (int j = 0; j < numFrames; j++) {
            if (pageTable[j] == pageNumber) {
                frameNumber = j;
                break;
            }
        }

        if (frameNumber == -1) { // Page fault
            pageFaults++; // Increment the page fault counter
            if (pagesInMemory < numFrames) {
                frameNumber = pagesInMemory; // Use a free frame
                pagesInMemory++; // Increment the counter for pages in memory
            } else {
                frameNumber = frameQueue[queueHead]; // Use the frame at the head of the FIFO queue
                queueHead = (queueHead + 1) % numFrames; // Update the head of the FIFO queue
            }
            pageTable[frameNumber] = pageNumber; // Update the page table
            frameQueue[queueTail] = frameNumber; // Update the FIFO queue
            queueTail = (queueTail + 1) % numFrames; // Update the tail of the FIFO queue
        }

        physicalAddress = frameNumber * bytesPerPage + offset; // Calculate the physical address
        fprintf(file, "%d\n", physicalAddress); // Write the physical address to the output file
    }

    fclose(file); // Close the output file
    free(pageTable); // Free the memory allocated for the page table
    free(frameQueue); // Free the memory allocated for the FIFO queue

    return pageFaults; // Return the number of page faults
}

int main(int argc, char* argv[]) {
    // Check if the correct number of arguments is provided
    if (argc != 6) {
        fprintf(stderr, "Usage: %s BytesPerPage SizeOfVirtualMemory SizeOfPhysicalMemory SequenceFile OutputFile\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int bytesPerPage = atoi(argv[1]); // Get the number of bytes per page
    int physicalSize = atoi(argv[3]); // Get the size of the physical memory
    const char* sequenceFile = argv[4]; // Get the name of the sequence file
    const char* outputFile = argv[5]; // Get the name of the output file

    int count; // Variable to store the count of addresses
    int* sequence = read_sequence(sequenceFile, &count); // Read the sequence of logical addresses

    int pageFaults = translate_addresses(sequence, count, bytesPerPage, physicalSize, outputFile); // Translate the addresses and track page faults

    free(sequence); // Free the memory allocated for the sequence

    printf("Page faults: %d\n", pageFaults); // Print the number of page faults
    return 0; // Return success
}