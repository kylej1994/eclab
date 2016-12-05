/*
 * CMSC 22200
 *
 * ARM pipeline timing simulator
 *
 * Reza Jokar and Gushu Li, 2016
 */
 
//Kyle Jablon (jablonk), Wesley Kelly (wesleyk) and Kwaku Ofori-Atta (kwaku)

#include "pipe.h"
#include "shell.h"
#include "bp.h"
#include "cache.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

int ERET_VERBOSE_FLAG = false;

void pipe_init()
{
    current_CPU = 0;
    cpu_0_active = true;
    cpu_1_active = false;
    cpu_2_active = false;
    cpu_3_active = false;

    

    int i,j;
    for (i = 0; i < 4; i++)
    {
    RUN_BIT_ARRAY[i] = true;
        
    memset(&CURRENT_STATE[i], 0, sizeof(CPU_State));
    memset(&C_FETCH[i], 0, sizeof(CYCLE_FETCH));
    memset(&C_DECODE[i], 0, sizeof(CYCLE_DECODE));
    memset(&C_EXECUTE[i], 0, sizeof(CYCLE_EXECUTE));
    memset(&C_MEMORY[i], 0, sizeof(CYCLE_MEMORY));
    memset(&C_WRITE[i], 0, sizeof(CYCLE_WRITE));
    
    //initialize BPT
    memset(&BPT[i], 0, sizeof(bp_t));
    memset(&(BPT[i].gshare), 0, sizeof(gshare_t));

    for (j = 0; j < BTB_ENTRY; j++){
        memset(&(BPT[i].btb[j]), 0, sizeof(btb_entry_t));
    }
    
    cache_new();

    C_FETCH[i].instr = -1;
    C_FETCH[i].pc = 0;
    C_FETCH[i].stall_bit = false;
    
    C_DECODE[i].instr = -1;
    C_DECODE[i].oppCode = OPP_MACRO_UNASS;
    C_DECODE[i].pc = 0;
    C_DECODE[i].stall_bit = true;
    C_DECODE[i].run_bit = 0;
    C_DECODE[i].retired = 0;
    
    C_EXECUTE[i].instr = -1;
    C_EXECUTE[i].oppCode = OPP_MACRO_UNASS;
    C_EXECUTE[i].result = 0;
    C_EXECUTE[i].stall_bit = false;
    C_EXECUTE[i].pc = 0;
    C_EXECUTE[i].run_bit = true;
    C_EXECUTE[i].branch_stall_bit = false;
    
    C_MEMORY[i].instr = -1;
    C_MEMORY[i].oppCode = OPP_MACRO_UNASS;
    C_MEMORY[i].pc = 0;
    C_MEMORY[i].stall_bit = false;
    C_MEMORY[i].result = 0;
    C_MEMORY[i].run_bit = true;
    
    C_WRITE[i].instr = OPP_MACRO_UNASS;
    C_WRITE[i].write_bit = false;
    C_WRITE[i].stall_bit = false;
    CURRENT_STATE[i].PC = 0x00400000;
    }
    
    STALL_FOR_CYCLES[current_CPU] = 0;
    VERBOSE_FLAG = false; //<----SET THIS TO FALSE WHEN YOU TURN IN ASSIGNMENTS
    // printf("PC initialized to  %08x\n",  CURRENT_STATE[current_CPU].PC);
}

    void pipe_cycle()
    {
        //~ if (CURRENT_STATE[current_CPU].REGS[30] == 1) {
            //~ printf("CURRENT_STATE[current_CPU].REGS[30] is 1... just fyi\n");
            //~ printf("and the current cycle is %d\n", stat_cycles + 1);
        //~ }
        //~ if (CURRENT_STATE[current_CPU].REGS[30] == 2) {
            //~ printf("CURRENT_STATE[current_CPU].REGS[30] is 2... just fyi\n");
            //~ printf("and the current cycle is %d\n", stat_cycles + 1);
        //~ }
        //~ if (CURRENT_STATE[current_CPU].REGS[30] == 3) {
            //~ printf("CURRENT_STATE[current_CPU].REGS[30] is 3... just fyi\n");
            //~ printf("and the current cycle is %d\n", stat_cycles + 1);
        //~ }
        
        if (ERET_VERBOSE_FLAG) {
            printf("----------------------------------------------------------------\n");
            printf("current cyle: %d\n", stat_cycles + 1);
        }
            
    uint8_t cpu_0_active_at_pipe_cycle_start = cpu_0_active;
    uint8_t cpu_1_active_at_pipe_cycle_start = cpu_1_active;
    uint8_t cpu_2_active_at_pipe_cycle_start = cpu_2_active;
    uint8_t cpu_3_active_at_pipe_cycle_start = cpu_3_active;
    
        if (cpu_0_active_at_pipe_cycle_start)
        {
            //~ printf("pipe_cycle: CPU 0 RUNNING\n");
            current_CPU = 0;
            pipe_stage_wb();
            pipe_stage_mem();
            pipe_stage_execute();
            pipe_stage_decode();
            pipe_stage_fetch();
        }
        if (cpu_1_active_at_pipe_cycle_start)
        {
            //~ printf("pipe_cycle: CPU 1 RUNNING\n");
            current_CPU = 1;
            pipe_stage_wb();
            pipe_stage_mem();
            pipe_stage_execute();
            pipe_stage_decode();
            pipe_stage_fetch();
        }
        if (cpu_2_active_at_pipe_cycle_start)
        {
            //~ printf("pipe_cycle: CPU 2 RUNNING\n");
            current_CPU = 2;
            pipe_stage_wb();
            pipe_stage_mem();
            pipe_stage_execute();
            pipe_stage_decode();
            pipe_stage_fetch();
        }
        if (cpu_3_active_at_pipe_cycle_start)
        {
            //~ printf("pipe_cycle: CPU 3 RUNNING\n");
            current_CPU = 3;
            pipe_stage_wb();
            pipe_stage_mem();
            pipe_stage_execute();
            pipe_stage_decode();
            pipe_stage_fetch();
        }

        if (ERET_VERBOSE_FLAG){
        int i;
        for (i = 0; i < 4; i++) {
            
            printf("CPU %d:\nPC for this CPU: 0x%" PRIx64 "\n", i, CURRENT_STATE[i].PC);
            printf("fetch\tdecode\texec\tmem\twrite\n");
            printf("%c\t%d\t%d\t%d\t%d\n", '?', C_DECODE[i].oppCode, C_EXECUTE[i].oppCode, C_MEMORY[i].oppCode, C_WRITE[i].oppCode);
            
        }
        printf("===============================================================\n");
        }
        
        
        
    }
    
/*******************************************************************
                            WRITE
*******************************************************************/

