/* cpu-fmt5.c         (c) Copyright Mike Noel, 2001-2008             */
/*                              and Miranda Noel                     */
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

#include "cpu-fmt5.h"

#define do_each(opc)                                                           \
  if (wd11_cpu_state->regs.tracing)                                                            \
    trace_fmt5(opc, dest);

void do_fmt_5(wd11_cpu_state_t* wd11_cpu_state) {
  int op5, dest;

  //      FORMAT 5 OP CODES
  //
  //      SINGLE WORD - 8 BIT SIGNED NUMERIC ARGUMENT
  //
  //      There are 15 op codes in this class representing op codes
  //      "0100" to "07FF" and "8000" to "87FF". All are branches with a
  //      signed 8 bit displacement that represents the word offset from PC
  //      (which points to the op code that follows) to the desired branch
  //      location. The op codes consist on one unconditional branch, 8
  //      signed conditional branches, and 6 unsigned conditional branches.
  //      No op code in this class modifies any of the indicator flags. Maximum
  //      branch range is +128, -127 words from the branch op code.
  //

  dest = wd11_cpu_state->op & 255;
  if (dest > 127)
    dest = dest - 256;
  op5 = wd11_cpu_state->op >> 8;

  switch (op5) {
  case 1:
    //      BR              BRANCH UNCONDITIONALLY
    //      -------------------------------------------------------------
    //      FORMAT:         BR DEST
    //      OPERATION:      PC <- PC + (DISP *2)
    //      FUNCTION:       Twice the value of the signed displacement
    //                      is added to PC.
    //
    do_each("BR");
    wd11_cpu_state->regs.PC = wd11_cpu_state->regs.PC + (dest * 2);
    break;
  case 2:
    //      BNE             BRANCH IF NOT EQUAL TO ZERO
    //      -------------------------------------------------------------
    //      FORMAT:         BNE DEST
    //      OPERATION:      IF Z = 0, PC <- PC + (DISP *2)
    //
    do_each("BNE");
    if (wd11_cpu_state->regs.PS.Z == 0)
      wd11_cpu_state->regs.PC = wd11_cpu_state->regs.PC + (dest * 2);
    break;
  case 3:
    //      BEQ             BRANCH IF EQUAL TO ZERO
    //      -------------------------------------------------------------
    //      FORMAT:         BEQ DEST
    //      OPERATION:      IF Z = 1, PC <- PC + (DISP *2)
    //
    do_each("BEQ");
    if (wd11_cpu_state->regs.PS.Z == 1)
      wd11_cpu_state->regs.PC = wd11_cpu_state->regs.PC + (dest * 2);
    break;
  case 4:
    //      BGE             BRANCH IF GREATER THAN OR EQUAL TO ZERO
    //      -------------------------------------------------------------
    //      FORMAT:         BGE DEST
    //      OPERATION:      IF N xor V = 0, PC <- PC + (DISP *2)
    //
    do_each("BGE");
    if ((wd11_cpu_state->regs.PS.N ^ wd11_cpu_state->regs.PS.V) == 0)
      wd11_cpu_state->regs.PC = wd11_cpu_state->regs.PC + (dest * 2);
    break;
  case 5:
    //      BLT             BRANCH IF LESS THAN ZERO
    //      -------------------------------------------------------------
    //      FORMAT:         BLT DEST
    //      OPERATION:      IF N xor V = 1, PC <- PC + (DISP *2)
    //
    do_each("BLT");
    if ((wd11_cpu_state->regs.PS.N ^ wd11_cpu_state->regs.PS.V) == 1)
      wd11_cpu_state->regs.PC = wd11_cpu_state->regs.PC + (dest * 2);
    break;
  case 6:
    //      BGT             BRANCH IF GREATER THAN ZERO
    //      -------------------------------------------------------------
    //      FORMAT:         BGT DEST
    //      OPERATION:      IF Z or (N xor V) = 0, PC <- PC + (DISP *2)
    //
    do_each("BGT");
    if ((wd11_cpu_state->regs.PS.Z | (wd11_cpu_state->regs.PS.N ^ wd11_cpu_state->regs.PS.V)) == 0)
      wd11_cpu_state->regs.PC = wd11_cpu_state->regs.PC + (dest * 2);
    break;
  case 7:
    //      BLE             BRANCH IF LESS THAN or EQUAL TO ZERO
    //      -------------------------------------------------------------
    //      FORMAT:         BLE DEST
    //      OPERATION:      IF Z or (N xor V) = 1, PC <- PC + (DISP *2)
    //
    do_each("BLE");
    if ((wd11_cpu_state->regs.PS.Z | (wd11_cpu_state->regs.PS.N ^ wd11_cpu_state->regs.PS.V)) == 1)
      wd11_cpu_state->regs.PC = wd11_cpu_state->regs.PC + (dest * 2);
    break;
  case 128:
    //      BPL             BRANCH IF PLUS
    //      -------------------------------------------------------------
    //      FORMAT:         BPL DEST
    //      OPERATION:      IF N = 0, PC <- PC + (DISP *2)
    //
    do_each("BPL");
    if (wd11_cpu_state->regs.PS.N == 0)
      wd11_cpu_state->regs.PC = wd11_cpu_state->regs.PC + (dest * 2);
    break;
  case 129:
    //      BMI             BRANCH IF MINUS
    //      -------------------------------------------------------------
    //      FORMAT:         BMI DEST
    //      OPERATION:      IF N = 1, PC <- PC + (DISP *2)
    //
    do_each("BMI");
    if (wd11_cpu_state->regs.PS.N == 1)
      wd11_cpu_state->regs.PC = wd11_cpu_state->regs.PC + (dest * 2);
    break;
  case 130:
    //      BHI             BRANCH IF HIGHER
    //      -------------------------------------------------------------
    //      FORMAT:         BHI DEST
    //      OPERATION:      IF C or Z = 0, PC <- PC + (DISP *2)
    //
    do_each("BHI");
    if ((wd11_cpu_state->regs.PS.C | wd11_cpu_state->regs.PS.Z) == 0)
      wd11_cpu_state->regs.PC = wd11_cpu_state->regs.PC + (dest * 2);
    break;
  case 131:
    //      BLOS            BRANCH IF LOWER OR SAME
    //      -------------------------------------------------------------
    //      FORMAT:         BLOS DEST
    //      OPERATION:      IF C or Z = 1, PC <- PC + (DISP *2)
    //
    do_each("BLOS");
    if ((wd11_cpu_state->regs.PS.C | wd11_cpu_state->regs.PS.Z) == 1)
      wd11_cpu_state->regs.PC = wd11_cpu_state->regs.PC + (dest * 2);
    break;
  case 132:
    //      BVC             BRANCH IF OVERFLOW CLEAR
    //      -------------------------------------------------------------
    //      FORMAT:         BVC DEST
    //      OPERATION:      IF V = 0, PC <- PC + (DISP *2)
    //
    do_each("BVC");
    if (wd11_cpu_state->regs.PS.V == 0)
      wd11_cpu_state->regs.PC = wd11_cpu_state->regs.PC + (dest * 2);
    break;
  case 133:
    //      BVS             BRANCH IF OVERFLOW SET
    //      -------------------------------------------------------------
    //      FORMAT:         BVS DEST
    //      OPERATION:      IF V = 1, PC <- PC + (DISP *2)
    //
    do_each("BVS");
    if (wd11_cpu_state->regs.PS.V == 1)
      wd11_cpu_state->regs.PC = wd11_cpu_state->regs.PC + (dest * 2);
    break;
  case 134:
    //      BCC             BRANCH IF CARRY CLEAR
    //      BHIS            BRANCH IF HIGHER OR SAME
    //      -------------------------------------------------------------
    //      FORMAT:         BCC  DEST
    //                      BHIS DEST
    //      OPERATION:      IF C = 0, PC <- PC + (DISP *2)
    //
    do_each("BCC");
    if (wd11_cpu_state->regs.PS.C == 0)
      wd11_cpu_state->regs.PC = wd11_cpu_state->regs.PC + (dest * 2);
    break;
  case 135:
    //      BCS             BRANCH IF CARRY SET
    //      BLO             BRANCH IF LOWER
    //      -------------------------------------------------------------
    //      FORMAT:         BCS DEST
    //                      BLO DEST
    //      OPERATION:      IF C = 1, PC <- PC + (DISP *2)
    //
    do_each("BCS");
    if (wd11_cpu_state->regs.PS.C == 1)
      wd11_cpu_state->regs.PC = wd11_cpu_state->regs.PC + (dest * 2);
    break;
  default:
    assert("cpu-fmt5.c - invalid return from fmt_5 lookup");
    do_fmt_invalid();
  } /* end switch(op5) */

} /* end function do_fmt_5 */
