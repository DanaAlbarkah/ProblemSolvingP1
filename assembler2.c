#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Function to translate assembly instruction to machine code (binary representation)
unsigned char translate_instruction(const char* instruction) {
    if (strcmp(instruction, "RA=RA+RB") == 0) {
        return 0x00;
    } else if (strcmp(instruction, "RB=RA+RB") == 0) {
        return 0x10;
    } else if (strcmp(instruction, "RA=RA-RB") == 0) {
        return 0x04;
    } else if (strcmp(instruction, "RB=RA-RB") == 0) {
        return 0x14;
    } else if (strcmp(instruction, "RO=RA") == 0) {
        return 0x20;
    }
    // Immediate loads to RA
    else if (strcmp(instruction, "RA=0") == 0) {
        return 0x08;
    } else if (strcmp(instruction, "RA=1") == 0) {
        return 0x09;
    } else if (strcmp(instruction, "RA=2") == 0) {
        return 0x0A;
    } else if (strcmp(instruction, "RA=3") == 0) {
        return 0x0B;
    } else if (strcmp(instruction, "RA=4") == 0) {
        return 0x0C;
    } else if (strcmp(instruction, "RA=5") == 0) {
        return 0x0D;
    } else if (strcmp(instruction, "RA=6") == 0) {
        return 0x0E;
    } else if (strcmp(instruction, "RA=7") == 0) {
        return 0x0F;
    }
    // Immediate loads to RB
    else if (strcmp(instruction, "RB=0") == 0) {
        return 0x18;
    } else if (strcmp(instruction, "RB=1") == 0) {
        return 0x19;
    } else if (strcmp(instruction, "RB=2") == 0) {
        return 0x1A;
    } else if (strcmp(instruction, "RB=3") == 0) {
        return 0x1B;
    } else if (strcmp(instruction, "RB=4") == 0) {
        return 0x1C;
    } else if (strcmp(instruction, "RB=5") == 0) {
        return 0x1D;
    } else if (strcmp(instruction, "RB=6") == 0) {
        return 0x1E;
    } else if (strcmp(instruction, "RB=7") == 0) {
        return 0x1F;
    }
    // Jump if carry (JC) to immediate
    else if (strcmp(instruction, "JC=0") == 0) {
        return 0x70;
    } else if (strcmp(instruction, "JC=1") == 0) {
        return 0x79;
    } else if (strcmp(instruction, "JC=2") == 0) {
        return 0x7A;
    } else if (strcmp(instruction, "JC=3") == 0) {
        return 0x7B;
    } else if (strcmp(instruction, "JC=4") == 0) {
        return 0x7C;
    } else if (strcmp(instruction, "JC=5") == 0) {
        return 0x7D;
    } else if (strcmp(instruction, "JC=6") == 0) {
        return 0x7E;
    } else if (strcmp(instruction, "JC=7") == 0) {
        return 0x7F;
    }
    // Unconditional jump (J) to immediate
    else if (strcmp(instruction, "J=0") == 0) {
        return 0x70;
    } else if (strcmp(instruction, "J=1") == 0) {
        return 0xB9;
    } else if (strcmp(instruction, "J=2") == 0) {
        return 0xB2;
    } else if (strcmp(instruction, "J=3") == 0) {
        return 0xBB;
    } else if (strcmp(instruction, "J=4") == 0) {
        return 0xBC;
    } else if (strcmp(instruction, "J=5") == 0) {
        return 0xBD;
    } else if (strcmp(instruction, "J=6") == 0) {
        return 0xBE;
    } else if (strcmp(instruction, "J=7") == 0) {
        return 0xBF;
    } else {
        return 0xFF; // Return 0xFF as an error marker for unrecognized instructions
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input file> <output file>\n", argv[0]);
        return 1;
    }

    FILE *inputFile = fopen(argv[1], "r");
    if (!inputFile) {
        fprintf(stderr, "Error: Could not open input file '%s'\n", argv[1]);
        return 1;
    }

    FILE *outputFile = fopen(argv[2], "wb");  // Open in binary mode
    if (!outputFile) {
        fprintf(stderr, "Error: Could not create output file '%s'\n", argv[2]);
        fclose(inputFile);
        return 1;
    }

    char line[256];
    while (fgets(line, sizeof(line), inputFile)) {
        line[strcspn(line, "\n")] = 0; // Remove newline character
        unsigned char machine_code = translate_instruction(line);
        if (machine_code != 0xFF) {
            fwrite(&machine_code, sizeof(unsigned char), 1, outputFile);
        } else {
            fprintf(stderr, "Error: Unrecognized instruction '%s'\n", line);
        }
    }

    fclose(inputFile);
    fclose(outputFile);

    printf("Successfully generated binary output file: %s\n", argv[2]);
    return 0;
}