//Determine if function writes to a register
//If it does then access the destination register, and write result to that
//Increment the PC counter
void pipe_stage_wb()
{
    if (!RUN_BIT_ARRAY[current_CPU])
        return;
    if (C_WRITE[current_CPU].stall_bit)
        return;


    uint32_t currOp = C_MEMORY[current_CPU].oppCode;
    
    //~ printf("pipe_stage_wb: C_WRITE[current_CPU].oppCode: %d\n", C_WRITE[current_CPU].oppCode);
    C_WRITE[current_CPU].oppCode = C_MEMORY[current_CPU].oppCode;
    C_WRITE[current_CPU].pc = C_MEMORY[current_CPU].pc;
    //~ printf("pipe_stage_wb: C_WRITE[current_CPU].oppCode: %d\n", C_WRITE[current_CPU].oppCode);
    
    C_WRITE[current_CPU].retired = C_MEMORY[current_CPU].retired;
    C_WRITE[current_CPU].instr = C_MEMORY[current_CPU].instr; // added this line 11/26/16, not sure if there was a reason this wasn't here before?

    int64_t result = -1; //Initialize result to negative 1, so zero flag is not automatically set

    if (is_writeable(currOp)){
        int destReg = C_MEMORY[current_CPU].resultRegister;
        result = C_MEMORY[current_CPU].result;
        CURRENT_STATE[current_CPU].REGS[destReg] = result;
        C_WRITE[current_CPU].write_bit = true;
        C_WRITE[current_CPU].result = C_MEMORY[current_CPU].result;
        C_WRITE[current_CPU].resultRegister = C_MEMORY[current_CPU].resultRegister;
        if (VERBOSE_FLAG)
            printf("*****value: %08x into reg %d\n", (uint32_t)result, destReg);
    }
    if (is_flaggable(currOp)){
        //Set flags equal to flags for execute
        CURRENT_STATE[current_CPU].FLAG_Z = C_MEMORY[current_CPU].FLAG_Z;
        CURRENT_STATE[current_CPU].FLAG_N = C_MEMORY[current_CPU].FLAG_N;
        CURRENT_STATE[current_CPU].FLAG_C = C_MEMORY[current_CPU].FLAG_C;
        CURRENT_STATE[current_CPU].FLAG_V = C_MEMORY[current_CPU].FLAG_V;
    }
    /* global run is set to false and we end simulation
    because HLT is in WB every instruction before it is completed
    */
    result = C_MEMORY[current_CPU].result;
    if (C_WRITE[current_CPU].oppCode != OPP_MACRO_UNK)
        stat_inst_retire += C_WRITE[current_CPU].retired;
    /*Unstall any pipelines if applicable*/
    unset_bits();
    if (C_WRITE[current_CPU].bubble_bit){
        C_WRITE[current_CPU].oppCode = OPP_MACRO_UNK;
        return;
    }
    
    //~ printf("pipe_stage_wb: here's C_FETCH[current_CPU].instr: %" PRId64 "\n", C_FETCH[current_CPU].instr);
    //~ printf("pipe_stage_wb: here's C_DECODE[current_CPU].instr: %" PRId64 "\n", C_DECODE[current_CPU].instr);
    //~ printf("pipe_stage_wb: here's C_EXECUTE[current_CPU].instr: %" PRId64 "\n", C_EXECUTE[current_CPU].instr);
    //~ printf("pipe_stage_wb: here's C_MEMORY[current_CPU].instr: %" PRId64 "\n", C_MEMORY[current_CPU].instr);
    //~ printf("pipe_stage_wb: here's C_WRITE[current_CPU].instr, which is being compared to ERET: %" PRId64 "\n", C_WRITE[current_CPU].instr);
    //~ printf("pipe_stage_wb: here's current_CPU: %d\n", current_CPU);
    if (C_WRITE[current_CPU].oppCode == OPP_MACRO_ERET)
    {
            //~ printf("eret in writeback stage\ncurrent_CPU is: %d\n", current_CPU);
            //~ printf("cycle: %d\n", stat_cycles + 1);
            //~ printf("X29: %" PRId64 "\n", CURRENT_STATE[current_CPU].REGS[29]);
            //~ printf("X30: %" PRId64 "\n", CURRENT_STATE[current_CPU].REGS[30]);
        if (ERET_VERBOSE_FLAG) {
            
            
        }
        //~ printf("I found at ERET!\n");
        switch(CURRENT_STATE[current_CPU].REGS[30]) // switch determines which CPU is being started
        {
            case 1:
            cpu_1_active = true;
            CURRENT_STATE[1].PC = C_WRITE[current_CPU].pc + 4;
            CURRENT_STATE[1].REGS[29] = 1;
            CURRENT_STATE[current_CPU].REGS[29] = 0;
            C_DECODE[1].run_bit= true;
            C_EXECUTE[1].run_bit = true;
            C_MEMORY[1].run_bit = true;
            C_WRITE[1].run_bit = true;
            

            break;
            case 2:
            cpu_2_active = true;
            CURRENT_STATE[2].PC = C_WRITE[current_CPU].pc + 4;
            CURRENT_STATE[2].REGS[29] = 1;
            CURRENT_STATE[current_CPU].REGS[29] = 0;
            C_DECODE[2].run_bit= true;
            C_EXECUTE[2].run_bit = true;
            C_MEMORY[2].run_bit = true;
            C_WRITE[2].run_bit = true;

            break;
            case 3:
            cpu_3_active = true;
            CURRENT_STATE[3].PC = C_WRITE[current_CPU].pc + 4;
            CURRENT_STATE[3].REGS[29] = 1;
            CURRENT_STATE[current_CPU].REGS[29] = 0;
            C_DECODE[3].run_bit= true;
            C_EXECUTE[3].run_bit = true;
            C_MEMORY[3].run_bit = true;
            C_WRITE[3].run_bit = true;

            break;
            case 11: // special console output system call
            printf("OUT (CPU %d): 0x%" PRIx64 "\n", current_CPU, CURRENT_STATE[current_CPU].REGS[29]);
            break;
            
            default:
            if (ERET_VERBOSE_FLAG) printf("ERROR: ERET instruction in writeback stage, X30 register not 1, 2, 3, or 11\n");
        }
        CURRENT_STATE[current_CPU].FLAG_Z = 0;
        CURRENT_STATE[current_CPU].FLAG_N = 0;
    }
    if (C_WRITE[current_CPU].oppCode == OPP_MACRO_ERET) {
        /* do not change the placement of this if statement or the one below it because
         * they differentiate between the case when ERET has left the pipeline and the one where it hasn't yet */
         if (VERBOSE_FLAG) printf("eret is in write\n");
            C_EXECUTE[current_CPU].oppCode = OPP_MACRO_UNK;
            set_stall(PL_ERET_STALL);
            //C_WRITE[current_CPU].bounce_bit = true;
            return;
    }
    if (C_WRITE[current_CPU].bounce_bit){
        C_WRITE[current_CPU].bounce_bit = false;
        return;
    }
    if ((C_DECODE[current_CPU].oppCode != OPP_MACRO_ERET) && C_EXECUTE[current_CPU].oppCode == OPP_MACRO_UNK && C_MEMORY[current_CPU].oppCode == OPP_MACRO_UNK && ERET_STALL[current_CPU]) {

        if (VERBOSE_FLAG) printf("eret clearing condition\n");
        //C_WRITE[current_CPU].bounce_bit = false;
        C_EXECUTE[current_CPU].oppCode = OPP_MACRO_UNK;
        unset_stall(PL_ERET_STALL);
        ERET_STALL[current_CPU] = false;
        C_EXECUTE[current_CPU].stall_bit = false;
        
    }
}

