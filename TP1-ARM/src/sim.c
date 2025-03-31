#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "shell.h"
#include <stdlib.h> 

#define ADDS_IMM     0b10110001 
#define ADDS_REG     0b10101011000 
#define HALT         0b11010100010 
#define ANDS_REG     0b11101010 
#define ORR_REG      0b10101010
#define B_cond       0b01010100
#define MOVZ         0b11010010100 
#define LSL          0b1101001101 
#define LSR          0b1101001100 
#define CMP_IMM      0b11110000 
#define STUR         0b11111000000 
#define SUBS_IMM     0b11110001 
#define CMP_REG_SUBS_EXT_REG 0b11101011000
#define EOR_REG      0b11001010 
#define BR           0b1101011000011111000000 
#define STURB        0b00111000000 
#define STURH        0b01111000000
#define LDURB        0b00111000010 
#define LDUR32       0b10111000010 
#define LDUR64       0b11111000010 
#define ADD_IMM      0b10010001 
#define ADD_EXT_REG  0b10001011001
#define CBZ          0b10110100
#define MUL          0b10011011000
#define CBNZ         0b10110101

int64_t signextend64(int32_t value, int bit_count) {
    int64_t mask = (int64_t)1 << (bit_count - 1);
    return (value ^ mask) - mask; 
}

void adds_imm(uint32_t instruction);
void adds_reg(uint32_t instruction);
void ands_reg(uint32_t instruction);
void orr_reg(uint32_t instruction);
void b_cond(uint32_t instruction);
void movz(uint32_t instruction);
void subs_imm(uint32_t instruction);
void subs_ext_reg(uint32_t instruction);
void eor_reg(uint32_t instruction);
void br(uint32_t instruction);
void lsr_imm(uint32_t instruction);
void lsl_imm(uint32_t instruction);
void sturb(uint32_t instruction);
void sturh(uint32_t instruction);
void ldurb(uint32_t instruction);
void add_ext_reg(uint32_t instruction);
void add_imm(uint32_t instruction);
void cbz(uint32_t instruction);
void cmp_reg(uint32_t instruction);
void stur(uint32_t instruction);
void cmp_imm(uint32_t instruction);
void mul(uint32_t instruction);
void cbnz(uint32_t instruction);
void ldur_32(uint32_t instruction);
void ldur_64(uint32_t instruction);

