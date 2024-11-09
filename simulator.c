#include <stdio.h>
#include <stdlib.h>

// Define a structure to represent the CPU with registers
typedef struct {
    int RA, RB, RO, PC, Carry;  // Registers: RA, RB, RO; Program Counter (PC); Carry flag
} CPU;

// Define a structure to hold the parts of an instruction
typedef struct {
    int J, C, D1, D0, Sreg, S, imm;  // Instruction fields: jump, carry, destination registers, etc.
} Instruction;

// Function to decode an 8-bit instruction into its individual components
Instruction decode_instruction(unsigned char byte) {
    Instruction inst;
    inst.J = (byte & 0x80) >> 7;       // Bit 7: Jump flag (J)
    inst.C = (byte & 0x40) >> 6;       // Bit 6: Conditional carry flag (C)
    inst.D1 = (byte & 0x20) >> 5;      // Bit 5: Higher bit of destination register (D1)
    inst.D0 = (byte & 0x10) >> 4;      // Bit 4: Lower bit of destination register (D0)
    inst.Sreg = (byte & 0x08) >> 3;    // Bit 3: Source selector (1 for immediate value, 0 for RB)
    inst.S = (byte & 0x04) >> 2;       // Bit 2: ALU operation (0 for ADD, 1 for SUB)
    inst.imm = byte & 0x07;            // Bits 0-2: Immediate value (3 bits)
    return inst;                       // Return the decoded instruction
}

// Arithmetic Logic Unit (ALU) function that performs ADD or SUB and manages the carry flag
int ALU(int op, int a, int b, int *carry) {
    int result = 0;
    *carry = 0;  // Initialize carry to 0 for each operation
    if (op == 0) {  // ADD operation
        result = a + b;
        if (result > 15) {       // If result is greater than 15, it's an overflow for 4 bits
            *carry = 1;          // Set carry to 1 if overflow occurs
            result &= 0xF;       // Limit result to 4 bits by masking with 0xF (only keeps lower 4 bits)
        }
    } else {  // SUB operation
        result = a - b;
        if (result < 0) {        // If result is negative, it's an underflow
            *carry = 1;          // Set carry to 1 if underflow occurs
            result = (result + 16) & 0xF;  // Adjust result to fit 4-bit range by adding 16 and masking
        }
    }
    return result;  // Return the final result after handling overflow/underflow
}

// Function to update the appropriate register (RA, RB, RO) based on D1 and D0 values
// If RO exceeds 13, it resets and restarts the sequence
void DEMUX(int D1, int D0, int value, CPU *cpu) {
    if (D1 == 0 && D0 == 0) {
        cpu->RA = value;         // If D1D0 is 00, store the value in RA
    } else if (D1 == 0 && D0 == 1) {
        cpu->RB = value;         // If D1D0 is 01, store the value in RB
    } else if (D1 == 1 && D0 == 0) {
        cpu->RO = value;         // If D1D0 is 10, store the value in RO
        if (cpu->RO > 13) {      // If RO exceeds 13, reset it and restart Fibonacci sequence
            printf("RO exceeded 13, resetting RO, RA, and RB to restart sequence.\n");
            cpu->RO = 0;         // Reset RO to 0
            cpu->RA = 0;         // Reset RA to 0
            cpu->RB = 1;         // Set RB to 1 to restart Fibonacci sequence
            cpu->PC = 0;         // Restart the program counter to begin sequence again
        }
    }
}

// Executes a single instruction based on the decoded components
void execute_instruction(CPU *cpu, Instruction inst) {
    int result;

    // Check for jump conditions
    if (inst.J == 1) {           // If jump (J) flag is set, make an unconditional jump
        cpu->PC = inst.imm;      // Set Program Counter (PC) to the immediate value for the jump
        return;                  // Exit since we're only jumping, no further operations
    } else if (inst.C == 1 && cpu->Carry) {  // Conditional jump if Carry flag is set
        cpu->PC = inst.imm;                  // Set PC to immediate value if carry is true
        return;                              // Exit since we're only jumping
    }

    // Determine which value to use for B based on Sreg (immediate value or RB)
    int B = (inst.Sreg) ? inst.imm : cpu->RB;  // B is imm if Sreg is 1, otherwise B is RB

    // Perform ALU operation (ADD or SUB) based on S flag and store the result
    result = ALU(inst.S, cpu->RA, B, &cpu->Carry);

    // Use DEMUX to direct the result to the appropriate register
    DEMUX(inst.D1, inst.D0, result, cpu);

    // Increment PC if no jump occurred, moving to the next instruction
    cpu->PC += 1;
}

// Function to simulate the program by processing each instruction
void simulate(const unsigned char instructions[], int length, char mode) {
    // Initialize the CPU with RA = 0, RB = 1 (initial values for Fibonacci), RO = 0, PC = 0
    CPU cpu = {0, 1, 0, 0, 0};  
    int instruction_count = 0;  // Counter to track the number of executed instructions

    printf("Starting simulation in %s mode...\n", (mode == 'S') ? "step-by-step" : "continuous");

    // Main loop to process instructions as long as PC is within bounds
    while (cpu.PC < length && (mode == 'S' || instruction_count < 50)) {
        Instruction inst = decode_instruction(instructions[cpu.PC]);  // Decode the current instruction
        execute_instruction(&cpu, inst);  // Execute the decoded instruction

        // Print RO if it was the result of the RA operation (special condition)
        if (inst.D1 == 1 && inst.D0 == 0) {
            printf("RO = %d\n", cpu.RO);  // Only print RO if it’s set by a specific instruction
        }

        if (mode == 'S') {  // Step-by-step mode: pauses for user input between instructions
            printf("[Press Enter to continue]");
            getchar();      // Waits for the user to press Enter
        }

        instruction_count++;  // Increment instruction count after each executed instruction
    }

    // Print final state of RO after simulation ends
    printf("Final State: RO=%d\n", cpu.RO);
}

// Main function that loads instructions from a binary file and starts the simulation
int main(int argc, char *argv[]) {
    if (argc < 2) {  // Check if filename is provided as an argument
        fprintf(stderr, "Usage: %s <binary file>\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "rb");  // Open binary file for reading
    if (!file) {  // Check if file opened successfully
        perror("Error opening file");
        return 1;
    }

    // Load the instructions from the file into an array
    unsigned char instructions[256];
    int length = fread(instructions, sizeof(unsigned char), 256, file);  // Read up to 256 instructions
    fclose(file);  // Close the file after reading

    // Prompt user to select simulation mode (step-by-step or continuous)
    char mode;
    printf("Select mode:\nR - Run in continuous mode\nS - Run step-by-step\nSelect mode: ");
    scanf(" %c", &mode);  // Read the user's choice
    getchar();  // Consume the newline character from user input

    // Start simulation with the loaded instructions and selected mode
    simulate(instructions, length, mode);
    return 0;  // End program
}