/*******************************************************************
                            MEMORY
                            
                            
*******************************************************************/

    //Set instr, pc and opp_code instr
    //Determine if function accesses memory, if it is then do your opp and set the result register
    //If it does not then do nothing
    void pipe_stage_mem()
    {
        if (!RUN_BIT_ARRAY[current_CPU])
            return;
        if (C_MEMORY[current_CPU].bubble_bit){
            if (VERBOSE_FLAG)
                printf("MEM: number of dcache stall cycles entering loop %d \n", STALL_FOR_CYCLES_DCACHE[current_CPU]);
            if (STALL_FOR_CYCLES_DCACHE[current_CPU]  == 0){
                /* execute normal opp*/
                C_MEMORY[current_CPU].oppCode = C_EXECUTE[current_CPU].oppCode;
                C_MEMORY[current_CPU].instr = C_EXECUTE[current_CPU].instr;
                C_MEMORY[current_CPU].pc = C_EXECUTE[current_CPU].pc;
                C_MEMORY[current_CPU].retired = C_EXECUTE[current_CPU].retired;
                C_MEMORY[current_CPU].run_bit = C_EXECUTE[current_CPU].run_bit;
                //Set flags equal to flags for execute
                C_MEMORY[current_CPU].FLAG_Z = C_EXECUTE[current_CPU].FLAG_Z;
                C_MEMORY[current_CPU].FLAG_N = C_EXECUTE[current_CPU].FLAG_N;
                C_MEMORY[current_CPU].FLAG_C = C_EXECUTE[current_CPU].FLAG_C;
                C_MEMORY[current_CPU].FLAG_V = C_EXECUTE[current_CPU].FLAG_V;

                uint32_t currOp = C_EXECUTE[current_CPU].oppCode;
                uint32_t instr = C_MEMORY[current_CPU].instr;
                int cacheHit = cache_hit(D_CACHE, C_EXECUTE[current_CPU].result);

                if (is_memory(currOp)){
                    if (VERBOSE_FLAG)
                        printf("MEMORY: basecase about to be triggered\n");
                    if (cacheHit >= 0){
                        if (VERBOSE_FLAG) printf("DCACHE HIT\n");
                        memoryOperation_hit(currOp);
                    } else {
                        if (VERBOSE_FLAG) {
                        printf("DCACHE MISS\n");
                        printf("<=============================DCACHE FILL TRIGGERED\n");
                        }
                        memoryOperation_basecase(currOp);
                    }
                    
                    /*Check for store after load stalls*/
                    if (is_stur(C_MEMORY[current_CPU].oppCode)){
                        if (is_load(C_EXECUTE[current_CPU].oppCode)){
                            if (VERBOSE_FLAG) printf("MEMORY: store after a load stall\n");
                            set_stall(PL_STAGE_EXECUTE);
                        }
                    }
                } else {
                    C_MEMORY[current_CPU].result = C_EXECUTE[current_CPU].result;
                    C_MEMORY[current_CPU].resultRegister = C_EXECUTE[current_CPU].resultRegister;
                }
                /*Check for false run bit in all CPUs*/
                if (!C_MEMORY[current_CPU].run_bit){ // && !C_MEMORY[1].run_bit && !C_MEMORY[2].run_bit && !C_MEMORY[3].run_bit) {
                    RUN_BIT_ARRAY[current_CPU] = false;
                    //~ printf("run_bit turned off for CPU %d\n", current_CPU);
                    if (RUN_BIT_ARRAY[0] == false && RUN_BIT_ARRAY[1] == false && RUN_BIT_ARRAY[2] == false && RUN_BIT_ARRAY[3] == false) {
                        //~ printf("THE RUN BIT IS TURNED OFF PERIOD\n");
                        RUN_BIT = false;
                    }
                    return;
                }
                unset_stall(PL_DECODE_INCR_FIFTY);
            }else {
                /* insert bubble*/
                STALL_FOR_CYCLES_DCACHE[current_CPU] -= 1;
                if (VERBOSE_FLAG) printf("MEMORY: number of stall cycles after decrement %d \n", STALL_FOR_CYCLES_DCACHE[current_CPU]);
            }
            return;
        }
        if (C_MEMORY[current_CPU].stall_bit)
            return;
        if (!C_MEMORY[current_CPU].run_bit)
            return;

        C_MEMORY[current_CPU].oppCode = C_EXECUTE[current_CPU].oppCode;
        C_MEMORY[current_CPU].instr = C_EXECUTE[current_CPU].instr;
        C_MEMORY[current_CPU].pc = C_EXECUTE[current_CPU].pc;
        C_MEMORY[current_CPU].retired = C_EXECUTE[current_CPU].retired;
        C_MEMORY[current_CPU].run_bit = C_EXECUTE[current_CPU].run_bit;
        //Set flags equal to flags for execute
        C_MEMORY[current_CPU].FLAG_Z = C_EXECUTE[current_CPU].FLAG_Z;
        C_MEMORY[current_CPU].FLAG_N = C_EXECUTE[current_CPU].FLAG_N;
        C_MEMORY[current_CPU].FLAG_C = C_EXECUTE[current_CPU].FLAG_C;
        C_MEMORY[current_CPU].FLAG_V = C_EXECUTE[current_CPU].FLAG_V;

        
        uint32_t currOp = C_EXECUTE[current_CPU].oppCode;
        uint32_t instr = C_MEMORY[current_CPU].instr;

        if (is_memory(currOp)){

            calculate(currOp);
            int cacheHit = cache_hit(D_CACHE, C_EXECUTE[current_CPU].result);
            if (cacheHit >= 0){
                if (VERBOSE_FLAG)
                printf("DCACHE HIT\n");
                memoryOperation_hit(currOp);
            } else {
                set_stall(PL_DECODE_INCR_FIFTY);
            }

            //CHECK IF OPERATING ON SAME REGISTERS, THEN DONE WITH STALL
            /* Check for Stalls. <---- is this ordering correct? should the op go first? */

            /*Check for store after load stalls*/
            if (is_stur(C_MEMORY[current_CPU].oppCode)){
                if (is_load(C_EXECUTE[current_CPU].oppCode)){
                    if (VERBOSE_FLAG) printf("MEMORY: store after a load stall\n");
                    set_stall(PL_STAGE_EXECUTE);
                }
            }
        } else {
            C_MEMORY[current_CPU].result = C_EXECUTE[current_CPU].result;
            C_MEMORY[current_CPU].resultRegister = C_EXECUTE[current_CPU].resultRegister;
        }
        /*Check for false run bit*/
         if (!C_MEMORY[current_CPU].run_bit){ // && !C_MEMORY[1].run_bit && !C_MEMORY[2].run_bit && !C_MEMORY[3].run_bit) {
                    RUN_BIT_ARRAY[current_CPU] = false;
                    //~ printf("run_bit turned off for CPU %d\n", current_CPU);
                    if (RUN_BIT_ARRAY[0] == false && RUN_BIT_ARRAY[1] == false && RUN_BIT_ARRAY[2] == false && RUN_BIT_ARRAY[3] == false) {
                        //~ printf("THE RUN BIT IS TURNED OFF PERIOD\n");
                        RUN_BIT = false;
                    }
                    return;
                }

        if (C_MEMORY[current_CPU].oppCode == OPP_MACRO_ERET) {
            C_EXECUTE[current_CPU].oppCode = OPP_MACRO_UNK;
            //ERET_BUBBLE_COUNTER[current_CPU] = 2;
            set_stall(PL_ERET_STALL);
            //if (C_WRITE[current_CPU].oppCode != OPP_MA
            C_EXECUTE[current_CPU].stall_bit = true;
        }

        //~ if (C_MEMORY[current_CPU].oppCode == OPP_MACRO_ERET)
    //~ {
            //~ printf("eret in memory stage\ncurrent_CPU is: %d\n", current_CPU);
            //~ printf("cycle: %d\n", stat_cycles + 1);
            //~ printf("X29: %" PRId64 "\n", CURRENT_STATE[current_CPU].REGS[29]);
            //~ printf("X30: %" PRId64 "\n", CURRENT_STATE[current_CPU].REGS[30]);
    //~ }
    }
    
    /* Executes memory operations at the end of our miss. Eg: our base case.*/
void memoryOperation_basecase(uint32_t currOp){
        //Code seperated into LOAD and STORE cases for ease of reading
        int cacheHit = cache_hit(D_CACHE, C_EXECUTE[current_CPU].result);

        //LOAD CASE
        if (is_load(currOp)){
            //LDUR and LDURBH will set the C_MEMORY[current_CPU].result, and C_MEMORY[current_CPU].resultRegister flags
          
          	calculate(currOp);
          	int cacheHit = cache_hit(D_CACHE, C_EXECUTE[current_CPU].result);
            uint64_t addr_index = (C_EXECUTE[current_CPU].result & 0x1FE0) >> 5;
            uint64_t cache_tag = (C_EXECUTE[current_CPU].result & 0xFFFFFFFFFFFFE000) >> 11;
          	uint64_t subblock_mask = (C_EXECUTE[current_CPU].result & (0x7 << 2)) >> 2;
            uint32_t data, data_h, data1, data2; 
            switch (currOp){
                case OPP_MACRO_LDUR:
                      	data1 = mem_read_32(C_EXECUTE[current_CPU].result);
                   		data2 = mem_read_32(C_EXECUTE[current_CPU].result + 4);
                    	C_MEMORY[current_CPU].result = ((uint64_t) data2 << 32) | data1;
                    	C_MEMORY[current_CPU].resultRegister = C_EXECUTE[current_CPU].resultRegister;
                      	set_stall(PL_DECODE_INCR_FIFTY);
                    	cache_update(C_EXECUTE[current_CPU].result, D_CACHE); // C_EXECUTE[current_CPU].result is the address
                    break;
              
                case OPP_MACRO_LDURB:
                      	data = mem_read_32(C_EXECUTE[current_CPU].result);
                    	C_MEMORY[current_CPU].result = data;
                    	C_MEMORY[current_CPU].resultRegister = C_EXECUTE[current_CPU].resultRegister;
                      	set_stall(PL_DECODE_INCR_FIFTY);
                    	cache_update(C_EXECUTE[current_CPU].result, D_CACHE); // C_EXECUTE[current_CPU].result is the address
              		break;
              
                case OPP_MACRO_LDURH:
                      	data_h = mem_read_32(C_EXECUTE[current_CPU].result);
                    	C_MEMORY[current_CPU].result = zeroExtend(data_h);
                    	C_MEMORY[current_CPU].resultRegister = C_EXECUTE[current_CPU].resultRegister;
                      	set_stall(PL_DECODE_INCR_FIFTY);
                    	cache_update(C_EXECUTE[current_CPU].result, D_CACHE);
                    break;
                default:
                    if (VERBOSE_FLAG) printf("You triggered is_load for some reason\n");
            }
        } else {
        // STORE CASE
            switch(currOp){
                //NO structs stored
                case OPP_MACRO_STUR:
                    calculate(currOp);
                    cache_update(C_EXECUTE[current_CPU].result, D_CACHE);
                    dcache_modify(C_EXECUTE[current_CPU].result, (CURRENT_STATE[current_CPU].REGS[C_EXECUTE[current_CPU].resultRegister] & 0xFFFFFFFF00000000) >> 32, 
                        CURRENT_STATE[current_CPU].REGS[C_EXECUTE[current_CPU].resultRegister] & 0xFFFFFFFF, true);
                                 // replaced mem_write_32 with dcache_modify calls
                    break;
                case OPP_MACRO_STURB:
                    calculate(currOp);
                    cache_update(C_EXECUTE[current_CPU].result, D_CACHE);
                    dcache_modify(C_EXECUTE[current_CPU].result, CURRENT_STATE[current_CPU].REGS[C_EXECUTE[current_CPU].resultRegister] & 0xFF, 0, false);
                    break;
                case OPP_MACRO_STURH:
                    calculate(currOp);
                    cache_update(C_EXECUTE[current_CPU].result, D_CACHE);
                    dcache_modify(C_EXECUTE[current_CPU].result, CURRENT_STATE[current_CPU].REGS[C_EXECUTE[current_CPU].resultRegister] & 0xFFFF, 0, false);
                    break;
                case OPP_MACRO_STURW:
                    calculate(currOp);
                    cache_update(C_EXECUTE[current_CPU].result, D_CACHE);
                    dcache_modify(C_EXECUTE[current_CPU].result, CURRENT_STATE[current_CPU].REGS[C_EXECUTE[current_CPU].resultRegister], 0, false);
                    break;
            }
        }
    }