void process_instruction() {
    
    uint32_t instruction = mem_read_32(CURRENT_STATE.PC);
    NEXT_STATE = CURRENT_STATE;

    uint32_t opcode24 = (instruction >> 24) & 0xFF;
    uint32_t opcode21 = (instruction >> 21) & 0x7FF;  
    uint32_t opcode22 = (instruction >> 22) & 0x3FF;

    switch (opcode21) {
        case ADDS_REG: {
            adds_reg(instruction);
            NEXT_STATE.PC += 4;
            break;
        }
        case MOVZ: {
            movz(instruction);
            NEXT_STATE.PC += 4;
            break;
        }
        case CMP_REG_SUBS_EXT_REG: {
            cmp_reg(instruction);
            subs_ext_reg(instruction);
            NEXT_STATE.PC += 4;
            break;
        }
        case STUR: {
            uint32_t rt = (instruction >> 0) & 0b11111;     
            uint32_t rn = (instruction >> 5) & 0b11111;      
            uint32_t imm9 = (instruction >> 12) & 0b111111111;
            uint64_t address = CURRENT_STATE.REGS[rn] + imm9;
            uint64_t data = CURRENT_STATE.REGS[rt];
            if (address < 0x10000000) {
                break;}
            mem_write_32(address, data);
            NEXT_STATE.PC += 4;
            break;
        }
        case STURB: {        
            uint32_t rt   = (instruction >> 0) & 0x1F;      
            uint32_t rn   = (instruction >> 5) & 0x1F;      
            uint32_t imm9 = (instruction >> 12) & 0x1FF;   
        
            int64_t offset = 0;
            if (imm9 & 0x100) { 
                offset = imm9 | 0xFFFFFFFFFFFFFE00ULL; 
            } else {
                offset = imm9; 
            }
        
            uint64_t address = CURRENT_STATE.REGS[rn] + offset;
        
            uint8_t data_byte = (uint8_t)(CURRENT_STATE.REGS[rt] & 0xFF);
        
        
            mem_write_32(address, data_byte);
        
            NEXT_STATE.PC += 4;
            break;
        }
        case STURH: {
            uint32_t rt = (instruction >> 0) & 0b11111;      
            uint32_t rn = (instruction >> 5) & 0b11111;      
            uint32_t imm9 = (instruction >> 12) & 0b111111111;
            uint64_t address = CURRENT_STATE.REGS[rn] + imm9;
            uint64_t data = CURRENT_STATE.REGS[rt];

            mem_write_32(address, data);
            NEXT_STATE.PC += 4;
            break;
        }
        case LDUR32: {
            ldur_32(instruction);
            NEXT_STATE.PC += 4;
            break;
        }
        case LDUR64: {
            ldur_64(instruction);
            NEXT_STATE.PC += 4;
            break;
        }
        case LDURB: {
            ldurb(instruction);
            NEXT_STATE.PC += 4;
            break;
        }
        case ADD_EXT_REG: {
            add_ext_reg(instruction);
            NEXT_STATE.PC += 4;
            break;
        }
        case MUL: {
            mul(instruction);
            NEXT_STATE.PC += 4;
            break;
        }
        case HALT:
            RUN_BIT = 0;
            NEXT_STATE.PC += 4;
            return;
    }
    switch (opcode22)
        {
        case LSL : { //lsl X4, X3, 4 (descripción: Logical left shift (X4 = X3 << 4 ))
            lsl_imm(instruction);
            NEXT_STATE.PC += 4;
            break; 
        }
        case LSR : {
            lsr_imm(instruction);
            NEXT_STATE.PC += 4;
            break;
        }}
    switch (opcode24) {
        case ADDS_IMM: {
            adds_imm(instruction);
            NEXT_STATE.PC += 4;
            break;
        }
        case ANDS_REG: {
            ands_reg(instruction);
            NEXT_STATE.PC += 4;
            break;
        }
        case ORR_REG:{
            orr_reg(instruction);
            NEXT_STATE.PC += 4;
            break;
        }
        case SUBS_IMM:{
            subs_imm(instruction);
            NEXT_STATE.PC += 4;
            break;
        }
        case EOR_REG:{
            eor_reg(instruction);
            NEXT_STATE.PC += 4;
            break;
        }
        case ADD_IMM:{
            add_imm(instruction);
            NEXT_STATE.PC += 4;    
            break;
        }
        case CBZ:{
            cbz(instruction);
            NEXT_STATE.PC += 4;
            break;
        }
        case CMP_IMM:{
            cmp_imm(instruction);
            NEXT_STATE.PC += 4;
            break;
        }
        case CBNZ:{
            cbnz(instruction);
            break;
        }
        case B_cond:{
            b_cond(instruction);
            break;}
}
}

void adds_imm(uint32_t instruction) {
        uint32_t rn   = (instruction >> 5)  & 0b11111;
        uint32_t rd   = (instruction >> 0)  & 0b11111;
        uint32_t imm12 = (instruction >> 10) & 0xFFF;
        uint32_t shift = (instruction >> 22) & 0b11;

        uint64_t imm = (shift == 0b01) ? ((uint64_t)imm12 << 12) : (uint64_t)imm12;

        NEXT_STATE.REGS[rd] = NEXT_STATE.REGS[rn] + imm;

        NEXT_STATE.FLAG_Z = (NEXT_STATE.REGS[rd] == 0);
        NEXT_STATE.FLAG_N = (NEXT_STATE.REGS[rd] >> 63) & 1;}

void adds_reg(uint32_t instruction) {
        uint32_t rm  = (instruction >> 16) & 0b11111;
        uint32_t rn  = (instruction >> 5)  & 0b11111;
        uint32_t rd  = (instruction >> 0)  & 0b11111;

        uint64_t result = NEXT_STATE.REGS[rn] + NEXT_STATE.REGS[rm];
        NEXT_STATE.REGS[rd] = result;
        NEXT_STATE.FLAG_Z = (result == 0);
        NEXT_STATE.FLAG_N = (result >> 63) & 1;
        }

void orr_reg(uint32_t instruction) {
        uint32_t rm  = (instruction >> 16) & 0b11111;
        uint32_t rn  = (instruction >> 5)  & 0b11111;
        uint32_t rd  = (instruction >> 0)  & 0b11111;
        uint32_t inm6 = (instruction >> 10) & 0b111111;


        uint64_t result = NEXT_STATE.REGS[rn] | NEXT_STATE.REGS[rm];
        NEXT_STATE.REGS[rd] = result;
        NEXT_STATE.FLAG_Z = (result == 0);
        NEXT_STATE.FLAG_N = (result >> 63) & 1;}

