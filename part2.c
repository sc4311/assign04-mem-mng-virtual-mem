#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_FRAMES 8
#define FRAME_SIZE 128
#define PAGE_TABLE_SIZE 256 // Fixed size for page table

// Page Table Entry structure
typedef struct {
    int valid_bit; // Valid bit to check if the page is in memory
    int frame_number; // Frame number where the page is stored
} PTE;

// Function to update the LRU counters
void update_LRU(int LRUcount[], int frame) {
    // Iterate through all frames starting from 1 (0 is reserved for OS)
    for (int i = 1; i < NUM_FRAMES; i++) {
        // Decrement the LRU count for frames with higher count than the current frame
        if (LRUcount[i] > LRUcount[frame]) {
            LRUcount[i]--;
        }
    }
    // Set the LRU count of the current frame to the maximum value
    LRUcount[frame] = NUM_FRAMES - 1;
}

// Function to find a free frame
int find_free_frame(int freeframes[]) {
    // Iterate through all frames starting from 1 (0 is reserved for OS)
    for (int i = 1; i < NUM_FRAMES; i++) {
        // Return the frame number if it is free
        if (freeframes[i] == 1) {
            return i;
        }
    }
    // Return -1 if no free frame is found
    return -1;
}

// Function to find the Least Recently Used (LRU) frame
int find_LRU_frame(int LRUcount[]) {
    int min_count = LRUcount[1]; // Initialize with the LRU count of the first frame
    int frame = 1; // Initialize with the first frame
    // Iterate through all frames starting from 1 (0 is reserved for OS)
    for (int i = 2; i < NUM_FRAMES; i++) {
        // Update the frame with the minimum LRU count
        if (LRUcount[i] < min_count) {
            min_count = LRUcount[i];
            frame = i;
        }
    }
    // Return the frame with the minimum LRU count
    return frame;
}

int main(int argc, char *argv[]) {
    // Check if the correct number of arguments is provided
    if (argc != 3) {
        fprintf(stderr, "Usage: %s infile outfile\n", argv[0]);
        return 1;
    }

    // Open the input file in binary read mode
    FILE *infile = fopen(argv[1], "rb");
    if (!infile) {
        perror("Error opening input file");
        return 1;
    }

    // Open the output file in binary write mode
    FILE *outfile = fopen(argv[2], "wb");
    if (!outfile) {
        perror("Error opening output file");
        fclose(infile);
        return 1;
    }

    unsigned long LA, PA; // Logical address and physical address
    PTE PT[PAGE_TABLE_SIZE]; // Page table
    int freeframes[NUM_FRAMES] = {0, 1, 1, 1, 1, 1, 1, 1}; // Free frames, frame 0 is reserved for OS
    int revmap[NUM_FRAMES]; // Reverse mapping from frames to pages
    int LRUcount[NUM_FRAMES]; // LRU counters

    // Initialize page table, reverse map, and LRU count
    for (int i = 0; i < PAGE_TABLE_SIZE; i++) {
        PT[i].valid_bit = 0;
        PT[i].frame_number = -1;
    }

    for (int i = 0; i < NUM_FRAMES; i++) {
        revmap[i] = -1;
        LRUcount[i] = 0;
    }

    int page_faults = 0; // Counter for page faults

    // Read logical addresses from the input file
    while (fread(&LA, sizeof(unsigned long), 1, infile)) {
        unsigned long pnum = LA >> 7; // Get page number by shifting right 7 bits
        unsigned long dnum = LA & 0x7F; // Get offset by masking lower 7 bits

        if (PT[pnum].valid_bit == 1) {
            // Page is valid, use existing frame
            PA = (PT[pnum].frame_number << 7) + dnum;
        } else {
            // Page fault
            page_faults++;
            int frame = find_free_frame(freeframes);

            if (frame == -1) {
                // No free frame, use LRU policy
                frame = find_LRU_frame(LRUcount);
                // Invalidate old page
                int old_pnum = revmap[frame];
                if (old_pnum != -1) {
                    PT[old_pnum].valid_bit = 0;
                }
            }

            // Update page table
            PT[pnum].valid_bit = 1;
            PT[pnum].frame_number = frame;
            revmap[frame] = pnum;
            freeframes[frame] = 0;

            // Calculate physical address
            PA = (frame << 7) + dnum;
        }

        // Update LRU counters
        update_LRU(LRUcount, PT[pnum].frame_number);

        // Write physical address to the output file
        fwrite(&PA, sizeof(unsigned long), 1, outfile);
    }

    // Close the input and output files
    fclose(infile);
    fclose(outfile);

    // Print the number of page faults
    printf("Part 2 page faults: %d\n", page_faults);

    return 0;
}