void memoryOperation_hit(uint32_t currOpp){
//Code seperated into LOAD and STORE cases for ease of reading
        //int cacheHit = cache_hit(D_CACHE, C_EXECUTE[current_CPU].resultRegister);
        //LOAD CASE
        if (is_load(currOpp)){
            //LDUR and LDURBH will set the C_MEMORY[current_CPU].result, and C_MEMORY[current_CPU].resultRegister flags


            calculate(currOpp);
            int cacheHit = cache_hit(D_CACHE, C_EXECUTE[current_CPU].result);
            uint64_t addr_index = (C_EXECUTE[current_CPU].result & 0x1FE0) >> 5;
            uint64_t cache_tag = (C_EXECUTE[current_CPU].result & 0xFFFFFFFFFFFFE000) >> 11;
            uint64_t subblock_mask = (C_EXECUTE[current_CPU].result & (0x7 << 2)) >> 2;
            uint32_t data1, data2, data, data_h; 
            switch (currOpp){
                case OPP_MACRO_LDUR:
                        data1 = CACHE[current_CPU].d_cache[addr_index].block[cacheHit].subblock_data[subblock_mask];
                        data2 = CACHE[current_CPU].d_cache[addr_index].block[cacheHit].subblock_data[subblock_mask + 1];
                        C_MEMORY[current_CPU].result = ((uint64_t) data2 << 32) | data1;
                        C_MEMORY[current_CPU].resultRegister = C_EXECUTE[current_CPU].resultRegister;
                    break;
              
                case OPP_MACRO_LDURB:
                        data = CACHE[current_CPU].d_cache[addr_index].block[cacheHit].subblock_data[subblock_mask];
                        C_MEMORY[current_CPU].result = data & 0x0000FFFF;
                        C_MEMORY[current_CPU].resultRegister = C_EXECUTE[current_CPU].resultRegister;
                    break;
              
                case OPP_MACRO_LDURH:
                        data_h = CACHE[current_CPU].d_cache[addr_index].block[cacheHit].subblock_data[subblock_mask] & 0x0000FFFF;
                        //uint32_t data3 = mem_read_32(C_EXECUTE[current_CPU].result) & 0x0000FFFF;
                        C_MEMORY[current_CPU].result = zeroExtend(data_h);
                        C_MEMORY[current_CPU].resultRegister = C_EXECUTE[current_CPU].resultRegister;
                    break;
                default:
                    if (VERBOSE_FLAG) printf("You triggered is_load for some reason\n");
            }
        } else {
        // STORE CASE
            switch(currOpp){
                //NO structs stored
                case OPP_MACRO_STUR:
                    dcache_modify(C_EXECUTE[current_CPU].result, (CURRENT_STATE[current_CPU].REGS[C_EXECUTE[current_CPU].resultRegister] & 0xFFFFFFFF00000000) >> 32, 
                        CURRENT_STATE[current_CPU].REGS[C_EXECUTE[current_CPU].resultRegister] & 0xFFFFFFFF, true); //maybe plus 4, this is a 64 bit int
                    break;
                case OPP_MACRO_STURB:
                    dcache_modify(C_EXECUTE[current_CPU].result, CURRENT_STATE[current_CPU].REGS[C_EXECUTE[current_CPU].resultRegister] & 0xFF, 0, false);
                    break;
                case OPP_MACRO_STURH:
                    dcache_modify(C_EXECUTE[current_CPU].result, CURRENT_STATE[current_CPU].REGS[C_EXECUTE[current_CPU].resultRegister] & 0xFFFF, 0, false);
                    break;
                case OPP_MACRO_STURW:
                    dcache_modify(C_EXECUTE[current_CPU].result, CURRENT_STATE[current_CPU].REGS[C_EXECUTE[current_CPU].resultRegister]);
                    break;
            }
        }
    }
/*******************************************************************
                        EXECUTE
*******************************************************************/

int can_transfer_eret_from_decode()
{
    
    if (C_EXECUTE[current_CPU].oppCode == OPP_MACRO_UNK && C_MEMORY[current_CPU].oppCode == OPP_MACRO_UNK && C_WRITE[current_CPU].oppCode == OPP_MACRO_UNK) {
        return true;
    }
    return false;
}

