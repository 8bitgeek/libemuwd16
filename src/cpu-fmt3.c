/* cpu-fmt3.c         (c) Copyright Mike Noel, 2001-2008             */
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

#include "cpu-fmt3.h"

#define do_each(opc)                                                           \
  if (wd11_cpu_state->regs.tracing)                                                            \
    wd11_cpu_state->trace_fmt3(opc, arg);

void do_fmt_3(wd11_cpu_state_t* wd11_cpu_state) {
  int op3, arg;

  //      FORMAT 3 OP CODES
  //
  //      SINGLE WORD - 4 BIT NUMERIC ARGUMENT
  //
  //      There is only one op code in this class representing op codes
  //      "0030" to "003F". It is a one word op code with a 4 bit numeric
  //      argument.
  //

  arg = wd11_cpu_state->op & 15;
  op3 = wd11_cpu_state->op >> 4;

  switch (op3) {
  case 3:
    //      LCC             LOAD CONDITION CODE
    //      -------------------------------------------------------------
    //      FORMAT:         LCC ARG
    //      FUNCTION:       The 4 indicators are loaded from bits 0-3
    //                      of the op code as specified.
    //      INDICATORS:     N = set per bit 3 of op code
    //                      Z = set per bit 2 of op code
    //                      V = set per bit 1 of op code
    //                      C = set per bit 0 of op code
    //
    do_each("LCC");
    wd11_cpu_state->regs.PS.N = (arg >> 3) & 1;
    wd11_cpu_state->regs.PS.Z = (arg >> 2) & 1;
    wd11_cpu_state->regs.PS.V = (arg >> 1) & 1;
    wd11_cpu_state->regs.PS.C = (arg >> 0) & 1;
    break;
  default:
    assert("cpu-fmt3.c - invalid return from fmt_3 lookup");
    do_fmt_invalid();
  } /* end switch(op3) */

} /* end function do_fmt_3 */
