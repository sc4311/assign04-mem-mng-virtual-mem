#include <stdio.h>
#include <stdlib.h>

#define NUM_PAGES 8 // Define the number of pages

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
    int PT[NUM_PAGES] = {2, 4, 1, 7, 3, 5, 6, -1}; // Fixed page table

    // Read logical addresses from the input file
    while (fread(&LA, sizeof(unsigned long), 1, infile)) {
        int pnum = LA >> 7; // Get page number by shifting right 7 bits
        int dnum = LA & 0x7F; // Get offset by masking lower 7 bits

        // Check if the page number is valid
        if (pnum < 0 || pnum >= NUM_PAGES || PT[pnum] == -1) {
            fprintf(stderr, "Invalid page number %d\n", pnum);
            fclose(infile);
            fclose(outfile);
            return 1;
        }

        int fnum = PT[pnum]; // Get frame number from the page table
        PA = (fnum << 7) + dnum; // Calculate physical address

        // Write physical address to the output file
        fwrite(&PA, sizeof(unsigned long), 1, outfile);
    }

    // Close the input and output files
    fclose(infile);
    fclose(outfile);

    return 0;
}