void pipe_stage_execute()
    {
    if (C_EXECUTE[current_CPU].oppCode == OPP_MACRO_ERET)
        {
            printf("eret at the beginning of execute stage\ncurrent_CPU is: %d\n", current_CPU);
            printf("cycle: %d\n", stat_cycles + 1);
            printf("X29: %" PRId64 "\n", CURRENT_STATE[current_CPU].REGS[29]);
            printf("X30: %" PRId64 "\n", CURRENT_STATE[current_CPU].REGS[30]);
        }
        //Set instr, and opp_code instruction
        //1) Determine if function is executeable, if it is then do your opp and set all the registers
        //2) If it does not, then do nothing
        if (C_EXECUTE[current_CPU].stall_bit)
            return;
        if (!C_DECODE[current_CPU].run_bit){
            //~ printf("run_bit is off\n");
            return;
        }
        if (C_DECODE[current_CPU].oppCode == OPP_MACRO_ERET)
        {
            if (VERBOSE_FLAG) printf("ERET is in decode\n");
        if (ERET_BUBBLE_COUNTER[current_CPU] == 2) { //unstall
                if (VERBOSE_FLAG) printf("can_transfer_eret_from_decode\n");
                unset_stall(PL_ERET_STALL);
                ERET_BUBBLE_COUNTER[current_CPU] = 0;
            }
            else {
                if (VERBOSE_FLAG) printf("pipe_stage_execute: INSERTING A BUBBLE <-------------------------------------- \n");
                C_EXECUTE[current_CPU].oppCode = OPP_MACRO_UNK;
                set_stall(PL_ERET_STALL);
                ERET_BUBBLE_COUNTER[current_CPU] += 1;
                return;
            }
        }
        C_EXECUTE[current_CPU].instr = C_DECODE[current_CPU].instr;
        C_EXECUTE[current_CPU].oppCode = C_DECODE[current_CPU].oppCode;
        C_EXECUTE[current_CPU].pc = C_DECODE[current_CPU].pc;
        C_EXECUTE[current_CPU].predicted_pc = C_DECODE[current_CPU].predicted_pc;
        C_EXECUTE[current_CPU].retired = C_DECODE[current_CPU].retired;
        C_EXECUTE[current_CPU].p_taken = C_DECODE[current_CPU].p_taken;
        //~ printf("transfered from decode to fetch\n");

        uint64_t result;
        uint32_t currOp = C_DECODE[current_CPU].oppCode;

        calculate(C_EXECUTE[current_CPU].oppCode);
        
        if (is_executeable(currOp)){
            //EXECUTE
            if(currOp == OPP_MACRO_HLT) C_EXECUTE[current_CPU].run_bit = 0;

            if(has_exec_result(currOp)){
                //result and resultRegister will be will be set in the helper function
                C_EXECUTE[current_CPU].pc = C_DECODE[current_CPU].pc + 4; 
            }
            uint64_t offset = 0; //dont know what this branching condition is
            C_EXECUTE[current_CPU].pc = C_DECODE[current_CPU].pc + offset;
        }
        

        //~ printf("C_EXECUTE[current_CPU].oppCode %d \n", C_EXECUTE[current_CPU].oppCode);
    }
    
    /*shortened for 'squash a motherfucker'*/
    void squash(int pl_stage_macro){
        if (VERBOSE_FLAG)
        printf("SQUASH TRIGGERED\n");
        switch(pl_stage_macro){
            case PL_STAGE_DECODE:
                C_DECODE[current_CPU].oppCode = OPP_MACRO_NOP;
                C_FETCH[current_CPU].instr = OPP_MACRO_NOP;
                // C_EXECUTE[current_CPU].retired = 0;
                C_FETCH[current_CPU].squash_bit = true;
                break;
            default:
            if (VERBOSE_FLAG)
                printf("tried to squash a non squashable macro stage\n");
                break;
        }
    }

    void pseudo_squash(int pl_stage_macro){
        if (VERBOSE_FLAG)
        printf("PSEUDO SQUASH TRIGGERED\n");
        switch(pl_stage_macro){
            case PL_STAGE_DECODE:
                C_FETCH[current_CPU].pseudo_stall_bit = true;
                C_FETCH[current_CPU].instr = OPP_MACRO_UNK; //<-- this will clear whats in decode
                break;
            default:
            if (VERBOSE_FLAG)
                printf("tried to squash a non squashable macro stage\n");
                break;
        }

        if (C_EXECUTE[current_CPU].oppCode == OPP_MACRO_ERET)
    {
            printf("eret at the end of execute stage\ncurrent_CPU is: %d\n", current_CPU);
            printf("cycle: %d\n", stat_cycles + 1);
            printf("X29: %" PRId64 "\n", CURRENT_STATE[current_CPU].REGS[29]);
            printf("X30: %" PRId64 "\n", CURRENT_STATE[current_CPU].REGS[30]);
    }
    }
    
/*******************************************************************
                            DECODE
*******************************************************************/

    void pipe_stage_decode()
    {
        if (C_DECODE[current_CPU].eret_stall_bit) {
            return;
        }
        if (C_DECODE[current_CPU].bubble_bit){
            C_DECODE[current_CPU].oppCode = OPP_MACRO_UNK;
            
            if (VERBOSE_FLAG) printf("pipe_stage_decode: INSERTED BUBBLE\n");
            return;
        }
        if (C_DECODE[current_CPU].stall_bit)
            return;
        if (C_DECODE[current_CPU].oppCode == OPP_MACRO_HLT){
            return;
        }
        if (!RUN_BIT)
            return;
        C_DECODE[current_CPU].run_bit = 1;   
        C_DECODE[current_CPU].predicted_pc = C_FETCH[current_CPU].predicted_pc;
        C_DECODE[current_CPU].pc = C_FETCH[current_CPU].pc;
        C_DECODE[current_CPU].instr = C_FETCH[current_CPU].instr;
        C_DECODE[current_CPU].oppCode = get_opp_code(C_FETCH[current_CPU].instr);
        C_DECODE[current_CPU].p_taken = C_FETCH[current_CPU].p_taken;

        if (VERBOSE_FLAG) {
        printf("C_DECODE[current_CPU].oppCode: %d\n", C_DECODE[current_CPU].oppCode);
        printf("C_FETCH[current_CPU].instr: %" PRIx64 "\n", C_FETCH[current_CPU].instr);
        }
        
        // printf("DECODE: Hexline is %08x\n", C_DECODE[current_CPU].instr);
        /* Do not touch the line below, we don't know why it works but it does*/
        if(is_retirable(C_DECODE[current_CPU].oppCode)){
            C_DECODE[current_CPU].retired = 1;
        } else{
            C_DECODE[current_CPU].retired = 0;
        }

        if (C_DECODE[current_CPU].oppCode == OPP_MACRO_ERET)
        {
            //set_stall(PL_ERET_STALL);
            ERET_STALL[current_CPU] = true;
            return;
        }
    }


/*******************************************************************
                        ERET helper function
*******************************************************************/

int is_eret(uint32_t instr) {
    if (instr == 0xd69f03e0)
    {
        return 1;
    }
    else {
        return 0;
    }
}


/*******************************************************************
                Decode helper to get the oppCode
*******************************************************************/

    int get_opp_code(uint32_t hexLine){
        if (hexLine ==  OPP_MACRO_UNASS) //<-- SPECIAL STALL CASE
                return OPP_MACRO_UNASS;
        if (hexLine == 0xd69f03e0) {
            return OPP_MACRO_ERET;
        }
        //Bitmasks
        int bitMask = 0xFFE00000; //BitMask: 0111 1111 1110 0->
        int bitMaskSix = 0xFC000000;
        int bitMaskEight = 0xFF000000;  
        int bitMaskTen = 0xFFC00000 ;//BitMask: 0111 1111 1100 0->

        // //Actual Bit Counts
        int firstEleven = bitMask & hexLine;
        if (VERBOSE_FLAG) {
        printf("DECODE: Hexline is %08x\n", hexLine);
        printf("DECODE: The current opp code in hex is %08x\n", firstEleven);
        }



        /* Switch Statement for 11 digit opp codes*/
        switch(firstEleven){
            case 0x8B200000: //extended add
            case 0x8B000000:
                return OPP_MACRO_ADD;
                             //shifted add
            case 0xAB200000: //extended adds
            case 0xAB000000:
                return OPP_MACRO_ADDS;
                             //shifted adds
            case 0X8A000000:
            case 0X8AC00000:
            case 0X8A400000:
            case 0X8A800000:
                return OPP_MACRO_AND; 
            case 0XEA000000:
            case 0XEAC00000:
            case 0XEA400000:
            case 0XEA800000:
                return OPP_MACRO_ANDS;
            case 0XCA000000:
            case 0XCAC00000:
            case 0XCA400000:
            case 0XCA800000:
                return OPP_MACRO_EOR;
            case 0XF8400000: //1_11 1000 0100
                return OPP_MACRO_LDUR;
            case 0X38400000:
                return OPP_MACRO_LDURB;
            case 0x78400000:
                return OPP_MACRO_LDURH;
            case 0X1AC00000:
                if((((hexLine & 0x03) << 15) >> 25) == 0b001000)
                    return OPP_MACRO_SDIV;
            //need an if statement between LSL and LSR around 0x3
                return OPP_MACRO_LSL;
                return OPP_MACRO_LSR;
            case 0xD6000000:
                return OPP_MACRO_LSL;
            case 0XD2000000:
            case 0XD2400000:
            case 0XD2c00000:
            case 0XD2800000: 
                return OPP_MACRO_MOVZ;
            case 0Xaa000000:
            case 0Xaa400000:
            case 0Xaa800000:
            case 0Xaac00000:
                return OPP_MACRO_ORR;
            case 0xF8000000:
                return OPP_MACRO_STUR;
            case 0x38000000:
                return OPP_MACRO_STURB;
            case 0x78000000:
                return OPP_MACRO_STURH;            
            case 0XCB000000:
            case 0XCB800000:
                return OPP_MACRO_SUB;
            case 0XEB000000:
            case 0XEB200000:
                return OPP_MACRO_SUBS;
            case 0X9b000000:
                return OPP_MACRO_MUL;
            case 0Xd4400000:
                return OPP_MACRO_HLT;
                //same as SUBIS
            default:
                /* Ten Digit Opp Codes*/
                switch(hexLine & bitMaskTen){
                    case 0x91000000:
                    case 0x91400000:
                    case 0x91800000:
                    case 0x91c00000:
                        return OPP_MACRO_ADDI;
                    case 0xb1000000:
                    case 0xb1400000:
                    case 0xb1800000:
                    case 0xb1c00000:
                        return OPP_MACRO_ADDIS;
                    case 0xD3400000:
                        return OPP_MACRO_LSLI;
                    case 0xD1000000:
                    case 0xD1400000:
                    case 0xD1800000:
                    case 0xD1C00000:
                        return OPP_MACRO_SUBI;
                    case 0xF1000000:
                    case 0xF1400000:
                    case 0xF1800000:
                    case 0xF1C00000:
                        return OPP_MACRO_SUBIS;
                    default:
                        break;
                }
                switch (hexLine & bitMaskSix){
                    case 0x14000000:
                        return OPP_MACRO_B;
                    case 0x54000000:
                        return OPP_MACRO_BEQ;
                    case 0x94000000:
                        return OPP_MACRO_BL;
                    case 0xD6000000:
                        return OPP_MACRO_BR;
                    default:
                        break;
                }
                switch (hexLine & bitMaskEight){
                    case 0xB5000000:
                        return OPP_MACRO_CBNZ;
                    case 0xB4000000:
                        return OPP_MACRO_CBZ;
                    default:
                        break;
                }
                return OPP_MACRO_UNK;
        }
    }
    