void ands_reg(uint32_t instruction) {
        uint32_t rm  = (instruction >> 16) & 0b11111;
        uint32_t rn  = (instruction >> 5)  & 0b11111;
        uint32_t rd  = (instruction >> 0)  & 0b11111;

        uint64_t result = NEXT_STATE.REGS[rn] & NEXT_STATE.REGS[rm];
        NEXT_STATE.REGS[rd] = result;
        NEXT_STATE.FLAG_Z = (result == 0);
        NEXT_STATE.FLAG_N = (result >> 63) & 1;}

void movz(uint32_t instruction) {
            
            uint32_t imm16 = (instruction >> 5) & 0xFFFF;  
            uint32_t rd    = (instruction >> 0) & 0x1F;    
            uint32_t shift = (instruction >> 21) & 0x3;           
            uint32_t result = 0;

            result = imm16;
                
            NEXT_STATE.REGS[rd]= result;}

void b_cond(uint32_t instruction) {
    uint32_t cond = (instruction >> 0) & 0b1111;
    uint32_t imm19 = (instruction >> 5) & 0b1111111111111111111;


    switch (cond) {

        case 0b0000:  
            if (CURRENT_STATE.FLAG_Z) {
                int32_t imm19 = (instruction >> 5) & 0x7FFFF;
                int64_t offset = signextend64(imm19, 19) << 2;
                NEXT_STATE.PC = CURRENT_STATE.PC + offset;     
            }
            else{
                NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            }
            break;
        case 0b0001:  
            if (!CURRENT_STATE.FLAG_Z) { 
                int32_t imm19 = (instruction >> 5) & 0x7FFFF;
                int64_t offset = signextend64(imm19, 19) << 2;
                
                NEXT_STATE.PC = CURRENT_STATE.PC + offset;    
            }
            else{
                NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            }
            break;
        case 0b1100:  
            if (CURRENT_STATE.FLAG_Z==0 && CURRENT_STATE.FLAG_N==0) { 
                int32_t imm19 = (instruction >> 5) & 0x7FFFF;
                int64_t offset = signextend64(imm19, 19) << 2;
                NEXT_STATE.PC = CURRENT_STATE.PC + offset;  
            }
            else{
                NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            }
            break;
        case 0b1011:  
            if (CURRENT_STATE.FLAG_N) { 
                int32_t imm19 = (instruction >> 5) & 0x7FFFF;
                int64_t offset = signextend64(imm19, 19) << 2;
                
                NEXT_STATE.PC = CURRENT_STATE.PC + offset;    
            }
            else{
                NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            }
            break;
        case 0b1010:  
            if (CURRENT_STATE.FLAG_N==0) { 
                int32_t imm19 = (instruction >> 5) & 0x7FFFF;
                int64_t offset = signextend64(imm19, 19) << 2;
                
                NEXT_STATE.PC = CURRENT_STATE.PC + offset;    
            }
            else{
                NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            }
            break;
        case 0b1101: 
            if ( ! (CURRENT_STATE.FLAG_Z==0 && CURRENT_STATE.FLAG_N==0)) { 
                int32_t imm19 = (instruction >> 5) & 0x7FFFF;
                int64_t offset = signextend64(imm19, 19) << 2;
                
                NEXT_STATE.PC = CURRENT_STATE.PC + offset;    
            }
            else{
                NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            }
            break;
        default:
            break;
            
    break;
    }
}

void subs_imm(uint32_t instruction){
        uint32_t rn = (instruction >> 5) & 0b11111;      
        uint32_t rd = (instruction >> 0) & 0b11111;
        uint32_t imm12 = (instruction >> 10) & 0b111111111111;
        uint32_t shift = (instruction >> 22) & 0b11;
        uint64_t imm = imm12; 
        if (shift == 0b00){
            imm = (uint64_t)imm12;
        } else if (shift == 0b01){
            imm = (uint64_t)imm12 << 12;
        }
        uint64_t result;
        uint64_t operand1 = NEXT_STATE.REGS[rn];
        uint64_t operand2= imm;
        result = operand1 - operand2;
        NEXT_STATE.REGS[rd] = result;
        NEXT_STATE.FLAG_Z = (result == 0);
        NEXT_STATE.FLAG_N = (result >> 63) & 1;}

