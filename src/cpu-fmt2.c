/* cpu-fmt2.c         (c) Copyright Mike Noel, 2001-2008             */
/* ----------------------------------------------------------------- */
/*                                                                   */
/* This software is an emulator for the Alpha-Micro AM-100 computer. */
/* It is copyright by Michael Noel and licensed for non-commercial   */
/* hobbyist use under terms of the "Q public license", an open         */
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

#include "cpu-fmt2.h"

#define do_each(opc)                                                           \
  if (wd11_cpu_state->regs.tracing)                                                            \
    trace_fmt2(opc, reg);

void do_fmt_2(wd11_cpu_state_t* wd11_cpu_state) {
  int op2, reg;
  uint16_t tmp;

  //      FORMAT 2 OP CODES
  //
  //      SINGLE WORD - 3 BIT REGISTER ARGUMENT
  //
  //      There are 4 op codes in this class representing op codes "0010"
  //      to "002F". Each is a one word op code with a single 3 - bit register
  //      argument.
  //

  reg = op & 7;
  op2 = op >> 3;

  switch (op2) {
  case 2:
    //      IAC             INTERRUPT ACKNOWLEDGE
    //      -------------------------------------------------------------
    //      FORMAT:         IAK REG
    //      FUNCTION:       An interrupt acknowledge (READ and IACK) is
    //                      executed, and the 16 bit code that is returned
    //                      is placed in REG unmodified. Used with the
    //                      nonvectored interrupt when the user does
    //                      not wish to use the vectored format.
    //      INDICATORS:     Unchanged
    //
    do_each("IAC");
    // ??? interrrupt acknowledge ???
    wd11_cpu_state->regs.gpr[reg] = 0;
    wd11_cpu_state->regs.PS.I2 = 0;
    break;
  case 3:
    //      RTN             RETURN FROM SUBROUTINE
    //      -------------------------------------------------------------
    //      FORMAT:         RTN REG
    //      OPERATION:      PC  <- REG
    //                      REG <- @SP, SP^
    //      FUNCTION:       The linkage register is placed in PC and the
    //                      saved linkage register is popped from the stack.
    //                      The register used must be the same one that was
    //                      used for the subroutine call.
    //      INDICATORS:     Unchanged
    //
    do_each("RTN");
    wd11_cpu_state->regs.PC = wd11_cpu_state->regs.gpr[reg];
    getAMword((unsigned char *)&wd11_cpu_state->regs.gpr[reg], wd11_cpu_state->regs.SP);
    wd11_cpu_state->regs.SP += 2;
    break;
  case 4:
    //      MSKO            MASK OUT
    //      -------------------------------------------------------------
    //      FORMAT:         MSKO REG
    //      OPERATION:      (loc "2E") <- REG
    //                      MSKO
    //      FUNCTION:       The contents of REG are written into location
    //                      "2E" and a MASK OUT state code (see appendix D)
    //                      is transmitted.
    //      INDICTORS:      Unchanged
    //
    do_each("MSKO");
    putAMword((unsigned char *)&wd11_cpu_state->regs.gpr[reg], 0x2E);
    // ??? mask out ???
    break;
  case 5:
    //      PRTN            POP STACK AND RETURN
    //      -------------------------------------------------------------
    //      FORMAT:         PRTN REG
    //      OPERATION:      TMP <- @SP
    //                      SP  <- SP+(TMP*2)
    //                      RNT REG
    //      FUNCTION:       Twice the value of the top word on
    //                      the stack is added to SP, and a standard
    //                      RTN call is then executed.
    //      INDICATORS:     unchanged
    //
    do_each("PRTN");
    getAMword((unsigned char *)&tmp, wd11_cpu_state->regs.SP);
    wd11_cpu_state->regs.SP += 2 * tmp;
    wd11_cpu_state->regs.PC = wd11_cpu_state->regs.gpr[reg];
    getAMword((unsigned char *)&wd11_cpu_state->regs.gpr[reg], wd11_cpu_state->regs.SP);
    wd11_cpu_state->regs.SP += 2;
    break;
  default:
    assert("cpu-fmt2.c - invalid return from fmt_2 lookup");
    do_fmt_invalid();
  } /* end switch(op2) */

} /* end function do_fmt_2 */