/*******************************************************************
                            FETCH
*******************************************************************/

    void pipe_stage_fetch()
    {
        //~ printf("FETCH: Currently %d STALL_FOR_CYCLES[current_CPU] for start_addr %08x\n", STALL_FOR_CYCLES[current_CPU], STALL_START_ADDR[current_CPU]);
        if (VERBOSE_FLAG) ("FETCH: stall for cycles %d\n", STALL_FOR_CYCLES[current_CPU]);
        uint32_t currOpp;
        int cacheHit;
        if (C_FETCH[current_CPU].eret_stall_bit) {
            return;
        }
        if ((!C_MEMORY[current_CPU].run_bit) || (!C_EXECUTE[current_CPU].run_bit) || (C_MEMORY[current_CPU].bubble_bit)){
            if (STALL_FOR_CYCLES[current_CPU] > 0){
                STALL_FOR_CYCLES[current_CPU] -= 1;
                if ((STALL_FOR_CYCLES[current_CPU] == 0) && (STALL_FOR_CYCLES_DCACHE[current_CPU] > 1))
                    C_DECODE[current_CPU].is_overrideable_bubble = true;
            }
            if (VERBOSE_FLAG)
            printf("STALL-FOR_CYCLES %d AND DCACHE STALL %d\n", STALL_FOR_CYCLES[current_CPU], STALL_FOR_CYCLES_DCACHE[current_CPU]);
            if ((STALL_FOR_CYCLES[current_CPU] == 0) && (STALL_FOR_CYCLES_DCACHE[current_CPU] == 1) && (C_DECODE[current_CPU].is_overrideable_bubble)){
                // C_DECODE[current_CPU].predicted_pc = C_FETCH[current_CPU].predicted_pc;
                // C_DECODE[current_CPU].pc = C_FETCH[current_CPU].pc;
                // C_DECODE[current_CPU].instr = C_FETCH[current_CPU].instr;
                // C_DECODE[current_CPU].oppCode = get_opp_code(C_FETCH[current_CPU].instr);
                // C_DECODE[current_CPU].p_taken = C_FETCH[current_CPU].p_taken;
                fetch_base();
            }
            return;
         }
        if (C_FETCH[current_CPU].bounce_bit){
            C_FETCH[current_CPU].bounce_bit = false;
            // C_DECODE[current_CPU].bubble_bit = false;
            unset_stall(PL_STAGE_MEMORY);
            return;
        }
        if (C_FETCH[current_CPU].stall_bit){
            /* Finished Stalling. Actually load mem values in */
            if (STALL_FOR_CYCLES[current_CPU] == 0){
                fetch_base();
                // unset_stall(PL_INCREMENT_FIFTY);
                // cacheHit = cache_hit(I_CACHE, CURRENT_STATE[current_CPU].PC);
                // currOpp = mem_read_32(CURRENT_STATE[current_CPU].PC);
                // cache_update(STALL_START_ADDR[current_CPU], I_CACHE);
                // C_EXECUTE[current_CPU].branch_stall_bit = false;
                // C_FETCH[current_CPU].instr = currOpp;
                // C_FETCH[current_CPU].pc = CURRENT_STATE[current_CPU].PC;  
                // bp_predict(CURRENT_STATE[current_CPU].PC);


                // if (C_FETCH[current_CPU].pseudo_stall_bit)
                // {
                //     C_FETCH[current_CPU].pseudo_stall_bit = false;
                //     return;
                // }
                // C_FETCH[current_CPU].predicted_pc = CURRENT_STATE[current_CPU].PC;
                return;
            } else if (STALL_FOR_CYCLES[current_CPU] > 0){
                insert_bubble(PL_STAGE_DECODE);
                STALL_FOR_CYCLES[current_CPU] -= 1;
            }
            return;
        }
        if (C_FETCH[current_CPU].squash_bit){
            C_FETCH[current_CPU].squash_bit = false;
            return;
        }


        if (!RUN_BIT)
            return;
        // printf("FETCH: pc as hex is %" PRIx64 "\n", CURRENT_STATE[current_CPU].PC);
        uint64_t current_state_index = (CURRENT_STATE[current_CPU].PC & 0x7E0) >> 5;
        uint64_t current_state_tag = (CURRENT_STATE[current_CPU].PC & 0xFFFFFFFFFFFFF800) >> 11;
        cacheHit = cache_hit(I_CACHE, CURRENT_STATE[current_CPU].PC);
        if (VERBOSE_FLAG)
        printf("FETCH: cacheHit is %d\n", cacheHit);
        /*Cache Hit: Load from Cache*/
		if(cacheHit >= 0)
		{
            if (VERBOSE_FLAG)
			printf("ICACHE HIT: The index is %d\n", (int) current_state_index);
        	/*setting the curOpp to the instruction that is stored then update cache*/
        	uint64_t subblock_mask = (CURRENT_STATE[current_CPU].PC & (0x7 << 2)) >> 2;
            if (VERBOSE_FLAG){
                printf("CURRENT_STATE[current_CPU].PC: 0x%" PRIx64 "\n", CURRENT_STATE[current_CPU].PC);
                printf("subblock mask is %d\n", (int32_t) subblock_mask);
            }
        	currOpp = CACHE[current_CPU].i_cache[current_state_index].block[cacheHit].subblock_data[subblock_mask];
            //printf("FETCH: cache hit @ instruction is 0x%" PRIx64 "\n", (int64_t) currOpp);
            //printf("FETCH: i_cache_index: %d, block: %d, subblock: %d\n", current_state_index, cacheHit, subblock_mask);
            // printf("FETCH currOpp: %08x\n", currOpp);
        	cache_update(CURRENT_STATE[current_CPU].PC, I_CACHE);
            C_EXECUTE[current_CPU].branch_stall_bit = false;
            C_FETCH[current_CPU].instr = currOpp;
            C_FETCH[current_CPU].pc = CURRENT_STATE[current_CPU].PC;  
            bp_predict(CURRENT_STATE[current_CPU].PC);

            if (C_FETCH[current_CPU].pseudo_stall_bit)
            {
            C_FETCH[current_CPU].pseudo_stall_bit = false;
            return;
            }
            C_FETCH[current_CPU].predicted_pc = CURRENT_STATE[current_CPU].PC;
		}
        /*Cache Miss: Trigger 50 cycle stall*/
		else
        {
            if (VERBOSE_FLAG)
        	printf("ICACHE MISS asdfas: The index is %d\n", (int) current_state_index);

            set_stall(PL_INCREMENT_FIFTY);

        	// currOpp = mem_read_32(CURRENT_STATE[current_CPU].PC);
         //    printf("currOpp is %d\n", currOpp);
         //    cache_update(CURRENT_STATE[current_CPU].PC, I_CACHE);
		}
    }
    
