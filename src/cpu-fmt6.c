/* cpu-fmt6.c         (c) Copyright Mike Noel, 2001-2008             */
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

#include "cpu-fmt6.h"

#define do_each(opc)                                             \
  if (wd16_cpu_state->regs.tracing)                              \
    wd16_cpu_state->trace_fmt6(opc, count, reg);

void do_fmt_6(wd16_cpu_state_t* wd16_cpu_state) {
  int op6, op6a, op6b, count, reg, tmp, tmp2, reg2, i;

  //      FORMAT 6 OP CODES
  //
  //      SINGLE WORD - SINGLE OPS - SPLIT FIELD - DM0 ONLY
  //
  //      There are 12 op codes in this class representing op codes "0800"
  //      to "09FF", "8800" to "89FF", and "8E00" to "8FFF". There are 4 immedi-
  //      ate mode op codes with a register as a destination, 4 multiple count
  //      single register shifts, and 4 multiple count double register shifts.
  //      In all op codes the actual count (or number in the case of the
  //      immediates) is the value of bits 0 - 3 plus one. Count is always a
  //      positive number in the range 1 - "10", but it is stored in the op code
  //      as 0 - "F". All of these op codes are one word op codes with the op
  //      codes them- selves split between bits 9-15 and 4-5.
  //
  //      In the case of the double shifts the 32 bit number (REG+l):(REG) is
  //      the operand. If REG = PC then (REG+l) = R0.
  //

  count = (wd16_cpu_state->op & 15) + 1;
  op6a = (wd16_cpu_state->op & 63) >> 4;  /* 0, 1, 2, 3 */
  reg = (wd16_cpu_state->op & 511) >> 6;
  op6b = wd16_cpu_state->op >> 9;         /* 4, 68, 71 */
  if (op6b == 68)
    op6b--;               /* 4, 67, 71 */
  op6 = op6b + op6a;      /* 4,5,6,7; 67,68,69,70; 71,72,73,74 */

  switch (op6) {
  case 4:
    //      ADDI            ADD IMMEDIATE
    //      -------------------------------------------------------------
    //      FORMAT:         ADDI NUMBER, REG
    //      OPERATION:      REG <- REG + (COUNT+1)
    //      FUNCTION:       The stored number plus one is added to the
    //                      destination register
    //      INDICATORS:     N = Set if bit 15 of the result is set
    //                      Z = Set if the result = 0
    //                      V = Set if arithmetic overflow occurs; i.e. set
    //                      if both operands were positive and the sign of
    //                      the result is negative
    //                      C = Set if a carry was generated from bit 15
    //                      of the result
    //
    do_each("ADDI");
    tmp = wd16_cpu_state->regs.spr[reg];
    wd16_cpu_state->regs.gpr[reg] += count;
    wd16_cpu_state->regs.PS.N = (wd16_cpu_state->regs.gpr[reg] >> 15) & 1;
    wd16_cpu_state->regs.PS.Z = 0;
    if (wd16_cpu_state->regs.gpr[reg] == 0)
      wd16_cpu_state->regs.PS.Z = 1;
    wd16_cpu_state->regs.PS.V = 0;
    if ((tmp >= 0) && (wd16_cpu_state->regs.spr[reg] < 0))
      wd16_cpu_state->regs.PS.V = 1;
    wd16_cpu_state->regs.PS.C = 0;
    if ((tmp < 0) && (wd16_cpu_state->regs.spr[reg] >= 0))
      wd16_cpu_state->regs.PS.C = 1;
    break;
  case 5:
    //      SUBI            SUBTRACT IMMEDIATE
    //      -------------------------------------------------------------
    //      FORMAT:         SUBI NUMBER, REG
    //      OPERATION:      REG <- REG - (COUNT+1)
    //      FUNCTION:       The stored number plus one is subtracted from
    //                      the destination register.
    //      INDICATORS:     N = Set if bit 15 of the result is set
    //                      Z = Set if the result = 0
    //                      V = Set if arithmetic underflow occurs; i.e. set
    //                      if the operands were of opposite signs and
    //                      the sign of the result is positive
    //                      C = Set if a borrow was generate from bit 15
    //                      of the result
    //
    do_each("SUBI");
    tmp = wd16_cpu_state->regs.spr[reg];
    wd16_cpu_state->regs.gpr[reg] -= count;
    wd16_cpu_state->regs.PS.N = (wd16_cpu_state->regs.gpr[reg] >> 15) & 1;
    wd16_cpu_state->regs.PS.Z = 0;
    if (wd16_cpu_state->regs.gpr[reg] == 0)
      wd16_cpu_state->regs.PS.Z = 1;
    wd16_cpu_state->regs.PS.V = 0;
    if ((tmp < 0) && (wd16_cpu_state->regs.spr[reg] >= 0))
      wd16_cpu_state->regs.PS.V = 1;
    wd16_cpu_state->regs.PS.C = 0;
    if ((tmp >= 0) && (wd16_cpu_state->regs.spr[reg] < 0))
      wd16_cpu_state->regs.PS.C = 1;
    break;
  case 6:
    //      BICI            BIT CLEAR IMMEDIATE
    //      -------------------------------------------------------------
    //      FORMAT:         BICI NUMBER, REG
    //      OPERATION:      REG <- REG AND (COMPLEMENT(COUNT + 1))
    //      FUNCTION:       The stored number plus one is one!s complemented
    //                      and ANDED to the destination register
    //      INDICATORS:     N = Set if bit 15 of the result is set
    //                      Z = Set if the result = 0
    //                      V = Reset
    //                      C = Unchanged
    //
    do_each("BICI");
    wd16_cpu_state->regs.gpr[reg] &= ~count;
    wd16_cpu_state->regs.PS.N = (wd16_cpu_state->regs.gpr[reg] >> 15) & 1;
    wd16_cpu_state->regs.PS.Z = 0;
    if (wd16_cpu_state->regs.gpr[reg] == 0)
      wd16_cpu_state->regs.PS.Z = 1;
    wd16_cpu_state->regs.PS.V = 0;
    break;
  case 7:
    //      MOVI            MOVE IMMEDIATE
    //      -------------------------------------------------------------
    //      FORMAT:         MOVI NUMBER, REG
    //      OPERATION:      REG <- (COUNT+ 1)
    //      FUNCTION:       The stored number plus one is placed in
    //                      the destination register
    //      INDICATORS:     N = Reset
    //                      Z = Reset
    //                      v = Reset
    //                      C = Unchanged
    //
    do_each("MOVI");
    wd16_cpu_state->regs.gpr[reg] = count;
    wd16_cpu_state->regs.PS.N = 0;
    wd16_cpu_state->regs.PS.Z = 0;
    wd16_cpu_state->regs.PS.V = 0;
    break;
  case 67:
    //      SSRR            SHIFT SINGLE RIGHT ROTATE
    //      -------------------------------------------------------------
    //      FORMAT:         SSRR REG, COUNT
    //      FUNCTION:       A 17-bit right rotate is done stored count+1
    //                      times on REG:C-Flag. The C-Flag is shifted into
    //                      bit 15 of REG, aud the C-Flag gets the last bit
    //                      shifted out of REG bit 0.
    //      INDICATORS:     N = Set if bit 7 of REG is set
    //                      Z = Set if REG = 0
    //                      V = Set to exclusive or of N and C flags
    //                      C = Set to the value of the last bit shifted
    //                      out of REG bit 0
    //
    // NOTE - comments (and expected result) of 'cpu' diagnostic test 271 are
    //        not consistant with V = exclusive-or of N and C flags.
    //   "TSTCC 10010     ;shift C into 15, set N, clear C, set V **"
    //
    do_each("SSRR");
    for (i = 0; i < count; i++) {
      tmp = wd16_cpu_state->regs.gpr[reg] & 1;
      wd16_cpu_state->regs.gpr[reg] = wd16_cpu_state->regs.gpr[reg] >> 1;
      if (wd16_cpu_state->regs.PS.C == 1)
        wd16_cpu_state->regs.gpr[reg] |= 0x8000;
      wd16_cpu_state->regs.PS.C = tmp;
    }
    wd16_cpu_state->regs.PS.N = (wd16_cpu_state->regs.gpr[reg] >> 7) & 1;
    wd16_cpu_state->regs.PS.Z = 0;
    if (wd16_cpu_state->regs.gpr[reg] == 0)
      wd16_cpu_state->regs.PS.Z = 1;
    wd16_cpu_state->regs.PS.V = wd16_cpu_state->regs.PS.C ^ wd16_cpu_state->regs.PS.N;
    /* ??? */ wd16_cpu_state->regs.PS.V = 0;
    break;
  case 68:
    //      SSLR            SHIFT SINGLE LEFT ROTATE
    //      -------------------------------------------------------------
    //      FORMAT:         SSLR REG, COUNT
    //      FUNCTION:       A 17-bit left rotate is done stored count+1
    //                      times on C-Flag:REG . The C-Flag is shifted
    //                      into bit 0 of REG and the C-Flag gets the
    //                      last bit shifted out of REG bit 15.
    //      INDICATORS:     N = Set if bit 15 of REG is set
    //                      Z = Set if REG = 0
    //                      V = Set to exclusive or of N and C flags
    //                      C = Set to the value of the last bit shifted
    //                      out of REG bit 15.
    //
    do_each("SSLR");
    for (i = 0; i < count; i++) {
      tmp = (wd16_cpu_state->regs.gpr[reg] >> 15) & 1;
      wd16_cpu_state->regs.gpr[reg] = wd16_cpu_state->regs.gpr[reg] << 1;
      if (wd16_cpu_state->regs.PS.C == 1)
        wd16_cpu_state->regs.gpr[reg] |= 1;
      wd16_cpu_state->regs.PS.C = tmp;
    }
    wd16_cpu_state->regs.PS.N = (wd16_cpu_state->regs.gpr[reg] >> 15) & 1;
    wd16_cpu_state->regs.PS.Z = 0;
    if (wd16_cpu_state->regs.gpr[reg] == 0)
      wd16_cpu_state->regs.PS.Z = 1;
    wd16_cpu_state->regs.PS.V = wd16_cpu_state->regs.PS.C ^ wd16_cpu_state->regs.PS.N;
    break;
  case 69:
    //      SSRA            SHIFT SINGLE RIGHT ARITHMETIC
    //      -------------------------------------------------------------
    //      FORMAT:         SSRA REG, COUNT
    //      FUNCTION:       A 17-bit right arithmetic shift is done
    //                      stored count+l times on REG:C-Flag. Bit
    //                      15 of REG is replicated. The C-Flag gets the
    //                      last bit shifted out of REG bit 0. Bits shifted
    //                      out of the C-Flag are lost.
    //      INDICATORS:     N = Set if bit 7 of REG is set
    //                      Z = Set if REG = 0
    //                      V = Set to exclusive or of N and C flags
    //                      C = Set to the value of the last bit shifted
    //                      out of REG bit 0
    //
    // NOTE - comments (and expected result) of 'cpu' diagnostic test 303 and
    //        306 are not consistant with V = exclusive-or of N and C flags.
    //   "TSTCC 10001     ;** V should be set"
    //   "TSTCC 10010     ;** V should be set"
    //
    do_each("SSRA");
    for (i = 0; i < count; i++) {
      tmp = (wd16_cpu_state->regs.gpr[reg] >> 15) & 1;
      wd16_cpu_state->regs.PS.C = wd16_cpu_state->regs.gpr[reg] & 1;
      wd16_cpu_state->regs.gpr[reg] = wd16_cpu_state->regs.gpr[reg] >> 1;
      if (tmp == 1)
        wd16_cpu_state->regs.gpr[reg] |= 0x8000;
    }
    wd16_cpu_state->regs.PS.N = (wd16_cpu_state->regs.gpr[reg] >> 7) & 1;
    wd16_cpu_state->regs.PS.Z = 0;
    if (wd16_cpu_state->regs.gpr[reg] == 0)
      wd16_cpu_state->regs.PS.Z = 1;
    wd16_cpu_state->regs.PS.V = wd16_cpu_state->regs.PS.C ^ wd16_cpu_state->regs.PS.N;
    /* ??? */ wd16_cpu_state->regs.PS.V = 0;
    break;
  case 70:
    //      SSLA            SHIFT SINGLE LEFT ARITHMETIC
    //      -------------------------------------------------------------
    //      FORMAT:         SSLA REG, COUNT
    //      FUNCTION:       A l7-bit left arithmetic shift is done stored
    //                      count+l times on C-Flag:REG. Zeros are shifted
    //                      into REG bit 0, and the C-FLAG gets the last bit
    //                      shifted out of REG bit 15. Bits shifted out of the
    //                      C-Flag are lost
    //      INDICATORS:     N = Set if REG bit 15 is set
    //                      Z = Set if REG = 0
    //                      V = Set to exclusive or of N and C flags
    //                      C - Set to the value of the last bit shifted
    //                      out of REG bit 15
    //
    do_each("SSLA");
    for (i = 0; i < count; i++) {
      wd16_cpu_state->regs.PS.C = (wd16_cpu_state->regs.gpr[reg] >> 15) & 1;
      wd16_cpu_state->regs.gpr[reg] = wd16_cpu_state->regs.gpr[reg] << 1;
    }
    wd16_cpu_state->regs.PS.N = (wd16_cpu_state->regs.gpr[reg] >> 15) & 1;
    wd16_cpu_state->regs.PS.Z = 0;
    if (wd16_cpu_state->regs.gpr[reg] == 0)
      wd16_cpu_state->regs.PS.Z = 1;
    wd16_cpu_state->regs.PS.V = wd16_cpu_state->regs.PS.C ^ wd16_cpu_state->regs.PS.N;
    break;
  case 71:
    //      SDRR            SHIFT DOUBLE RIGHT ROTATE
    //      -------------------------------------------------------------
    //      FORMAT:         SDRR REG, COUNT
    //      FUNCTION:       REG+l:REG:C-Flag is rotate right stored
    //                      count+1 times. The C-Flag is shifted into
    //                      REG+l bit 15, REG+1 bit 0 is shifted into
    //                      REG bit 15, and REG bit 0 is shifted into the
    //                      C-Flag.
    //      INDICATORS:     N = Set if bit 7 of REG is set
    //                      Z = Set if REG = 0
    //                      V = Set to exclusive or of N and C flags
    //                      C = Set to the value of the last bit shifted
    //                      out of REG bit 0
    //
    // NOTE - comments (and expected result) of 'cpu' diagnostic test 314 are
    //        not consistant with V = exclusive-or of N and C flags.
    //   "TSTCC 10010     ;** V should be set"
    //
    do_each("SDRR");
    reg2 = (reg + 1) % 8;
    for (i = 0; i < count; i++) {
      tmp = wd16_cpu_state->regs.gpr[reg] & 1;
      tmp2 = wd16_cpu_state->regs.gpr[reg2] & 1;
      wd16_cpu_state->regs.gpr[reg] = wd16_cpu_state->regs.gpr[reg] >> 1;
      wd16_cpu_state->regs.gpr[reg2] = wd16_cpu_state->regs.gpr[reg2] >> 1;
      if (wd16_cpu_state->regs.PS.C == 1)
        wd16_cpu_state->regs.gpr[reg2] |= 0x8000;
      if (tmp2 == 1)
        wd16_cpu_state->regs.gpr[reg] |= 0x8000;
      wd16_cpu_state->regs.PS.C = tmp;
    }
    wd16_cpu_state->regs.PS.N = (wd16_cpu_state->regs.gpr[reg] >> 7) & 1;
    wd16_cpu_state->regs.PS.Z = 0;
    if (wd16_cpu_state->regs.gpr[reg] == 0)
      wd16_cpu_state->regs.PS.Z = 1;
    wd16_cpu_state->regs.PS.V = wd16_cpu_state->regs.PS.C ^ wd16_cpu_state->regs.PS.N;
    /* ??? */ wd16_cpu_state->regs.PS.V = 0;
    break;
  case 72:
    //      SDLR            SHIFT DOUBLE LEFT ROTATE
    //      -------------------------------------------------------------
    //      FORMAT:         SDLR RRG, COUNT
    //      FUNCTION:       A 33 bit left rotate is done stored couNT+1
    //                      times on C-Flag:REG+l:RRG. The C-Flag is
    //                      shifted into REG bit 0, REG bit 15 is shifted
    //                      into REG+l bit 0, and REG+l bit 15 is shifted
    //                      into the C-Flag
    //      INDICATORS:     N - Set if REG+l bit 15 is set
    //                      Z = Set if REG+l = 0
    //                      V = Set to exclusive or of N and C. flags
    //                      C = Set to the value of the last bit shifted
    //                      out of REG+l bit 15.
    //
    do_each("SDLR");
    reg2 = (reg + 1) % 8;
    for (i = 0; i < count; i++) {
      tmp = (wd16_cpu_state->regs.gpr[reg] >> 15) & 1;
      tmp2 = (wd16_cpu_state->regs.gpr[reg2] >> 15) & 1;
      wd16_cpu_state->regs.gpr[reg] = wd16_cpu_state->regs.gpr[reg] << 1;
      wd16_cpu_state->regs.gpr[reg2] = wd16_cpu_state->regs.gpr[reg2] << 1;
      if (wd16_cpu_state->regs.PS.C == 1)
        wd16_cpu_state->regs.gpr[reg] |= 1;
      if (tmp == 1)
        wd16_cpu_state->regs.gpr[reg2] |= 1;
      wd16_cpu_state->regs.PS.C = tmp2;
    }
    wd16_cpu_state->regs.PS.N = (wd16_cpu_state->regs.gpr[reg2] >> 15) & 1;
    wd16_cpu_state->regs.PS.Z = 0;
    if (wd16_cpu_state->regs.gpr[reg2] == 0)
      wd16_cpu_state->regs.PS.Z = 1;
    wd16_cpu_state->regs.PS.V = wd16_cpu_state->regs.PS.C ^ wd16_cpu_state->regs.PS.N;
    break;
  case 73:
    //      SDRA            SHIFT DOUBLE RIGHT ARITHMETIC
    //      -------------------------------------------------------------
    //      FORMAT:         SDRA REG, COUNT
    //      FUNCTION:       A right arithmetic shift is done stored
    //                      count+1 times on REG+1:REG:C-Flag,
    //                      Bit 15 of REG+1 is replicated. Bit 0 of
    //                      REG+1 is shifted to bit 15 of REG. Bit
    //                      0 of REG is shifted to the C-Flag. Bits
    //                      shifted out of the C-Flag are lost.
    //      INDICATORS:     N = Set if bit 7 of REG is set
    //                      Z = Set if REG = 0
    //                      V = Set to exclusive or of N and C flags
    //                      C = Set to the value of the last bit
    //                      shifted out of REG bit 0
    //
    // NOTE - comments (and expected result) of 'cpu' diagnostic test 326 and
    //        331 are not consistant with V = exclusive-or of N and C flags.
    //   "TSTCC 10010     ;** V should be set"
    //   "TSTCC 10001     ;** V should be set"
    //
    do_each("SDRA");
    reg2 = (reg + 1) % 8;
    for (i = 0; i < count; i++) {
      wd16_cpu_state->regs.PS.C = wd16_cpu_state->regs.gpr[reg] & 1;
      tmp = wd16_cpu_state->regs.gpr[reg2] & 1;
      tmp2 = (wd16_cpu_state->regs.gpr[reg2] >> 15) & 1;
      wd16_cpu_state->regs.gpr[reg] = wd16_cpu_state->regs.gpr[reg] >> 1;
      wd16_cpu_state->regs.gpr[reg2] = wd16_cpu_state->regs.gpr[reg2] >> 1;
      if (tmp == 1)
        wd16_cpu_state->regs.gpr[reg] |= 0x8000;
      if (tmp2 == 1)
        wd16_cpu_state->regs.gpr[reg2] |= 0x8000;
    }
    wd16_cpu_state->regs.PS.N = (wd16_cpu_state->regs.gpr[reg] >> 7) & 1;
    wd16_cpu_state->regs.PS.Z = 0;
    if (wd16_cpu_state->regs.gpr[reg] == 0)
      wd16_cpu_state->regs.PS.Z = 1;
    wd16_cpu_state->regs.PS.V = wd16_cpu_state->regs.PS.C ^ wd16_cpu_state->regs.PS.N;
    /* ??? */ wd16_cpu_state->regs.PS.V = 0;
    break;
  case 74:
    //      SDLA            SHIFT DOUBLE LEFT ARITHMETIC
    //      -------------------------------------------------------------
    //      FORMAT:         SDLA REG, COUNT
    //      FUNCTION:       A left arithmetic shift is done stored
    //                      count+1 times on C-Flag:REG+l:REG.
    //                      Zeros are shifted into REG bit 0, REG bit
    //                      15 is shifted to REG+l bit 0. REG+l
    //                      bit 15 is shifted to the C-Flag. Bits
    //                      shifted out of the C-Flag are lost.
    //      INDICATORS:     N = Set if REG+l bit 15 is set
    //                      Z = Set if REG+l = 0
    //                      V = Set to exclusive or.of N and C flags
    //                      C = Set to the value of the last bit shifted
    //                      out of REG+l bit 15
    //
    do_each("SDLA");
    reg2 = (reg + 1) % 8;
    for (i = 0; i < count; i++) {
      wd16_cpu_state->regs.PS.C = (wd16_cpu_state->regs.gpr[reg2] >> 15) & 1;
      tmp = (wd16_cpu_state->regs.gpr[reg] >> 15) & 1;
      wd16_cpu_state->regs.gpr[reg] = wd16_cpu_state->regs.gpr[reg] << 1;
      wd16_cpu_state->regs.gpr[reg2] = wd16_cpu_state->regs.gpr[reg2] << 1;
      if (tmp == 1)
        wd16_cpu_state->regs.gpr[reg2] |= 1;
    }
    wd16_cpu_state->regs.PS.N = (wd16_cpu_state->regs.gpr[reg2] >> 15) & 1;
    wd16_cpu_state->regs.PS.Z = 0;
    if (wd16_cpu_state->regs.gpr[reg2] == 0)
      wd16_cpu_state->regs.PS.Z = 1;
    wd16_cpu_state->regs.PS.V = wd16_cpu_state->regs.PS.C ^ wd16_cpu_state->regs.PS.N;
    break;
  default:
    assert("cp-fmt6.c - invalid return from fmt_6 lookup");
    do_fmt_invalid();
  } /* end switch(op6) */

} /* end function do_fmt_6 */
