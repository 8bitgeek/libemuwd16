/* cpu.c         (c) Copyright Mike Noel, 2001-2008                  */
/* ----------------------------------------------------------------- */
/*                                                                   */
/* This software is an emulator for the Alpha-Micro AM-100 computer. */
/* It is copyright by Michael Noel and licensed for non-commercial   */
/* hobbyist use under terms of the "Q public license", an open       */
/* source certified license.  A copy of that license may be found    */
/* here:       http://www.otterway.com/am100/license.html            */
/*                                                                   */
/* There exist known serious discrepancies between this software's   */
/* internal functioning and that of a real AM-100, as well as        */
/* between it and the WD-1600 manual describing the functionality of */
/* a real AM-100, and even between it and the comments in the code   */
/* describing what it is intended to do! Use it at your own risk!    */
/*                                                                   */
/* Reliability aside, it isn't the intent of the copyright holder to */
/* use this software to compete with current or future Alpha-Micro   */
/* products, and no such competing application of the software will  */
/* be supported.                                                     */
/*                                                                   */
/* Alpha-Micro and other software that may be run on this emulator   */
/* are not covered by the above copyright or license and must be     */
/* legally obtained from an authorized source.                       */
/*                                                                   */
/* ----------------------------------------------------------------- */

#include "wd16.h"
#include "cpu-fmt1.h"
#include "cpu-fmt2.h"
#include "cpu-fmt3.h"
#include "cpu-fmt4.h"
#include "cpu-fmt5.h"
#include "cpu-fmt6.h"
#include "cpu-fmt7.h"
#include "cpu-fmt8.h"
#include "cpu-fmt9.h"
#include "cpu-fmt10.h"
#include "cpu-fmt11.h"

REGS regs;

uint16_t oldPCs[256];       /* table of prior PC's */
unsigned oldPCindex;        /* pointer to next entry in prior PC's table */
uint16_t op, opPC;          /* current opcode (base) and its location */
char cpu4_svcctxt[16];      /* my SVCCs starts LO and go up, or */
                            /*          starts HI and go down.. */

pthread_mutex_t intlock_t;  /* interrupt lock */
pthread_t cpu_t;            /* cpu thread */

char instruction_type[65536];  /* maps all words to inst format type */



/*-------------------------------------------------------------------*/
/*                                                                   */
/* This module executes am100 instructions.                          */
/*                                                                   */
/*-------------------------------------------------------------------*/

/*-------------------------------------------------------------------*/
/* build instruction decode table                                    */
/*-------------------------------------------------------------------*/
void build_decode() {
  int i, j;
  /* fill table with 0's (invalid opcode type) */
  for (i = 0; i < 65536; i++)
    instruction_type[i] = 0;
  /* fill format 1 part of table */
  for (i = 0; i < 16; i++)
    instruction_type[i] = 1;
  /* fill format 2 part of table */
  for (i = 16; i < 48; i++)
    instruction_type[i] = 2;
  /* fill format 3 part of table */
  for (i = 48; i < 64; i++)
    instruction_type[i] = 3;
  /* fill format 4 part of table */
  for (i = 64; i < 256; i++)
    instruction_type[i] = 4;
  /* fill format 5 part of table */
  for (i = 1; i < 8; i++)
    for (j = 0; j < 256; j++)
      instruction_type[j + (i * 256)] = 5;
  for (i = 0; i < 8; i++)
    for (j = 0; j < 256; j++)
      instruction_type[32768 + j + (i * 256)] = 5;
  /* fill format 6 (split ops) part of table */
  for (i = 8; i < 10; i++)
    for (j = 0; j < 256; j++)
      instruction_type[j + (i * 256)] = 6;
  for (i = 8; i < 10; i++)
    for (j = 0; j < 256; j++)
      instruction_type[32768 + j + (i * 256)] = 6;
  for (i = 14; i < 16; i++)
    for (j = 0; j < 256; j++)
      instruction_type[32768 + j + (i * 256)] = 6;
  /* fill format 7 part of table */
  for (i = 10; i < 14; i++)
    for (j = 0; j < 256; j++)
      instruction_type[j + (i * 256)] = 7;
  for (i = 10; i < 14; i++)
    for (j = 0; j < 256; j++)
      instruction_type[32768 + j + (i * 256)] = 7;
  /* fill format 8 part of table */
  for (i = 14; i < 16; i++)
    for (j = 0; j < 256; j++)
      instruction_type[j + (i * 256)] = 8;
  /* fill format 9 part of table */
  for (i = 112; i < 128; i++)
    for (j = 0; j < 256; j++)
      instruction_type[j + (i * 256)] = 9;
  /* fill format 10 part of table */
  for (i = 16; i < 112; i++)
    for (j = 0; j < 256; j++)
      instruction_type[j + (i * 256)] = 10;
  for (i = 16; i < 112; i++)
    for (j = 0; j < 256; j++)
      instruction_type[32768 + j + (i * 256)] = 10;
  /* fill format 11 part of table */
  for (i = 0; i < 5; i++)
    for (j = 0; j < 256; j++)
      instruction_type[61440 + j + (i * 256)] = 11;

} /* end function build_decode */