/*******************************************************************
    STALLING: Functions to implement stalling
*******************************************************************/
    void fetch_base(void){
            unset_stall(PL_INCREMENT_FIFTY);
            int cacheHit = cache_hit(I_CACHE, CURRENT_STATE[current_CPU].PC);
            uint32_t currOpp = mem_read_32(CURRENT_STATE[current_CPU].PC);
            if (VERBOSE_FLAG) ("FETCH: fetch base @ instruction is 0x%" PRIx64 "\n", (int64_t)currOpp);
            cache_update(STALL_START_ADDR[current_CPU], I_CACHE);
            if (VERBOSE_FLAG) ("current_CPU: %d, loading from 0x%" PRIx64 " from cache\n", current_CPU, STALL_START_ADDR[current_CPU]);
            C_EXECUTE[current_CPU].branch_stall_bit = false;
            C_FETCH[current_CPU].instr = currOpp;
            C_FETCH[current_CPU].pc = CURRENT_STATE[current_CPU].PC;  
            bp_predict(CURRENT_STATE[current_CPU].PC);


            if (C_FETCH[current_CPU].pseudo_stall_bit)
            {
                C_FETCH[current_CPU].pseudo_stall_bit = false;
                return;
            }
            C_FETCH[current_CPU].predicted_pc = CURRENT_STATE[current_CPU].PC;
    }


    /*Helper function to set stalls. Input is the stage you "start"
    stalling at. Then you work backwards. EX/ if you "start" at MEM, 
    then stall MEM, EXEC, DECODE AND FETCH*/
    void set_stall(int start_stage){
        
        switch(start_stage){
            case PL_STAGE_FETCH:
            if (VERBOSE_FLAG)
                printf("PL_STAGE_FETCH STALL SET\n");
                //set just fetch stall
                C_FETCH[current_CPU].stall_bit = true;
                break;
            case PL_STAGE_DECODE:
            if (VERBOSE_FLAG)
                printf("PL_STAGE_DECODE STALL SET\n");
                //set decode stall
                C_EXECUTE[current_CPU].branch_stall_bit = true;
                C_FETCH[current_CPU].stall_bit = true;
                C_DECODE[current_CPU].stall_bit = true;
                break;
            case PL_STAGE_EXECUTE:
            if (VERBOSE_FLAG)
                printf("PL_STAGE_EXECUTE STALL SET\n");
                //set execute stall
                C_FETCH[current_CPU].stall_bit = true;
                C_DECODE[current_CPU].stall_bit = true;
                C_EXECUTE[current_CPU].stall_bit = true;

                C_FETCH[current_CPU].bounce_bit = true;
                break;
            case PL_STAGE_MEMORY:
            if (VERBOSE_FLAG)
                printf("PL_STAGE_MEMORY STALL SET\n");
                //set memory stall
                C_FETCH[current_CPU].stall_bit = true;
                C_DECODE[current_CPU].stall_bit = true;
                C_EXECUTE[current_CPU].stall_bit = true;
                C_MEMORY[current_CPU].stall_bit = true;
                break;
            case PL_STAGE_WRITE:
            if (VERBOSE_FLAG)
                printf("PL_STAGE_WRITE STALL SET\n");
                /*I'm not quite sure when you would execute a stall on write?!?!?!
                Leaving it here for consistency reasons*/
                C_FETCH[current_CPU].stall_bit = true;
                C_DECODE[current_CPU].stall_bit = true;
                C_EXECUTE[current_CPU].stall_bit = true;
                C_MEMORY[current_CPU].stall_bit = true;
                C_WRITE[current_CPU].stall_bit = true;
                if (VERBOSE_FLAG)
                printf("Error, attempted stall at WB stage");
                break;
            case PL_INCREMENT_FIFTY:
            if (VERBOSE_FLAG)
                printf("PL_INCREMENT_FIFTY STALL SET\n");
                C_FETCH[current_CPU].stall_bit = true;
                // C_DECODE[current_CPU].stall_bit = true;
                // C_EXECUTE[current_CPU].stall_bit = true;
                // C_MEMORY[current_CPU].stall_bit = true;
                // C_WRITE[current_CPU].stall_bit = true;
                STALL_FOR_CYCLES[current_CPU] = 49;
                STALL_START_ADDR[current_CPU] = CURRENT_STATE[current_CPU].PC;

                break;
            case PL_DECODE_INCR_FIFTY:
            if (VERBOSE_FLAG)
                printf("PL_DECODE_INCR_FIFTY STALL SET\n");
                // C_FETCH[current_CPU].stall_bit = true;
                C_DECODE[current_CPU].stall_bit = true;
                C_EXECUTE[current_CPU].stall_bit = true;
                // C_MEMORY[current_CPU].stall_bit = true;
                C_MEMORY[current_CPU].bubble_bit = true;

                C_WRITE[current_CPU].stall_bit = true;
                STALL_FOR_CYCLES_DCACHE[current_CPU] = 49;
                break;
            case PL_ERET_STALL:
                if (VERBOSE_FLAG) printf("PL_ERET_STALL STALL SET\n");
                C_FETCH[current_CPU].eret_stall_bit = true;
                C_DECODE[current_CPU].eret_stall_bit = true;
                ERET_STALL[current_CPU] = true;
                break;
            default:
                break;
        }
    }

    /* Inverse of set_stall*/
    void unset_stall(int start_stage){
        switch(start_stage){
            case PL_INCREMENT_FIFTY:
            if (VERBOSE_FLAG)
                printf("unincrement fifty\n");
                C_FETCH[current_CPU].stall_bit = false;
                C_DECODE[current_CPU].stall_bit = false;
                C_DECODE[current_CPU].bubble_bit = false;
                // C_EXECUTE[current_CPU].stall_bit = false;
                // C_MEMORY[current_CPU].stall_bit = false;
                // C_WRITE[current_CPU].stall_bit = false;
                STALL_FOR_CYCLES[current_CPU] = 0;
                break;

            case PL_STAGE_MEMORY:
            if (VERBOSE_FLAG)
                printf("unset pl_stage_memory\n");
                C_FETCH[current_CPU].stall_bit = false;
                C_DECODE[current_CPU].bubble_bit = false;
                C_DECODE[current_CPU].stall_bit = false;
                C_EXECUTE[current_CPU].stall_bit = false;
                C_MEMORY[current_CPU].bubble_bit = false;
                break;
            case PL_DECODE_INCR_FIFTY:
            if (VERBOSE_FLAG)
                printf("ending dcache stall\n");
                // C_FETCH[current_CPU].stall_bit = false;
                C_DECODE[current_CPU].stall_bit = false;
                C_EXECUTE[current_CPU].stall_bit = false;
                // C_MEMORY[current_CPU].stall_bit = false;
                C_MEMORY[current_CPU].bubble_bit = false;

                C_WRITE[current_CPU].stall_bit = false;
                STALL_FOR_CYCLES_DCACHE[current_CPU] = 0;
                break;
            case PL_ERET_STALL:
                C_FETCH[current_CPU].eret_stall_bit = false;
                C_DECODE[current_CPU].eret_stall_bit = false;
                break;
            default:
            if (VERBOSE_FLAG)
                printf("You are trying to unstall an invalid stage\n");
                break;
        }
    }
    /*Fxn to unset stall bits */
    void unset_bits(void){
        if (C_WRITE[current_CPU].stall_bit)
            unset_stall(PL_STAGE_WRITE);
        if (C_MEMORY[current_CPU].stall_bit)
            unset_stall(PL_STAGE_MEMORY);
        if (C_EXECUTE[current_CPU].stall_bit)
            unset_stall(PL_STAGE_EXECUTE);
        if (C_DECODE[current_CPU].stall_bit)
            unset_stall(PL_STAGE_DECODE);
        if (C_FETCH[current_CPU].stall_bit)
            unset_stall(PL_STAGE_FETCH);
    }


    void exec_stall(int register_number){
        /*Check for LOAD in MEM, and "write" Execution operations for stalls*/
        //Check if the operation in MEMORY stage is a LOAD, and is possible of triggering stuff
        if (VERBOSE_FLAG) printf("EXEC_STALL: exec stall entered\n");
        if (is_load(C_MEMORY[current_CPU].oppCode)){
            //Check if the registers overlap between MEM, and current
            if (register_number == C_MEMORY[current_CPU].resultRegister){ 
                if (VERBOSE_FLAG) printf("EXEC STALL: EXEC STALL EXECUTED\n");
                set_stall(PL_STAGE_EXECUTE);
                // insert_bubble(PL_STAGE_MEMORY);
                C_EXECUTE[current_CPU].retired = 0;
            }
        }
    }

    void insert_bubble(int at_stage){
        switch(at_stage){
            case PL_STAGE_DECODE:
            if (VERBOSE_FLAG)
                printf("bubble inserted\n");

                C_FETCH[current_CPU].stall_bit = true;
                C_DECODE[current_CPU].bubble_bit = true;
                C_DECODE[current_CPU].stall_bit = false;
                C_DECODE[current_CPU].instr = OPP_MACRO_UNK;
                C_DECODE[current_CPU].oppCode = OPP_MACRO_UNK;
                // STALL_FOR_CYCLES[current_CPU] = 1;
                break;
            /* Note this option actually inserts a bubble into decode. The code is the same
            as the above, but this is a bubble that is executed from a STALL, and not 
            as a result of a cache miss*/
            case PL_STAGE_MEMORY:
            if (VERBOSE_FLAG)
                printf("RAW bubble inserted\n");
                C_FETCH[current_CPU].stall_bit = true;
                C_DECODE[current_CPU].stall_bit = true;
                C_EXECUTE[current_CPU].stall_bit = true;

                C_MEMORY[current_CPU].bubble_bit = true;
                C_MEMORY[current_CPU].instr = OPP_MACRO_UNK;
                C_MEMORY[current_CPU].oppCode = OPP_MACRO_UNK;
                // STALL_FOR_CYCLES[current_CPU] = 1;
                break;
            default:
            if (VERBOSE_FLAG)
                printf("You are trying to unstall an invalid stage\n");
                break;
            }
    }