void lsr_imm(uint32_t instruction){
        uint32_t rn = (instruction >> 5) & 0b11111;   
        uint32_t inms = (instruction >> 10) & 0b111111;                  
        uint32_t rd = (instruction >> 0) & 0b11111;
        uint32_t shift = (instruction >> 16) & 0x3F; 
        uint64_t operand2;

        if (inms ==0b111111) {
            operand2 = CURRENT_STATE.REGS[rn] >> shift;
        }
        else if (inms != 0b111111) {
            operand2 = CURRENT_STATE.REGS[rn] <<64- shift;
        }
        NEXT_STATE.REGS[rd] = operand2;
}

void lsl_imm(uint32_t instruction){

        uint32_t rn = (instruction >> 5) & 0b11111;   
        uint32_t inms = (instruction >> 10) & 0b111111;                  
        uint32_t rd = (instruction >> 0) & 0b11111;
        uint32_t shift = (instruction >> 16) & 0x3F; 
        uint64_t operand2;

        if (inms ==0b111111) {
            operand2 = CURRENT_STATE.REGS[rn] >> shift;
        }
        else if (inms != 0b111111) {
            operand2 = CURRENT_STATE.REGS[rn] <<64- shift;
        }
        NEXT_STATE.REGS[rd] = operand2;
}

void subs_ext_reg(uint32_t instruction){
    uint32_t rm = (instruction >> 16) & 0b11111;      
    uint32_t rn = (instruction >> 5)  & 0b11111;      
    uint32_t rd = (instruction >> 0)  & 0b11111;
    uint32_t shift = (instruction >> 22) & 0b11;
    
    uint64_t operand2 = NEXT_STATE.REGS[rm];

    if (shift ==0b01){
        operand2 <<= 12;
    }
    operand2 = ~operand2;
    uint64_t result = NEXT_STATE.REGS[rn] + operand2 + 1;
    NEXT_STATE.REGS[rd] = result;
    
    NEXT_STATE.FLAG_Z = (result == 0);
    NEXT_STATE.FLAG_N = (result >> 63) & 1;
}

void eor_reg(uint32_t instruction){
        uint32_t rm = (instruction >> 16) & 0b11111;      
        uint32_t rn = (instruction >> 5) & 0b11111; 
        uint32_t imm6 = (instruction >> 10) & 0b111111;     
        uint32_t rd = (instruction >> 0) & 0b11111;
        uint64_t result;
        uint64_t operand1 = NEXT_STATE.REGS[rn];
        uint64_t operand2 = NEXT_STATE.REGS[rm];
        result= operand1 ^ operand2;
        NEXT_STATE.REGS[rd] = result;
}

void br(uint32_t instruction){
        uint32_t imm26 = (instruction >> 0) & 0b11111111111111111111111111;
        uint64_t result;
        uint64_t operand1 = NEXT_STATE.PC;
        result= operand1 + imm26;
        NEXT_STATE.PC = result;
}

void cmp_reg(uint32_t instruction){
        uint32_t rm = (instruction >> 16) & 0b11111;      
        uint32_t rn = (instruction >> 5) & 0b11111;      
        uint32_t rd = (instruction >> 0) & 0b11111;
        uint32_t imm3 = (instruction >> 10) & 0b111;
        uint32_t option = (instruction >> 13) & 0b111;
        uint64_t result;
        uint64_t operand1 = NEXT_STATE.REGS[rn];
        uint64_t operand2 = NEXT_STATE.REGS[rm];
        result= operand1 - operand2;
        NEXT_STATE.FLAG_Z = (result == 0);
        NEXT_STATE.FLAG_N = (result >> 63) & 1;
}

void cmp_imm(uint32_t instruction){
        uint32_t rn = (instruction >> 5) & 0b11111;      
        uint32_t rd = (instruction >> 0) & 0b11111;
        uint32_t imm12 = (instruction >> 10) & 0b111111111111;
        uint32_t shift = (instruction >> 22) & 0b11;
        uint64_t imm = imm12; 
        if (shift == 0b00){
            imm = (uint64_t)imm12;
        } else if (shift == 0b01){
            imm = (uint64_t)imm12 << 12;
        }
        uint64_t result;
        uint64_t operand1 = NEXT_STATE.REGS[rn];
        uint64_t operand2= ~imm;
        result = operand1 + operand2;
        NEXT_STATE.FLAG_Z = (result == 0);
        NEXT_STATE.FLAG_N = (result >> 63) & 1;
}

