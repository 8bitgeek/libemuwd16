/* cpu-fmt10.c         (c) Copyright Mike Noel, 2001-2008            */
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

#include "cpu-fmt10.h"

#define do_each(opc)                                                           \
  if (wd11_cpu_state->regs.tracing)                                                            \
    trace_fmt10(opc, smode, sreg, dmode, dreg, n1word);

void do_fmt_10(wd11_cpu_state_t* wd11_cpu_state) {
  int op10, smode, sreg, dmode, dreg, itmp;
  uint16_t tmp, tmp2, tmp3, n1word, n2word;

  //      FORMAT 10 OP CODES
  //      DOUBLE OPS - ONE TO THREE WORDS - SM0 TO SM7, DM0 TO DM7
  //
  //      There are 12 op codes in this class representing op codes "1000"
  //      to "6FFF" and "9000" to "EFFF". Nine of the op codes are word ops.
  //      Three are byte ops. Full source and destination mode addressing with
  //      any register is allowed. A one word op code is generated for SM0-
  //      SM5 and DM0-DM5 addressing. A two word op code is generated for either
  //      SM6-SM7 or DM6-DM7 addressing, but not both. For both SM6-SM7 and
  //      DM6-DM7 addressing a three word op code is generated. For a two word
  //      op code with word #1 at location X: X + 2 contains the source or
  //      destination offset and PC = X + 4 if PC is the register that applies
  //      to the offset in location X + 2. For a three word op code with word
  //      #1 at location X: X + 2 contains the source offset and X + 4 contains
  //      the destination offset. If the source register is PC than PC = X + 4
  //      when added to the offset to compute the source address. If the
  //      destination register is PC then PC = X + 6 when added to the offset to
  //      compute the destination address.
  //
  //      When using auto increments or decrements in either the source
  //      or destination (or both) fields the user must remember the following
  //      rule: All increments or decremnts in the source are fully completed
  //      before any destination decoding begins even if the same index register
  //      is used in both the source and destination. The two fields are
  //      totally independent.
  //

  op10 = (op >> 12) & 15; /* 0-15, but 0,7,8,15 invalid */
  dreg = op & 7;
  dmode = (op >> 3) & 7;
  sreg = (op >> 6) & 7;
  smode = (op >> 9) & 7;

  if (smode > 5) {
    getAMword((unsigned char *)&n1word, wd11_cpu_state->regs.PC);
    wd11_cpu_state->regs.PC += 2;
  }

  switch (op10) {
  case 1:
    //      ADD             ADD
    //      -------------------------------------------------------------
    //      FORMAT:         ADD SRC, DST
    //      OPERATION:      (DST) <- (SRC) + (DST)
    //      FUNCTION:       The source and destination operands are added to-
    //                      gether, and the sum is placed in, the destination.
    //      INDICATORS:     N = Set if (DST) bit 15 is set
    //                      Z = Set if (DST) = 0
    //                      V = Set if both operands were of the same sign and
    //                      the result was of the opposite sign
    //                      C = Set if a carry is generated from bit 15 of the
    //                      result
    //
    do_each("ADD");
    tmp = getAMwordBYmode(sreg, smode, n1word);
    if (dmode > 5) {
      getAMword((unsigned char *)&n2word, wd11_cpu_state->regs.PC);
      wd11_cpu_state->regs.PC += 2;
    }
    tmp2 = getAMwordBYmode(dreg, dmode, n2word);
    tmp3 = itmp = tmp + tmp2;
    undAMwordBYmode(dreg, dmode);
    putAMwordBYmode(dreg, dmode, n2word, tmp3);
    wd11_cpu_state->regs.PS.N = (tmp3 >> 15) & 1;
    wd11_cpu_state->regs.PS.Z = 0;
    if (tmp3 == 0)
      wd11_cpu_state->regs.PS.Z = 1;
    wd11_cpu_state->regs.PS.C = (tmp3 != itmp);
    wd11_cpu_state->regs.PS.V = 0;
    if ((tmp < 32768) & (tmp2 < 32768) & (tmp3 > 32767))
      wd11_cpu_state->regs.PS.V = 1;
    if ((tmp > 32767) & (tmp2 > 32767) & (tmp3 < 32768))
      wd11_cpu_state->regs.PS.V = 1;
    break;
  case 2:
    //      SUB             SUBTRACT
    //      -------------------------------------------------------------
    //      FORMAT:         SUB SRC, DST
    //      OPERATION:      (DST) <- (DST) - (SRC)
    //      FUNCTION:       The twos complemnt of the source operand is added
    //                      to the destination operand, and the sum is placed
    //                      in the destination.
    //      INDICATORS:     N = Set if (DST) bit 15 is set
    //                      Z = Set if (DST) = 0
    //                      V = Set if operands were of different signs and
    //                      the sign of the result is the same as the sign
    //                      of the source operand
    //                      C = Set if a borrow is generated from bit 15 of the
    //                      result
    //
    do_each("SUB");
    tmp = getAMwordBYmode(sreg, smode, n1word);
    if (dmode > 5) {
      getAMword((unsigned char *)&n2word, wd11_cpu_state->regs.PC);
      wd11_cpu_state->regs.PC += 2;
    }
    tmp2 = getAMwordBYmode(dreg, dmode, n2word);
    tmp3 = itmp = tmp2 - tmp;
    undAMwordBYmode(dreg, dmode);
    putAMwordBYmode(dreg, dmode, n2word, tmp3);
    wd11_cpu_state->regs.PS.N = (tmp3 >> 15) & 1;
    wd11_cpu_state->regs.PS.Z = 0;
    if (tmp3 == 0)
      wd11_cpu_state->regs.PS.Z = 1;
    wd11_cpu_state->regs.PS.C = (tmp3 != itmp);
    wd11_cpu_state->regs.PS.V = 0;
    if ((tmp < 32768) & (tmp2 > 32767) & (tmp3 < 32768))
      wd11_cpu_state->regs.PS.V = 1;
    if ((tmp > 32767) & (tmp2 < 32768) & (tmp3 > 32767))
      wd11_cpu_state->regs.PS.V = 1;
    break;
  case 3:
    //      AND             AND
    //      -------------------------------------------------------------
    //      FORMAT:         AND SRC, DST
    //      OPERATION:      (DST) <- (SRC) AND (DST)
    //      FUNCTION:       The source and destination operands are logically
    //                      ANDED together, and the result is placed in the
    //                      destination.
    //      INDICATORS:     N = Set if (DST) bit 15 is set
    //                      Z = Set if (DST) = 0
    //                      V = Reset
    //                      C = Unchanged
    //
    do_each("AND");
    tmp = getAMwordBYmode(sreg, smode, n1word);
    if (dmode > 5) {
      getAMword((unsigned char *)&n2word, wd11_cpu_state->regs.PC);
      wd11_cpu_state->regs.PC += 2;
    }
    tmp2 = getAMwordBYmode(dreg, dmode, n2word);
    tmp3 = tmp2 & tmp;
    undAMwordBYmode(dreg, dmode);
    putAMwordBYmode(dreg, dmode, n2word, tmp3);
    wd11_cpu_state->regs.PS.N = (tmp3 >> 15) & 1;
    wd11_cpu_state->regs.PS.Z = 0;
    if (tmp3 == 0)
      wd11_cpu_state->regs.PS.Z = 1;
    wd11_cpu_state->regs.PS.V = 0;
    break;
  case 4:
    //      BIC             BIT CLEAR
    //      -------------------------------------------------------------
    //      FORMAT:         BIC SRC, DST
    //      OPERATION:      (DST) <- [not (SRC)] AND (DST)
    //      FUNCTION:       The one's complement of the source operand is
    //      logically
    //                      ANDed with the destination operand, and the
    //                      result is placed in the destination.
    //      INDICATORS:     N = Set if (DST) bit 15 is set
    //                      Z = Set if (DST) = 0
    //                      V = Reset
    //                      C = Unchanged
    //
    do_each("BIC");
    tmp = getAMwordBYmode(sreg, smode, n1word);
    if (dmode > 5) {
      getAMword((unsigned char *)&n2word, wd11_cpu_state->regs.PC);
      wd11_cpu_state->regs.PC += 2;
    }
    tmp2 = getAMwordBYmode(dreg, dmode, n2word);
    tmp3 = (~tmp) & tmp2;
    undAMwordBYmode(dreg, dmode);
    putAMwordBYmode(dreg, dmode, n2word, tmp3);
    wd11_cpu_state->regs.PS.N = (tmp3 >> 15) & 1;
    wd11_cpu_state->regs.PS.Z = 0;
    if (tmp3 == 0)
      wd11_cpu_state->regs.PS.Z = 1;
    wd11_cpu_state->regs.PS.V = 0;
    break;
  case 5:
    //      BIS             BIT SET
    //      -------------------------------------------------------------
    //      FORMAT:         BIS SRC, DST
    //      OPERATION:      (DST) <- (SRC) OR (DST)
    //      FUNCTION:       The source and destination operands are logically
    //                      ORed, and the result is placed in the destination.
    //      INDICATORS:     N = Set if (DST) bit 15 is set
    //                      Z = Set if (DST) = 0
    //                      V= Reset
    //                      C = Unchanged
    //
    do_each("BIS");
    tmp = getAMwordBYmode(sreg, smode, n1word);
    if (dmode > 5) {
      getAMword((unsigned char *)&n2word, wd11_cpu_state->regs.PC);
      wd11_cpu_state->regs.PC += 2;
    }
    tmp2 = getAMwordBYmode(dreg, dmode, n2word);
    tmp3 = tmp2 | tmp;
    undAMwordBYmode(dreg, dmode);
    putAMwordBYmode(dreg, dmode, n2word, tmp3);
    wd11_cpu_state->regs.PS.N = (tmp3 >> 15) & 1;
    wd11_cpu_state->regs.PS.Z = 0;
    if (tmp3 == 0)
      wd11_cpu_state->regs.PS.Z = 1;
    wd11_cpu_state->regs.PS.V = 0;
    break;
  case 6:
    //      XOR             EXCLUSIVE OR
    //      -------------------------------------------------------------
    //      FORMAT:         XOR SRC, DST
    //      OPERATION:      (DST) <- (SRC) XOR (DST)
    //      FUNCTION:       The source and destination operands are logically
    //      Ex-
    //                      clusive ORed, and the result is placed in the
    //                      destination.
    //      INDICATORS:     N = Set if (DST) bit 15 is set
    //                      Z = Set if (DST) = 0
    //                      V = Reset
    //                      C = Unchanged
    //
    do_each("XOR");
    tmp = getAMwordBYmode(sreg, smode, n1word);
    if (dmode > 5) {
      getAMword((unsigned char *)&n2word, wd11_cpu_state->regs.PC);
      wd11_cpu_state->regs.PC += 2;
    }
    tmp2 = getAMwordBYmode(dreg, dmode, n2word);
    tmp3 = tmp2 ^ tmp;
    undAMwordBYmode(dreg, dmode);
    putAMwordBYmode(dreg, dmode, n2word, tmp3);
    wd11_cpu_state->regs.PS.N = (tmp3 >> 15) & 1;
    wd11_cpu_state->regs.PS.Z = 0;
    if (tmp3 == 0)
      wd11_cpu_state->regs.PS.Z = 1;
    wd11_cpu_state->regs.PS.V = 0;
    break;
  case 9:
    //      CMP             COMPARE
    //      -------------------------------------------------------------
    //      FORMAT:         CMP SRC, DST
    //      OPERATION:      (SRC) - (DST)
    //      FUNCTION:       The destination operand is subtracted from the
    //                      source operand and the result sets the indicators.
    //                      Neither operand is altered.
    //      INDICATORS:     N = Set if result bit 15 is set
    //                      Z = Set if result = 0
    //                      V = Set if operands were of opposite sign and the
    //                      sign of the result is the same as the sign of (DST)
    //                      C = Set if a borrow is generated from bit 15 of the
    //                      result
    //
    do_each("CMP");
    tmp = getAMwordBYmode(sreg, smode, n1word);
    if (dmode > 5) {
      getAMword((unsigned char *)&n2word, wd11_cpu_state->regs.PC);
      wd11_cpu_state->regs.PC += 2;
    }
    tmp2 = getAMwordBYmode(dreg, dmode, n2word);
    tmp3 = itmp = tmp - tmp2;
    //      if (wd11_cpu_state->regs.tracing)
    //        fprintf(stderr,"  - %04x, %04x, %04x, %04x", tmp, tmp2, tmp3,
    //        itmp);
    wd11_cpu_state->regs.PS.N = (tmp3 >> 15) & 1;
    wd11_cpu_state->regs.PS.Z = 0;
    if (tmp3 == 0)
      wd11_cpu_state->regs.PS.Z = 1;
    wd11_cpu_state->regs.PS.C = (tmp3 != itmp);
    wd11_cpu_state->regs.PS.V = 0;
    if ((tmp < 32768) & (tmp2 > 32767) & (tmp3 > 32767))
      wd11_cpu_state->regs.PS.V = 1;
    if ((tmp > 32767) & (tmp2 < 32768) & (tmp3 < 32768))
      wd11_cpu_state->regs.PS.V = 1;
    break;
  case 10:
    //      BIT             BIT TEST
    //      -------------------------------------------------------------
    //      FORMAT:         BIT SRC, DST
    //      OPERATION:      (SRC) AND (DST)
    //      FUNCTION:       The source and destination operands are logically
    //                      ANDed and the result sets the indicators.  Neither
    //                      operand is altered.
    //      INDICATORS:     N = Set if result bit 15 is set
    //                      Z = Set if result = 0
    //                      V = Reset
    //                      C = Unchanged
    //
    do_each("BIT");
    tmp = getAMwordBYmode(sreg, smode, n1word);
    if (dmode > 5) {
      getAMword((unsigned char *)&n2word, wd11_cpu_state->regs.PC);
      wd11_cpu_state->regs.PC += 2;
    }
    tmp2 = getAMwordBYmode(dreg, dmode, n2word);
    tmp3 = tmp2 & tmp;
    wd11_cpu_state->regs.PS.N = (tmp3 >> 15) & 1;
    wd11_cpu_state->regs.PS.Z = 0;
    if (tmp3 == 0)
      wd11_cpu_state->regs.PS.Z = 1;
    wd11_cpu_state->regs.PS.V = 0;
    break;
  case 11:
    //      MOV             MOVE
    //      -------------------------------------------------------------
    //      FORMAT:         MOV SRC, DST
    //      OPERATION:      (DST) <- (SRC)
    //      FUNCTION:       The destination operand is replaced with the source
    //                      operand.
    //      INDICATORS:     N = Set if (DST) bit 15 is set
    //                      Z = Set if (DST) = 0
    //                      V = Reset
    //                      C = Unchanged
    //
    do_each("MOV");
    tmp = getAMwordBYmode(sreg, smode, n1word);
    if (dmode > 5) {
      getAMword((unsigned char *)&n2word, wd11_cpu_state->regs.PC);
      wd11_cpu_state->regs.PC += 2;
    }
    putAMwordBYmode(dreg, dmode, n2word, tmp);
    wd11_cpu_state->regs.PS.N = (tmp >> 15) & 1;
    wd11_cpu_state->regs.PS.Z = 0;
    if (tmp == 0)
      wd11_cpu_state->regs.PS.Z = 1;
    wd11_cpu_state->regs.PS.V = 0;
    break;
  case 12:
    //
    //      ------------- BYTE OPS --------------------------------------
    //
    //      For SM0 addressing only the lower byte of the source register is
    //      used as an operand. For SMl-SM7 addressing only the addressed memory
    //      byte is used as an operand. For DM0 addressing only the lower byte
    //      of of the destination register is used as an operand with the
    //      exception: MOVB will extend the sign through bit 15. For DMl-DM7
    //      addressing only the addressed memory byte is used as an operand.
    //

    //      CMPB            COMPARE BYTE
    //      -------------------------------------------------------------
    //      FORMAT:         CMPB SRC, DST
    //      OPERATION:      (SRC) - (DST)
    //      FUNCTION:       The destination operaud is subtracted from the
    //                      source operand, and the result sets the indicators.
    //                      Neither operand is altered.
    //      INDICATORS:     N = Set if result bit 7 is set
    //                      Z = Set if result = 0
    //                      V = Set if operands were of different signs and
    //                      the sign of the result is the same as the sign
    //                      of (DST).
    //                      C = Set if a borrow is generated frown result bit 7
    //
    do_each("CMPB");
    tmp = getAMbyteBYmode(sreg, smode, n1word);
    tmp &= 255;
    if (dmode > 5) {
      getAMword((unsigned char *)&n2word, wd11_cpu_state->regs.PC);
      wd11_cpu_state->regs.PC += 2;
    }
    tmp2 = getAMbyteBYmode(dreg, dmode, n2word);
    tmp2 &= 255;
    tmp3 = tmp - tmp2;
    wd11_cpu_state->regs.PS.N = (tmp3 >> 7) & 1;
    wd11_cpu_state->regs.PS.Z = 0;
    if (tmp3 == 0)
      wd11_cpu_state->regs.PS.Z = 1;
    wd11_cpu_state->regs.PS.C = (tmp2 > tmp);
    wd11_cpu_state->regs.PS.V = 0;
    if ((tmp < 127) & (tmp2 > 128) & (tmp3 > 128))
      wd11_cpu_state->regs.PS.V = 1;
    if ((tmp > 128) & (tmp2 < 127) & (tmp3 < 127))
      wd11_cpu_state->regs.PS.V = 1;
    break;
  case 13:
    //      MOVB            MOVE BYTE
    //      -------------------------------------------------------------
    //      FORMAT:         MOVB SRC, DST
    //      OPERATION:      (DST) <- (SRC)
    //      FUNCTION:       The destination operand is replaced with the source
    //                      operand. If DM0 the sign bit (bit 7) is replicated
    //                      through bit 15.
    //      INDICATORS:     N = Set if (DST) bit 7 is set
    //                      Z = Set if (DST) = 0
    //                      V = Reset
    //                      C = Unchanged
    //
    do_each("MOVB");
    tmp = getAMbyteBYmode(sreg, smode, n1word);
    if (dmode > 5) {
      getAMword((unsigned char *)&n2word, wd11_cpu_state->regs.PC);
      wd11_cpu_state->regs.PC += 2;
    }
    putAMbyteBYmode(dreg, dmode, n2word, tmp);
    wd11_cpu_state->regs.PS.N = (tmp >> 7) & 1;
    if (dmode == 0) {
      wd11_cpu_state->regs.gpr[dreg] &= 0xff;
      if (wd11_cpu_state->regs.PS.N == 1)
        wd11_cpu_state->regs.gpr[dreg] |= 0xff00;
    }
    wd11_cpu_state->regs.PS.Z = 0;
    if (tmp == 0)
      wd11_cpu_state->regs.PS.Z = 1;
    wd11_cpu_state->regs.PS.V = 0;
    break;
  case 14:
    //      BISB            BIT SET BYTE
    //      -------------------------------------------------------------
    //      FORMAT:         BISB SRC, DST
    //      FUNCTION:       (DST) <- (SRC) OR (DST)
    //      OPERATION:      The source and destination operands are logically
    //                      ORED, and the result is placed in the destination.
    //      INDICATORS:     N = Set if (DST) bit 7 is set
    //                      Z = Set if (DST) = 0
    //                      V = Reset
    //                      c = Unchanged
    //
    do_each("BISB");
    tmp = getAMbyteBYmode(sreg, smode, n1word);
    if (dmode > 5) {
      getAMword((unsigned char *)&n2word, wd11_cpu_state->regs.PC);
      wd11_cpu_state->regs.PC += 2;
    }
    tmp2 = getAMbyteBYmode(dreg, dmode, n2word);
    tmp3 = tmp2 | tmp;
    undAMbyteBYmode(dreg, dmode);
    putAMbyteBYmode(dreg, dmode, n2word, tmp3);
    wd11_cpu_state->regs.PS.N = (tmp3 >> 7) & 1;
    wd11_cpu_state->regs.PS.Z = 0;
    if (tmp3 == 0)
      wd11_cpu_state->regs.PS.Z = 1;
    wd11_cpu_state->regs.PS.V = 0;
    break;
  default:
    assert("invalid return from fmt_10 lookup...");
    do_fmt_invalid();
    break;
  } /* end switch(op10) */

} /* end function do_fmt_10 */