/*-------------------------------------------------------------------*/
/* when the opcode is invalid...                                     */
/*-------------------------------------------------------------------*/
void do_fmt_invalid() {

  //      SYSTEM ERROR TRAPS
  //      -------------------------------------------------------------
  //      With the exception of the major power fail error that is a function
  //      of a system reset, all error conditions perform a common routine as
  //      outlined below.
  //      A non-vectored interrupt and some op codes also use this routine.
  //      The numbers in parenthesis refer to notes that follow the table.
  //
  //      1) PS is pushed onto the stack
  //      2) PC is pushed onto the stack
  //      3) PC is fetched from location X where "X" is from the following table
  //
  //      (1)(2)(3) "12" for bus error PC
  //      (1)(2)(3) "14" for nonvectored interrupt power fail PC
  //      (1)(2)(3) "18" for parity error PC
  // **   (1)(2)(3) "lA" for reserved op code error PC
  // **   (1)(2)(3) "1C" for illegal op code format error PC
  //      (1)(2)(3) "1E" for XCT error PC
  //      (1)(2)    "20" for XCT trace PC
  //      (1)(2)(3) "2A" for nonvectored interrupt PC
  //      (1)(2)    "2C" for BPT PC
  //
  //      NOTE 1: wait flag reset if on
  //      NOTE 2: trace flag reset if on
  //      NOTE 3: interrupt enable (I2) reset if on
  //
  //      RESERVRD TRAPS
  //      ... the last 11 floating point OP codes are the only ones that
  //      will cause a reserved OP code trap if executed...
  //
  // --- so, this routine will load up PC from "1C" unless the offending
  // --- opcode is greater than F000 (fmt 11) when it will load from "1A".
  //
  regs.SP -= 2;
  putAMword((unsigned char *)&regs.PS, regs.SP);
  regs.SP -= 2;
  putAMword((unsigned char *)&regs.PC, regs.SP);
  regs.waiting = 0;
  regs.trace = 0;
  regs.PS.I2 = 0;
  if (op > 0xf000)
    getAMword((unsigned char *)&regs.PC, 0x1A);
  else
    getAMword((unsigned char *)&regs.PC, 0x1C);

} /* end function do_fmt_invalid */

/*-------------------------------------------------------------------*/
/* Execute an instruction                                            */
/*-------------------------------------------------------------------*/
void execute_instruction() {
  char fmt;

  regs.instcount++;

  oldPCindex = (oldPCindex + 1) % 256;
  oldPCs[oldPCindex] = opPC = regs.PC;

  getAMword((unsigned char *)&op, regs.PC);
  regs.PC += 2;

  fmt = instruction_type[op];
  switch (fmt) {
  case 0:
    if (regs.tracing) // <<-- here instead of in do_fmt_invalid
      trace_fmtInvalid();
    do_fmt_invalid(); // because other fmts may later call
    break;            // do_fmt_invalid if further decode fails
  case 1:
    do_fmt_1();
    break;
  case 2:
    do_fmt_2();
    break;
  case 3:
    do_fmt_3();
    break;
  case 4:
    do_fmt_4();
    break;
  case 5:
    do_fmt_5();
    break;
  case 6:
    do_fmt_6();
    break;
  case 7:
    do_fmt_7();
    break;
  case 8:
    do_fmt_8();
    break;
  case 9:
    do_fmt_9();
    break;
  case 10:
    do_fmt_10();
    break;
  case 11:
    do_fmt_11();
    break;
  default:
    assert("cpu.c - bad return from inst format lookup");
    do_fmt_invalid();
  } /* end switch(fmt) */

} /* end function execute_instruction */

/*-------------------------------------------------------------------*/
/* Perform interrupt if pending                                      */
/*-------------------------------------------------------------------*/
void perform_interrupt() {
  int i = 0;
  uint16_t tmp;

  if (regs.stepping == 1)
    return;

  while ((regs.whichint[i] == 0) && (i < 9))
    i++;

  if (regs.tracing)
    trace_Interrupt(i);

  pthread_mutex_lock(&intlock_t);

  switch (i) {
  case 0: // non-vectored
    regs.SP -= 2;
    putAMword((unsigned char *)&regs.PS, regs.SP);
    regs.SP -= 2;
    putAMword((unsigned char *)&regs.PC, regs.SP);
    regs.waiting = 0;
    regs.trace = 0;
    regs.PS.I2 = 0;
    getAMword((unsigned char *)&regs.PC, 0x2A); // non-power-fail
    regs.whichint[i] = 0;
    break;
  case 1:
  case 2:
  case 3:
  case 4:
  case 5:
  case 6:
  case 7:
  case 8:
    regs.SP -= 2;
    putAMword((unsigned char *)&regs.PS, regs.SP);
    regs.SP -= 2;
    putAMword((unsigned char *)&regs.PC, regs.SP);
    regs.waiting = 0;
    regs.trace = 0;
    regs.PS.I2 = 0;
    getAMword((unsigned char *)&tmp, 050);
    tmp += (016 - 2 * i);
    getAMword((unsigned char *)&regs.PC, tmp);
    regs.PC += tmp;
    regs.whichint[i] = 0;
    break;
  default:
    assert("cpu.c - invalid interrupt level");
  }

  for (i = 0, regs.intpending = 0; i < 9; i++)
    if (regs.whichint[i] == 1)
      regs.intpending = 1;

  pthread_mutex_unlock(&intlock_t);

} /* end function perform_interrupt */

/*-------------------------------------------------------------------*/
/* CPU instruction execution thread                                  */
/*-------------------------------------------------------------------*/
void cpu_thread() {

  do {
    if (regs.waiting == 0) {
      if ((regs.intpending == 1) && (regs.PS.I2 == 1))
        perform_interrupt();
      execute_instruction();
      if (regs.stepping == 1) {
        regs.waiting = 1;
        regs.stepping = 0;
      }
    } else
      usleep(500);
  } while (regs.halting == 0);
  pthread_exit(0);
} /* end function cpu_thread */

/*-------------------------------------------------------------------*/
/* CPU stop                                                          */
/*-------------------------------------------------------------------*/
void cpu_stop() {
  regs.halting = 1;
  pthread_join(cpu_t, NULL);
}