void add_ext_reg(uint32_t instruction){
        uint32_t rm = (instruction >> 16) & 0b11111;      
        uint32_t rn = (instruction >> 5) & 0b11111; 
        uint32_t rd = (instruction >> 0) & 0b11111;
        uint32_t imm3 = (instruction >> 10) & 0b111;
        uint32_t option = (instruction >> 13) & 0b111;
        uint64_t result;
        uint64_t operand1 = NEXT_STATE.REGS[rn];
        uint64_t operand2 = NEXT_STATE.REGS[rm];
        result= operand1 + operand2;
        NEXT_STATE.REGS[rd] = result;
}

void add_imm(uint32_t instruction){
        uint32_t rn = (instruction >> 5) & 0b11111;      
        uint32_t rd = (instruction >> 0) & 0b11111;
        uint32_t imm12 = (instruction >> 10) & 0b111111111111;
        uint32_t shift = (instruction >> 22) & 0b11;
        uint64_t imm = imm12;
        if (shift == 0b00){
            imm = (uint64_t)imm12;
        } else if (shift == 0b01){
            imm = (uint64_t)imm12 << 12;
        }
        uint64_t result;
        uint64_t operand1 = NEXT_STATE.REGS[rn];
        result = operand1 + imm;
        NEXT_STATE.REGS[rd] = result;
}

void cbz(uint32_t instruction){
        uint32_t rt = (instruction >> 0) & 0b11111;      
        uint32_t imm19 = (instruction >> 5) & 0b1111111111111111111;    
        uint64_t result;
        uint64_t operand1 = NEXT_STATE.REGS[rt];
        if (operand1 == 0){
            result= operand1 + imm19;
            NEXT_STATE.PC = result;
        }
}

void ldur_64(uint32_t instruction) {
    uint32_t rt   = (instruction >> 0) & 0x1F;   
    uint32_t rn   = (instruction >> 5) & 0x1F;   
    uint32_t imm9 = (instruction >> 12) & 0x1FF;  

    int64_t offset = signextend64(imm9,9);
    uint64_t address = CURRENT_STATE.REGS[rn] + offset;

    uint32_t low  = mem_read_32(address);
    uint32_t high = mem_read_32(address + 4);
    uint64_t data = ((uint64_t) high << 32) | low;
    NEXT_STATE.REGS[rt] = data;
}

void ldur_32(uint32_t instruction) {
    uint32_t rt   = (instruction >> 0) & 0x1F;   
    uint32_t rn   = (instruction >> 5) & 0x1F;   
    uint32_t imm9 = (instruction >> 12) & 0x1FF;  

    int64_t offset = signextend64(imm9,9);
    uint64_t address = CURRENT_STATE.REGS[rn] + offset;

    uint64_t data = mem_read_32(address);
    NEXT_STATE.REGS[rt] = data;
}

void ldurb(uint32_t instruction) {
    uint32_t rt   = instruction & 0x1F;           
    uint32_t rn   = (instruction >> 5) & 0x1F;       
    uint32_t imm9 = (instruction >> 12) & 0x1FF;      

    int64_t offset = signextend64(imm9,9);
    uint64_t address = CURRENT_STATE.REGS[rn] + offset;

    uint32_t word = mem_read_32(address);
    uint8_t data = (uint8_t)(word & 0xFF);
    NEXT_STATE.REGS[rt] = (uint64_t)data;
}

void ldurh(uint32_t instruction) {
    uint32_t rt   = (instruction >> 0) & 0x1F;  
    uint32_t rn   = (instruction >> 5) & 0x1F;   
    uint32_t imm9 = (instruction >> 12) & 0x1FF; 

    int64_t offset = signextend64(imm9,9);
    uint64_t address = CURRENT_STATE.REGS[rn] + offset;

    uint16_t data = mem_read_32(address) & 0xFFFF;
    NEXT_STATE.REGS[rt] = data;
}

void mul(uint32_t instruction) {

    uint32_t rm = (instruction >> 16) & 0x1F; 
    uint32_t rn = (instruction >> 5)  & 0x1F;
    uint32_t rd = instruction         & 0x1F;
    uint64_t op1 = NEXT_STATE.REGS[rn];
    uint64_t op2 = NEXT_STATE.REGS[rm];
    uint64_t result = op1 * op2;

    NEXT_STATE.REGS[rd] = result;
}

void cbnz(uint32_t instruction) {

    uint32_t rt = instruction & 0x1F;

    uint32_t imm19 = (instruction >> 5) & 0x7FFFF;

    int64_t offset = signextend64(imm19,19) << 2;

    if (CURRENT_STATE.REGS[rt] != 0) {
        NEXT_STATE.PC = CURRENT_STATE.PC + offset;
    } else {
        NEXT_STATE.PC += 4;
    }
}