/*******************************************************************
    FORWARDING: helper functions for forwarding
*******************************************************************/

    uint64_t forward(int register_number){
        if (VERBOSE_FLAG)
        printf("FORWARD: forward on register_number -----> %d\n", register_number);
        int64_t register_value = CURRENT_STATE[current_CPU].REGS[register_number];
        if (VERBOSE_FLAG)
        printf("FORWARD: register_value %d for register %d\n", (int) register_value, register_number);
        /*Check for wb forward*/
        int wb_reg = C_WRITE[current_CPU].resultRegister;
        if (wb_reg == register_number){
            if (VERBOSE_FLAG)
            printf("FORWARD: WB FORWARD TRIGGERED\n");


            //Check if the value in the WRITE stage is the same register
            register_value = C_WRITE[current_CPU].result; 
        }
        if (VERBOSE_FLAG)
        printf("FORWARD: returning WRITE mem forward %d\n", (int) register_value);
        /*Check for mem forward*/
        int mem_reg = C_MEMORY[current_CPU].resultRegister;
        if (mem_reg == register_number){

            //check if the value in the MEMORY stage is the same
            //registers are the same and we SHOULD forward ONE OF THE OPERANDS
            register_value = C_MEMORY[current_CPU].result; 
        }
        if (VERBOSE_FLAG)
        printf("FORWARD: returning MEMORY forward %d\n", (int) register_value);

        return register_value;
    }



/*******************************************************************
    OPPCODE HELPERS: helper functions to determine if a certain instruction 
    should pass through following step
*******************************************************************/

    int is_executeable(int opp_code){
        switch(opp_code){
            /* The following codes will not pass through executable */
            case OPP_MACRO_LDUR:
            case OPP_MACRO_LDURB:
            case OPP_MACRO_LDURH:
            case OPP_MACRO_MOVZ:
            case OPP_MACRO_STUR:
            case OPP_MACRO_STURB:
            case OPP_MACRO_STURH:
            case OPP_MACRO_STURW:
            case OPP_MACRO_UNK:
            case OPP_MACRO_UNASS:
            case OPP_MACRO_ERET:
                return false;
            default:
                return true;
        }
    }
    int has_exec_result(int opp_code){
        switch(opp_code){
            /* The following opp codes will EXECUTE but will NOT create a result or write to a register */
            case OPP_MACRO_B:
            case OPP_MACRO_BEQ:
            case OPP_MACRO_BL:
            case OPP_MACRO_BR:
            case OPP_MACRO_CBZ:
            case OPP_MACRO_CBNZ:
            case OPP_MACRO_HLT:
            case OPP_MACRO_UNK:
            case OPP_MACRO_UNASS:
            case OPP_MACRO_ERET:
                return false;
            default:
                return true;
        }
    }
    int is_writeable(int opp_code){
        /* The following functions will not write to a register*/
        switch(opp_code){
            case OPP_MACRO_B:
            case OPP_MACRO_BEQ:
            case OPP_MACRO_BL:
            case OPP_MACRO_BR:
            case OPP_MACRO_CBZ:
            case OPP_MACRO_CBNZ:
            case OPP_MACRO_UNK:
            case OPP_MACRO_STUR:
            case OPP_MACRO_STURB:
            case OPP_MACRO_STURH:
            case OPP_MACRO_STURW:
            case OPP_MACRO_HLT:
            case OPP_MACRO_ERET:
                return false;
            default:
                return true;
        }
    }
    int is_memory(int opp_code){
        switch (opp_code){
            /* The following codes will access memory*/
            case OPP_MACRO_LDUR:
            case OPP_MACRO_LDURB:
            case OPP_MACRO_LDURH:
            case OPP_MACRO_STUR:
            case OPP_MACRO_STURB:
            case OPP_MACRO_STURH:
            case OPP_MACRO_STURW:
                return true;
            default:
                return false;
        }
    }
    int is_load(int opp_code){
        switch (opp_code){
            /* The following codes are LOADS*/
            case OPP_MACRO_LDUR:
            case OPP_MACRO_LDURB:
            case OPP_MACRO_LDURH:
                return true;
            default:
                return false;
        }
    }
    int is_stur(int opp_code){
    switch (opp_code){
        /* The following codes are LOADS*/
        case OPP_MACRO_STUR:
        case OPP_MACRO_STURB:
        case OPP_MACRO_STURH:
        case OPP_MACRO_STURW:
            return true;
        default:
            return false;
        }
    }
    int is_flaggable(int opp_code){
        switch(opp_code){
            case OPP_MACRO_ADDS:
            case OPP_MACRO_ADDIS:
            case OPP_MACRO_CMP:
            case OPP_MACRO_SUBS:
            case OPP_MACRO_SUBIS:
            case OPP_MACRO_ANDS:
            case OPP_MACRO_ORR:
            case OPP_MACRO_EOR:
                return true;
            default:
                return false;
        }
    }
    int is_retirable(int oppCode){
        //TODO: If have time: fix the HLT thing
        switch(oppCode){
            case OPP_MACRO_UNK:
            case OPP_MACRO_UNASS:
            case OPP_MACRO_HLT:
                return false;
            default:
                return true;
        }
    }
    int is_stall_branch(int oppCode){
        switch(oppCode){
            case OPP_MACRO_CBZ:
            case OPP_MACRO_CBNZ:
            case OPP_MACRO_BEQ:
            case OPP_MACRO_BR:
                return true;
            default:
                return false;
        }
    }
    int is_squash_branch(int oppCode){
        switch(oppCode){
            case OPP_MACRO_CBZ:
            case OPP_MACRO_CBNZ:
            case OPP_MACRO_BEQ:
            case OPP_MACRO_B:
                return true;
            default:
                return false;
        }
    }
    int is_uncond(int opp_code){
        switch(opp_code){
            /* Automatically always take the condition */
            case OPP_MACRO_B:
            case OPP_MACRO_BL:
            case OPP_MACRO_BR:
                return true;
            default:
                return false;
        }
    }

    int same_subblock(uint64_t stall_start_addr, uint64_t test_addr){
        if (VERBOSE_FLAG)
        printf("<------------\n");
        int i;
        int flag = false;
        for (i = 0; i < 8; i ++){
            if (test_addr == stall_start_addr + (i * 4)){
                flag = true;
                if (VERBOSE_FLAG)
                printf("*********************************SAME SUBBLOCK\n");
            }

        }
        return flag;
    }
/* Helper function to distribute stores and loads */
