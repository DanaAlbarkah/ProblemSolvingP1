#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int RA, RB, RO, PC, Carry;
} CPU;

typedef struct {
    int J, C, D1, D0, Sreg, S, imm;
} Instruction;

// Decode an 8-bit instruction into its components
Instruction decode_instruction(unsigned char byte) {
    Instruction inst;
    inst.J = (byte & 0x80) >> 7;
    inst.C = (byte & 0x40) >> 6;
    inst.D1 = (byte & 0x20) >> 5;
    inst.D0 = (byte & 0x10) >> 4;
    inst.Sreg = (byte & 0x08) >> 3;
    inst.S = (byte & 0x04) >> 2;
    inst.imm = byte & 0x07;
    return inst;
}

// ALU function for ADD/SUB with carry
int ALU(int op, int a, int b, int *carry) {
    int result = 0;
    *carry = 0;
    if (op == 0) {  // ADD operation
        result = a + b;
        if (result > 15) {  // Check for 4-bit overflow
            *carry = 1;
            result &= 0xF;  // Limit to 4 bits
        }
    } else {  // SUB operation
        result = a - b;
        if (result < 0) {  // Check for 4-bit underflow
            *carry = 1;
            result = (result + 16) & 0xF;  // Handle underflow
        }
    }
    return result;
}

// Update registers based on the D1 and D0 fields, with RO reset and sequence restart when RO > 13
void DEMUX(int D1, int D0, int value, CPU *cpu) {
    if (D1 == 0 && D0 == 0) {
        cpu->RA = value;
    } else if (D1 == 0 && D0 == 1) {
        cpu->RB = value;
    } else if (D1 == 1 && D0 == 0) {
        cpu->RO = value;
        if (cpu->RO > 13) {  // Reset RO, RA, and RB when RO exceeds 13
            printf("RO exceeded 13, resetting RO, RA, and RB to restart Fibonacci sequence.\n");
            cpu->RO = 0;
            cpu->RA = 0;
            cpu->RB = 1;  // Set initial values for Fibonacci restart
        }
    }
}

// Execute a decoded instruction, supporting generic operations
void execute_instruction(CPU *cpu, Instruction inst) {
    int result;

    // Handle jump instructions
    if (inst.J == 1) {
        cpu->PC = inst.imm;
        return;
    } else if (inst.C == 1 && cpu->Carry) {
        cpu->PC = inst.imm;
        return;
    }

    // Check if the operation uses immediate or register values
    int B = (inst.Sreg) ? inst.imm : cpu->RB;
    result = ALU(inst.S, cpu->RA, B, &cpu->Carry);

    // Update the correct register based on D1 and D0 fields
    DEMUX(inst.D1, inst.D0, result, cpu);

    // Increment PC if no jump occurred
    cpu->PC += 1;
}

// Simulation function for running the program
void simulate(const unsigned char instructions[], int length, char mode) {
    CPU cpu = {0, 1, 0, 0, 0};  // Initialize CPU state
    int instruction_count = 0;  // Counter for executed instructions

    printf("Starting simulation in %s mode...\n", (mode == 'S') ? "step-by-step" : "continuous");

    while (cpu.PC < length && (mode == 'S' || instruction_count < 50)) {
        Instruction inst = decode_instruction(instructions[cpu.PC]);
        execute_instruction(&cpu, inst);

        // Print RO only when RO = RA instruction is executed
        if (inst.D1 == 1 && inst.D0 == 0) {
            printf("RO = %d\n", cpu.RO);
        }

        if (mode == 'S') {  // Step-by-step mode
            printf("[Press Enter to continue]");
            getchar();
        }

        instruction_count++;  // Increment the instruction counter
    }

    printf("Final State: RO=%d\n", cpu.RO);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <binary file>\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "rb");
    if (!file) {
        perror("Error opening file");
        return 1;
    }

    // Load the instructions from the binary file
    unsigned char instructions[256];
    int length = fread(instructions, sizeof(unsigned char), 256, file);
    fclose(file);

    char mode;
    printf("Select mode:\nR - Run in continuous mode\nS - Run step-by-step\nSelect mode: ");
    scanf(" %c", &mode);
    getchar();  // Consume newline after mode selection

    simulate(instructions, length, mode);
    return 0;
}
 
