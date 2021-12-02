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
#include "instruction-type.h"

wd11_cpu_state_t wd11_cpu_state;

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
  wd11_cpu_state.regs.SP -= 2;
  putAMword((unsigned char *)&wd11_cpu_state.regs.PS, wd11_cpu_state.regs.SP);
  wd11_cpu_state.regs.SP -= 2;
  putAMword((unsigned char *)&wd11_cpu_state.regs.PC, wd11_cpu_state.regs.SP);
  wd11_cpu_state.regs.waiting = 0;
  wd11_cpu_state.regs.trace = 0;
  wd11_cpu_state.regs.PS.I2 = 0;
  if (wd11_cpu_state.op > 0xf000)
    getAMword((unsigned char *)&wd11_cpu_state.regs.PC, 0x1A);
  else
    getAMword((unsigned char *)&wd11_cpu_state.regs.PC, 0x1C);

} /* end function do_fmt_invalid */

/*-------------------------------------------------------------------*/
/* Execute an instruction                                            */
/*-------------------------------------------------------------------*/
void execute_instruction() {
  char fmt;

  wd11_cpu_state.regs.instcount++;

  wd11_cpu_state.oldPCindex = (wd11_cpu_state.oldPCindex + 1) % 256;
  wd11_cpu_state.oldPCs[wd11_cpu_state.oldPCindex] = wd11_cpu_state.opPC = wd11_cpu_state.regs.PC;

  getAMword((unsigned char *)&wd11_cpu_state.op, wd11_cpu_state.regs.PC);
  wd11_cpu_state.regs.PC += 2;

  fmt = instruction_type(op);
  switch (fmt) {
  case 0:
    if (wd11_cpu_state.regs.tracing) // <<-- here instead of in do_fmt_invalid
      trace_fmtInvalid();
    do_fmt_invalid(); // because other fmts may later call
    break;            // do_fmt_invalid if further decode fails
  case 1:
    do_fmt_1(&wd11_cpu_state);
    break;
  case 2:
    do_fmt_2(&wd11_cpu_state);
    break;
  case 3:
    do_fmt_3(&wd11_cpu_state);
    break;
  case 4:
    do_fmt_4(&wd11_cpu_state);
    break;
  case 5:
    do_fmt_5(&wd11_cpu_state);
    break;
  case 6:
    do_fmt_6(&wd11_cpu_state);
    break;
  case 7:
    do_fmt_7(&wd11_cpu_state);
    break;
  case 8:
    do_fmt_8(&wd11_cpu_state);
    break;
  case 9:
    do_fmt_9(&wd11_cpu_state);
    break;
  case 10:
    do_fmt_10(&wd11_cpu_state);
    break;
  case 11:
    do_fmt_11(&wd11_cpu_state);
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

  if (wd11_cpu_state.regs.stepping == 1)
    return;

  while ((wd11_cpu_state.regs.whichint[i] == 0) && (i < 9))
    i++;

  if (wd11_cpu_state.regs.tracing)
    trace_Interrupt(i);

  pthread_mutex_lock(&intlock_t);

  switch (i) {
  case 0: // non-vectored
    wd11_cpu_state.regs.SP -= 2;
    putAMword((unsigned char *)&wd11_cpu_state.regs.PS, wd11_cpu_state.regs.SP);
    wd11_cpu_state.regs.SP -= 2;
    putAMword((unsigned char *)&wd11_cpu_state.regs.PC, wd11_cpu_state.regs.SP);
    wd11_cpu_state.regs.waiting = 0;
    wd11_cpu_state.regs.trace = 0;
    wd11_cpu_state.regs.PS.I2 = 0;
    getAMword((unsigned char *)&wd11_cpu_state.regs.PC, 0x2A); // non-power-fail
    wd11_cpu_state.regs.whichint[i] = 0;
    break;
  case 1:
  case 2:
  case 3:
  case 4:
  case 5:
  case 6:
  case 7:
  case 8:
    wd11_cpu_state.regs.SP -= 2;
    putAMword((unsigned char *)&wd11_cpu_state.regs.PS, wd11_cpu_state.regs.SP);
    wd11_cpu_state.regs.SP -= 2;
    putAMword((unsigned char *)&wd11_cpu_state.regs.PC, wd11_cpu_state.regs.SP);
    wd11_cpu_state.regs.waiting = 0;
    wd11_cpu_state.regs.trace = 0;
    wd11_cpu_state.regs.PS.I2 = 0;
    getAMword((unsigned char *)&tmp, 050);
    tmp += (016 - 2 * i);
    getAMword((unsigned char *)&wd11_cpu_state.regs.PC, tmp);
    wd11_cpu_state.regs.PC += tmp;
    wd11_cpu_state.regs.whichint[i] = 0;
    break;
  default:
    assert("cpu.c - invalid interrupt level");
  }

  for (i = 0, wd11_cpu_state.regs.intpending = 0; i < 9; i++)
    if (wd11_cpu_state.regs.whichint[i] == 1)
      wd11_cpu_state.regs.intpending = 1;

  pthread_mutex_unlock(&intlock_t);

} /* end function perform_interrupt */

/*-------------------------------------------------------------------*/
/* CPU instruction execution thread                                  */
/*-------------------------------------------------------------------*/
void cpu_thread() {

  do {
    if (wd11_cpu_state.regs.waiting == 0) {
      if ((wd11_cpu_state.regs.intpending == 1) && (wd11_cpu_state.regs.PS.I2 == 1))
        perform_interrupt();
      execute_instruction();
      if (wd11_cpu_state.regs.stepping == 1) {
        wd11_cpu_state.regs.waiting = 1;
        wd11_cpu_state.regs.stepping = 0;
      }
    } else
      usleep(500);
  } while (wd11_cpu_state.regs.halting == 0);
  pthread_exit(0);
} /* end function cpu_thread */

/*-------------------------------------------------------------------*/
/* CPU stop                                                          */
/*-------------------------------------------------------------------*/
void cpu_stop() {
  wd11_cpu_state.regs.halting = 1;
  pthread_join(cpu_t, NULL);
}
