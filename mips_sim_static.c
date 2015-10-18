/********************************************************
 *
 *	note: new order is: if, wb, id, exec1, exec2, mem
 *			to solve problem of register being read
 *			before writing
 *
 *********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int VERBOSE = 0;
int clk = 0;
int PC = 0;

typedef enum{

	TRUE = 1, FALSE = 0
} boolean;

/**********************************************************
 *
 *	Defining the stages
 *
 **********************************************************/

//	enums for stages
typedef enum {

	IF, ID, EXE1, EXE2, MEM, WB

} stage_t;

/**********************************************************
 *
 *	Define the opcode structure
 *
 **********************************************************/

//	enums for registers
typedef enum {

	R0 = 0, R1 = 1, R2 = 2, R3 = 3 , R4 = 4, R5 = 5

} register_t;

//	Enums for the opcode
typedef enum {

	LW, SW, ADD, ADDI, SUB, SUBI, XOR, BLTZ, BNEZ, EOP, STALL

} opcode_t;

//	Structure used to parse instruction
typedef struct {

	opcode_t opcode = NULL;	//opcode
	register_t src1 = NULL;		//first source register
	register_t src2 = NULL;		//second source register
	register_t dest = NULL;		//destination register
	unsigned int immediate = NULL;	//immediate field

} mips_instr_t;

int stages[14]

/**********************************************************
 *
 *	Defining the normal and pipeline registers
 *
 **********************************************************/

struct {

	unsigned reg[ 5 ] = { 0 };	//init all reg values to 0
	
} reg_block;

struct {

	mips_instr_t IR = NULL;
	opcode_t instr = NULL;
	
} pipeline_reg_if_id;

struct {

	unsigned src1;
	unsigned src2;
	unsigned dest;
	mips_instr_t IR = NULL;
	unsigned immediate;
	boolean select_immediate = FALSE;
	opcode_t instr = NULL;

} pipeline_reg_id_exe1;

struct {

	unsigned reg[ 5 ];
	opcode_t instr = NULL;

} pipeline_reg_exe1_exe2;

struct {

	boolean branch_taken = FALSE;
	unsigned branch_addr = 0;
	
	opcode_t instr = NULL;

} pipeline_reg_exe2_mem;

struct {

	unsigned reg[ 5 ];
	opcode_t instr = NULL;

} pipeline_reg_mem_wb;

/**********************************************************
 *
 *	Define the data and instruction memory
 *
 **********************************************************/

//	instruction memory
mips_instr_t * instruction_memory = NULL;
int lineNumber = -1;

// max size of mem is 0xFFF0 --> 65,520
//	4 bytes for an integer, thus 65520/4 = 16380
int * data_memory;
data_memory = (int*) malloc( 16380 * sizeof( int ) );

/**********************************************************
 *
 *	Populate the instruction memory with instructions
 *
 **********************************************************/
void init_instr_memory();

/*----------------------------------------------------------------------------------------------------*/

int main( int argc, char **argv ){

	//	Init the instructions
	init_instr_memory();

	// CLOCK CYCLE LOOP
	int instr_buffer;
	while( 1 ){
	
		//	NOTE: Order matters. All functions should fetch value
		//	from pipelined reg before the previous stage override the 
		//	reg values
		write_back();
		memory_write();
		execute_2();
		execute_1();
		instruction_decode();
		instruction_fetch();
		
		clk++;		
	}
	
	free( instruction_memory );
	free( data_memory );

exit( 0 );
}

/*----------------------------------------------------------------------------------------------------*/

void instruction_fetch(){

	//	React to stall

	//	Pass instruction register to pipelined reg
	mips_instr_t buffer;
	buffer = instruction_memory[ PC ];
	pipeline_reg_if_id.IR = buffer;
	pipeline_reg_if_id.instr = buffer.opcode;

	// Determine next instruction
	if( !pipeline_reg_exe2_mem.branch_taken ){
		PC++;
	} else {
		PC = pipeline_reg_exe2_mem.branch_addr;
		pipeline_reg_exe2_mem.branch_taken = 0;
	}
	
}

/*----------------------------------------------------------------------------------------------------*/

void instruction_decode(){

	//	Moving regs from IR to reg block and pipelined reg
	mips_instr_t buffer;	
	buffer = pipeline_reg_if_id.IR;

	//	Check for stall
	if( buffer.opcode == STALL ){
		pipeline_reg_if_id.instr = STALL;
		pipeline_reg_id_exe1.instr = STALL;
		
		//TODO:	do stall stuff
	}
	
	pipeline_reg_id_exe1.src1 = reg_block[ buffer.src1 ];
	pipeline_reg_id_exe1.src2 = reg_block[ buffer.src2 ];
	pipeline_reg_id_exe1.dest = reg_block[ buffer.dest ];
	
	//	Moving immediate in IR to pipelined reg
	pipeline_reg_id_exe1.immediate = buffer.immediate;
	if( buffer.immediate == NULL ){
		pipeline_reg_id_exe1.select_immediate = FALSE;
	} else {
		pipeline_reg_id_exe1.select_immediate = TRUE;
	}
	
	//	Moving IR to pipelined reg
	pipeline_reg_id_exe1.IR = buffer;
	pipeline_reg_id_exe1.instr = buffer.opcode;	
		
	//	(Sign extended?)
}

void execute_1(){
	
	pipeline_reg_exe1_exe2.instr = pipeline_reg_id_exe1.instr;
	

}

void execute_2(){

	pipeline_reg_exe2_mem.instr = pipeline_reg_exe1_exe2.instr;

}

void memory_write(){

	pipeline_reg_mem_wb.instr = pipeline_reg_exe2_mem.instr;

}

void write_back(){


}

void update_pipeline_reg(){


}

/*----------------------------------------------------------------------------------------------------*/

void init_instr_memory(){

	lineNumber = 14;
	instruction_memory = ( mips_instr_t * ) malloc( lineNumber * sizeof( mips_instr_t ) );
	
	instruction_memory[ 0 ] = { XOR, R0, R0, R0, NULL };
	instruction_memory[ 1 ] = { XOR, R1, R1, R1, NULL };
	instruction_memory[ 2 ] = { ADDI, R1, NULL, R2, 40960 };
	instruction_memory[ 3 ] = { ADDI, R0, NULL, R3, 45056 };
	instruction_memory[ 4 ] = { LW, R1, NULL, R4, 0 };
	instruction_memory[ 5 ] = { BLTZ, R4, NULL, 8, NULL };
	instruction_memory[ 6 ] = { SW, R4, R3, NULL, 0 };
	instruction_memory[ 7 ] = { SUBI, R3, NULL, R3, 4 };
	instruction_memory[ 8 ] = { ADD, R1, R4, R1, NULL };
	instruction_memory[ 9 ] = { ADDI, R2, NULL, R2, 4 };
	instruction_memory[ 10 ] = { SUBI, R2, NULL, R5, 40992 };
	instruction_memory[ 11 ] = { BNEZ, R5, NULL, 4, NULL };
	instruction_memory[ 12 ] = { SW, R1, R2, NULL, 0 };
	instruction_memory[ 13 ] = { SW, R3, R2, NULL, 4 };
	
	//TODO: loop and analyze hazards before manually insert
	//		STALL opcode between codes
}
