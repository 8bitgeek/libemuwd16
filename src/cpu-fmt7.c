/* cpu-fmt7.c         (c) Copyright Mike Noel, 2001-2008             */
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

#include "cpu-fmt7.h"

#define do_each(opc)                                                           \
  if (wd11_cpu_state->regs.tracing)                                                            \
    trace_fmt7(opc, mode, reg, n1word);

void do_fmt_7(wd11_cpu_state_t* wd11_cpu_state) {
  int op7, mode, reg;
  uint16_t tmp, tmp2, tmp3, n1word;

  //      FORMAT 7 OP CODES
  //
  //      SINGLE OPS - ONE OR TWO WORDS - DM0 TO DM7
  //
  //      There are 32 op codes in this class representing op codes
  //      "0A00" to "0DFF" and "8A00" to "8DFF". All addressing modes from
  //      0 to 7 are available with all registers available as index registers
  //      (see chapter two). A one word op code is generated for addressing
  //      modes 0 to 5. A two word op code is generated for addressing
  //      modes 6 and 7 with the offset value in word two. For DM6 and,
  //      DM7 with PC as the index register PC is added to the offset from word
  //      two after the offset is fetched from memory. The offset is therefore
  //      relative to a PC that points to the op code that follows (i.e.
  //      current op code + 4). Codes "8A00" to "8CC0" are BYTE ops.
  //

  reg = (wd11_cpu_state->op & 7);
  mode = (wd11_cpu_state->op & 63) >> 3;
  op7 = wd11_cpu_state->op >> 6; /* 40-55, 552-567 */
  if (op7 > 55)
    op7 = op7 - 496; /* 40-71 */

  if (mode > 5) {
    wd11_cpu_state->getAMword((unsigned char *)&n1word, wd11_cpu_state->regs.PC);
    wd11_cpu_state->regs.PC += 2;
  }

  switch (op7) {
  case 40:
    //      ROR             ROTATE RIGHT
    //      -------------------------------------------------------------
    //      FORMAT:         ROR DST
    //      FUNCTION:       A l-bit right rotate is done on (DST):C-Flag
    //                      The C-Flag is shifted into (DST) bit 15, and (DST)
    //                      bit 0 is shifted into the C-flag.
    //      INDICATORS:     N = Set if bit 7 of (DST) is set
    //                      Z = Set if (DST) = 0
    //                      V = Set to exclusive or of N and C flags
    //                      C = Set to the value of the bit shifted out of (DST)
    //
    // NOTE - comments (and expected result) of 'cpu' diagnostic test 343 are
    //        not consistant with V = exclusive-or of N and C flags.
    //   "TSTCC 10005     ;** V should be set"
    //
    do_each("ROR");
    tmp = wd11_cpu_state->getAMwordBYmode(reg, mode, n1word);
    tmp2 = tmp & 1;
    tmp = tmp >> 1;
    if (wd11_cpu_state->regs.PS.C == 1)
      tmp |= 0x8000;
    wd11_cpu_state->regs.PS.C = tmp2;
    wd11_cpu_state->undAMwordBYmode(reg, mode);
    wd11_cpu_state->putAMwordBYmode(reg, mode, n1word, tmp);
    wd11_cpu_state->regs.PS.N = (tmp >> 7) & 1;
    wd11_cpu_state->regs.PS.Z = 0;
    if (tmp == 0)
      wd11_cpu_state->regs.PS.Z = 1;
    wd11_cpu_state->regs.PS.V = wd11_cpu_state->regs.PS.C ^ wd11_cpu_state->regs.PS.N;
    /* ??? */ wd11_cpu_state->regs.PS.V = 0;
    break;
  case 41:
    //      ROL             ROTATE LEFT
    //      -------------------------------------------------------------
    //      FORMAT:         ROL DST
    //      FUNCTION:       A l-bit left rotate is done on C-Flag:(DST). The
    //                      C-Flag is shifted into (DST) bit 0, and (DST)
    //                      bit 15 is shifted into the C-Flag.
    //      INDICATORS:     N = Set if bit 15 of (DST) is set
    //                      Z = Set if (DST) = 0
    //                      V = Set to exclusive or of N and C flags
    //                      C = Set to the value of the bit shifted out of (DST)
    //
    do_each("ROL");
    tmp = wd11_cpu_state->getAMwordBYmode(reg, mode, n1word);
    tmp2 = ((tmp & 0x8000) != 0);
    tmp = tmp << 1;
    if (wd11_cpu_state->regs.PS.C == 1)
      tmp |= 1;
    wd11_cpu_state->regs.PS.C = tmp2;
    wd11_cpu_state->undAMwordBYmode(reg, mode);
    wd11_cpu_state->putAMwordBYmode(reg, mode, n1word, tmp);
    wd11_cpu_state->regs.PS.N = (tmp >> 15) & 1;
    wd11_cpu_state->regs.PS.Z = 0;
    if (tmp == 0)
      wd11_cpu_state->regs.PS.Z = 1;
    wd11_cpu_state->regs.PS.V = wd11_cpu_state->regs.PS.C ^ wd11_cpu_state->regs.PS.N;
    break;
  case 42:
    //      TST             TEST WORD
    //      -------------------------------------------------------------
    //      FORMAT:         TST DST
    //      OPERATION:      (DST) and (DST)
    //      FUNCTION:       The indicators are set to reflect the destination
    //                      operand status.
    //      INDICATORS:     N = Set if (DST) bit 15 is set
    //                      Z = Set if (DST) = 0
    //                      V = Reset
    //                      C = Unchanged
    //
    do_each("TST");
    tmp = wd11_cpu_state->getAMwordBYmode(reg, mode, n1word);
    wd11_cpu_state->regs.PS.N = (tmp >> 15) & 1;
    wd11_cpu_state->regs.PS.Z = 0;
    if (tmp == 0)
      wd11_cpu_state->regs.PS.Z = 1;
    wd11_cpu_state->regs.PS.V = 0;
    break;
  case 43:
    //      ASL             ARITHMETIC SHIFT LEFT
    //      -------------------------------------------------------------
    //      FORMAT:         ASL DST
    //      FUNCTION:       A l-bit left arithmetic shift is done on (DST). A
    //                      zero is shifted into (DST) bit 0, and (DST) bit 15
    //                      is shifted into the C-Flag.
    //      INDICATORS:     N = Set if (DST) bit 15 is set
    //                      Z = Set if (DST) = 0
    //                      V = Set to exclusive or of N and C flags
    //                      C = Set to the value of the bit shifted out of (DST)
    //
    do_each("ASL");
    tmp = wd11_cpu_state->getAMwordBYmode(reg, mode, n1word);
    tmp2 = ((tmp & 0x8000) != 0);
    tmp = tmp << 1;
    wd11_cpu_state->regs.PS.C = tmp2;
    wd11_cpu_state->undAMwordBYmode(reg, mode);
    wd11_cpu_state->putAMwordBYmode(reg, mode, n1word, tmp);
    wd11_cpu_state->regs.PS.N = (tmp >> 15) & 1;
    wd11_cpu_state->regs.PS.Z = 0;
    if (tmp == 0)
      wd11_cpu_state->regs.PS.Z = 1;
    wd11_cpu_state->regs.PS.V = wd11_cpu_state->regs.PS.C ^ wd11_cpu_state->regs.PS.N;
    break;
  case 44:
    //      SET             SET TO ONES
    //      -------------------------------------------------------------
    //      FORMAT:         SET DST
    //      OPERATION:      (DST) <- "FFFF"
    //      FUNCTION:       The destination operand is set to all ones
    //      INDICATORS:     N = Set
    //                      Z = Reset
    //                      V = Reset
    //                      C = Unchanged
    //
    do_each("SET");
    tmp = -1;
    wd11_cpu_state->putAMwordBYmode(reg, mode, n1word, tmp);
    wd11_cpu_state->regs.PS.N = 1;
    wd11_cpu_state->regs.PS.Z = 0;
    wd11_cpu_state->regs.PS.V = 0;
    break;
  case 45:
    //      CLR             CLEAR TO ZEROS
    //      -------------------------------------------------------------
    //      FORMAT:         CLR DST
    //      OPERATION:      (DST) <- 0
    //      FUNCTION:       The destination operand is cleared to all zeros
    //      INDICATORS:     N = Reset
    //                      Z = Set
    //                      V = Reset
    //                      C = Unchanged if DM0. Reset if DMl-DM7.
    //
    do_each("CLR");
    tmp = 0;
    wd11_cpu_state->putAMwordBYmode(reg, mode, n1word, tmp);
    wd11_cpu_state->regs.PS.N = 0;
    wd11_cpu_state->regs.PS.Z = 1;
    wd11_cpu_state->regs.PS.V = 0;
    if (mode > 0)
      wd11_cpu_state->regs.PS.C = 0;
    break;
  case 46:
    //      ASR             ARITHMETIC SHIFT RIGHT
    //      -------------------------------------------------------------
    //      FORMAT:         ASR DST
    //      FUNCTION:       A l-bit right arithmetic shift is done on (DST). Bit
    //                      15 of (DST) is replicated. Bit 0 of (DST) is shifted
    //                      into the C-Flag.
    //      INDICATORS:     N = set if (DST) bit 7 is set
    //                      Z = Set if (DST) = 0
    //                      V = Set to exclusive or of N and C flags
    //                      C = set to the value of the bit shifted out of (DST)
    //
    // NOTE - comments (and expected result) of 'cpu' diagnostic test 375 and
    //        400 are not consistant with V = exclusive-or of N and C flags.
    //   "TSTCC 10005     ;** V should be set"
    //   "TSTCC 10001     ;** V should be set; should not set N"
    //
    do_each("ASR");
    tmp = wd11_cpu_state->getAMwordBYmode(reg, mode, n1word);
    tmp2 = tmp & 1;
    tmp3 = tmp & 0x8000;
    tmp = tmp >> 1;
    tmp = tmp | tmp3;
    wd11_cpu_state->regs.PS.C = tmp2;
    wd11_cpu_state->undAMwordBYmode(reg, mode);
    wd11_cpu_state->putAMwordBYmode(reg, mode, n1word, tmp);
    wd11_cpu_state->regs.PS.N = (tmp >> 7) & 1;
    wd11_cpu_state->regs.PS.Z = 0;
    if (tmp == 0)
      wd11_cpu_state->regs.PS.Z = 1;
    wd11_cpu_state->regs.PS.V = wd11_cpu_state->regs.PS.C ^ wd11_cpu_state->regs.PS.N;
    /* ??? */ wd11_cpu_state->regs.PS.V = 0;
    break;
  case 47:
    //      SWAB            SWAP BYTES
    //      -------------------------------------------------------------
    //      FORMAT:         SWAB DST
    //      OPEBATION:      (DST) 15-8 <--> (DST) 7-0
    //      FUNCTION:       The upper and lower bytes of (DST) are exhanged,
    //      INDICATORS:     N = Set if (DST) bit 7 is set
    //                      Z = Set if (DST) lower byte = 0
    //                      V = Reset
    //                      C = Unchanged
    //
    do_each("SWAB");
    tmp = wd11_cpu_state->getAMwordBYmode(reg, mode, n1word);
    tmp2 = tmp >> 8;
    tmp3 = (tmp & 0xff) << 8;
    tmp = tmp2 | tmp3;
    wd11_cpu_state->undAMwordBYmode(reg, mode);
    wd11_cpu_state->putAMwordBYmode(reg, mode, n1word, tmp);
    wd11_cpu_state->regs.PS.N = (tmp >> 7) & 1;
    wd11_cpu_state->regs.PS.Z = 0;
    if ((tmp & 0xff) == 0)
      wd11_cpu_state->regs.PS.Z = 1;
    wd11_cpu_state->regs.PS.V = 0;
    break;
  case 48:
    //      COM             COMPLEMENT
    //      -------------------------------------------------------------
    //      FORMAT:         COM DST
    //      OPERATION:      (DST) <- not (DST)
    //      FUNCTION:       The destination operand is one's complemented
    //      INDICATORS:     N = Set if (DST) bit 15 is set
    //                      Z = Set if (DST) = 0
    //                      V = Reset
    //                      C = Set
    //
    do_each("COM");
    tmp = wd11_cpu_state->getAMwordBYmode(reg, mode, n1word);
    tmp = (~tmp) & 0xffff;
    wd11_cpu_state->undAMwordBYmode(reg, mode);
    wd11_cpu_state->putAMwordBYmode(reg, mode, n1word, tmp);
    wd11_cpu_state->regs.PS.N = (tmp >> 15) & 1;
    wd11_cpu_state->regs.PS.Z = 0;
    if (tmp == 0)
      wd11_cpu_state->regs.PS.Z = 1;
    wd11_cpu_state->regs.PS.V = 0;
    wd11_cpu_state->regs.PS.C = 1;
    break;
  case 49:
    //      NEG             NEGATE
    //      -------------------------------------------------------------
    //      FORMAT:         NEG DST
    //      OPERATION:      (DST) <- -(DST)
    //      FUNCTION:       The destination operand is two's complemented
    //      INDICATORS:     N = Set if (DST) bit 15 is set
    //                      Z = Set if (DST) = 0
    //                      V = Set if (DST) = "8000"
    //                      C = Reset if (DST) = 0
    //
    do_each("NEG");
    tmp = wd11_cpu_state->getAMwordBYmode(reg, mode, n1word);
    tmp = (-tmp) & 0xffff;
    wd11_cpu_state->undAMwordBYmode(reg, mode);
    wd11_cpu_state->putAMwordBYmode(reg, mode, n1word, tmp);
    wd11_cpu_state->regs.PS.N = (tmp >> 15) & 1;
    wd11_cpu_state->regs.PS.V = 0;
    if (tmp == 0x8000)
      wd11_cpu_state->regs.PS.V = 1;
    if (tmp == 0) {
      wd11_cpu_state->regs.PS.Z = 1;
      wd11_cpu_state->regs.PS.C = 0;
    } else {
      wd11_cpu_state->regs.PS.Z = 0;
      wd11_cpu_state->regs.PS.C = 1;
    }
    break;
  case 50:
    //      INC             INCREMENT
    //      -------------------------------------------------------------
    //      FORMAT:         INC DST
    //      OPERATION:      (DST) <- (DST) + 1
    //      FUNCTION:       The destination operand is incremnted by one
    //      INDICATORS:     N = Set if (DST) bit 15 is set
    //                      Z = set if (DST) = 0
    //                      V = Set if (DST) = "8000"
    //                      C = Set if a carry is generated from (DST) bit 15
    //
    do_each("INC");
    tmp = wd11_cpu_state->getAMwordBYmode(reg, mode, n1word);
    tmp2 = (tmp >> 15) & 1;
    tmp = (tmp + 1) & 0xffff;
    wd11_cpu_state->undAMwordBYmode(reg, mode);
    wd11_cpu_state->putAMwordBYmode(reg, mode, n1word, tmp);
    wd11_cpu_state->regs.PS.N = (tmp >> 15) & 1;
    wd11_cpu_state->regs.PS.Z = 0;
    if (tmp == 0)
      wd11_cpu_state->regs.PS.Z = 1;
    wd11_cpu_state->regs.PS.V = 0;
    if (tmp == 0x8000)
      wd11_cpu_state->regs.PS.V = 1;
    wd11_cpu_state->regs.PS.C = wd11_cpu_state->regs.PS.Z;
    break;
  case 51:
    //      DEC             DECREMENT
    //      -------------------------------------------------------------
    //      FORMAT:         DEC DST
    //      OPERATION:      (DST) <- (DST) - 1
    //      FUNCTION:       The destination operand is decremented by one
    //      INDICATORS:     N = Set if (DST) bit 15 is set
    //                      Z = Set if (DST) = 0
    //                      V = Set if (DST) = "7FFF"
    //                      C = Set if a borrow is generated from (DST) bit 15
    //
    do_each("DEC");
    tmp = wd11_cpu_state->getAMwordBYmode(reg, mode, n1word);
    wd11_cpu_state->regs.PS.C = 0;
    if (tmp == 0)
      wd11_cpu_state->regs.PS.C = 1;
    tmp2 = (tmp >> 15) & 1;
    tmp = (tmp - 1) & 0xffff;
    wd11_cpu_state->undAMwordBYmode(reg, mode);
    wd11_cpu_state->putAMwordBYmode(reg, mode, n1word, tmp);
    wd11_cpu_state->regs.PS.N = (tmp >> 15) & 1;
    wd11_cpu_state->regs.PS.Z = 0;
    if (tmp == 0)
      wd11_cpu_state->regs.PS.Z = 1;
    wd11_cpu_state->regs.PS.V = 0;
    if (tmp == 0x7FFF)
      wd11_cpu_state->regs.PS.V = 1;
    break;
  case 52:
    //      IW2             INCREMENT WORD BY TWO
    //      -------------------------------------------------------------
    //      FORMAT:         IW2 DST
    //      OPERATION:      (DST) <- (DST) + 2
    //      FUNCTION:       The destination operand is incremented by two
    //      INDICATORS:     N = Set if (DST) bit 15 is set
    //                      Z = Set if (DST) = 0
    //                      V = Set if (DST) = "8000' or "800l"
    //                      C = Set if a carry is generated from (DST) bit 15
    //
    do_each("IW2");
    wd11_cpu_state->regs.PS.C = 0;
    tmp = wd11_cpu_state->getAMwordBYmode(reg, mode, n1word);
    tmp2 = (tmp >> 15) & 1;
    tmp = (tmp + 1) & 0xffff;
    if (tmp == 0)
      wd11_cpu_state->regs.PS.C = 1;
    tmp = (tmp + 1) & 0xffff;
    if (tmp == 0)
      wd11_cpu_state->regs.PS.C = 1;
    wd11_cpu_state->undAMwordBYmode(reg, mode);
    wd11_cpu_state->putAMwordBYmode(reg, mode, n1word, tmp);
    wd11_cpu_state->regs.PS.N = (tmp >> 15) & 1;
    wd11_cpu_state->regs.PS.Z = 0;
    if (tmp == 0)
      wd11_cpu_state->regs.PS.Z = 1;
    wd11_cpu_state->regs.PS.V = 0;
    if (tmp == 0x8000)
      wd11_cpu_state->regs.PS.V = 1;
    break;
  case 53:
    //      SXT             SIGN EXTEND
    //      -------------------------------------------------------------
    //      FORMAT:         SXT DST
    //      OPERATION:      IF N = 0, (DST) <- 0
    //                      IF N = 1, (DST) <- "FFFF"
    //      FUNCTION:       The N-Flag status is replicated in the destination
    //      operand INDICATORS:     Unchanged
    //
    do_each("SXT");
    tmp = 0;
    if (wd11_cpu_state->regs.PS.N == 1)
      tmp = 0xFFFF;
    wd11_cpu_state->putAMwordBYmode(reg, mode, n1word, tmp);
    break;
  case 54:
    //      TCALL           TABLED SUBROUTINE CALL
    //      -------------------------------------------------------------
    //      FORMAT:         TCALL DST
    //      OPERATION:      !SP, @SP <- PC
    //                      PC <- PC + (DST)
    //                      PC <- PC + @PC
    //      FUNCTION:       PC, which points to the op code that follows, is
    //      pushed
    //                      onto the stack. The destination operand is added to
    //                      PC. The contents of this intermediate table address
    //                      is also added to PC to get the final destination
    //                      address. Note that at least one op code must exist
    //                      between the TCALL and the table for a subroutine
    //                      return.
    //      INDICATORS:     Unchanged
    //
    do_each("TCALL");
    /* ORIGINAL CODE
                        wd11_cpu_state->regs.SP -= 2; putAMword((unsigned char
       *)&wd11_cpu_state->regs.PC,wd11_cpu_state->regs.SP); tmp = wd11_cpu_state->getAMwordBYmode(reg, mode, n1word); wd11_cpu_state->regs.PC +=
       tmp; wd11_cpu_state->getAMword((unsigned char *)&tmp, wd11_cpu_state->regs.PC); wd11_cpu_state->regs.PC += tmp;
    */

    // MODIFIED CODE BY FJC
    tmp2 = wd11_cpu_state->regs.PC; // save return address
    tmp = wd11_cpu_state->getAMwordBYmode(reg, mode, n1word);
    wd11_cpu_state->regs.SP -= 2; // mov tmp,-(sp)
    wd11_cpu_state->putAMword((unsigned char *)&tmp2, wd11_cpu_state->regs.SP);
    wd11_cpu_state->regs.PC += tmp; // add @pc,pc
    wd11_cpu_state->getAMword((unsigned char *)&tmp, wd11_cpu_state->regs.PC);
    wd11_cpu_state->regs.PC += tmp;

    break;
  case 55:
    //      TJMP            TABLED JUMP
    //      -------------------------------------------------------------
    //      FORMAT:         TJMP DST
    //      OPERATION:      PC <- PC + (DST)
    //                      PC <- PC + @PC
    //      FUNCTION:       The destination operand is added to PC, and the
    //      contents
    //                      of this intermediate location is also added to PC to
    //                      get the final destination address.
    //      INDICATORS:     Unchanged
    //
    do_each("TJMP");
    tmp = wd11_cpu_state->getAMwordBYmode(reg, mode, n1word);
    wd11_cpu_state->regs.PC += tmp;
    wd11_cpu_state->getAMword((unsigned char *)&tmp, wd11_cpu_state->regs.PC);
    wd11_cpu_state->regs.PC += tmp;
    break;
  case 56:
    //
    //
    //      ------------- BYTE OPS --------------------------------------
    //
    //      For DM0 addressing only the lower byte of the destination register
    //      is affected by a byte op code. For DMl-DM7 addressing only the spec-
    //      fied memory byte is affected by a byte op.  For even memory
    //      addresses the lower byte is altered, and for odd memory addresses
    //      the upper byte is altered.
    //
    //
    //      RORB            ROTATE RIGHT BYTE
    //      -------------------------------------------------------------
    //      FORMAT:         RORB DST
    //      FUNCTION:       A l-bit right rotate is done on (DST):C-Flag. Bit 0
    //                      of (DST) is shifted into the C-Flag, and the C-Flag
    //                      is shifted into (DST) bit 7.
    //      INDICATORS:     N = Set if (DST) bit 7 is set
    //                      Z = Set if (DST) = 0
    //                      V = Set to exclusive or of N and C flags
    //                      C = Set to value of bit shifted out of (DST) bit 0
    //
    // NOTE - comments (and expected result) of 'cpu' diagnostic test 535 are
    //        not consistant with V = exclusive-or of N and C flags.
    //   "TSTCC 10005     ;** V should be set"
    //
    do_each("RORB");
    tmp = wd11_cpu_state->getAMbyteBYmode(reg, mode, n1word);
    tmp2 = tmp & 1;
    tmp = tmp >> 1;
    if (wd11_cpu_state->regs.PS.C == 1)
      tmp |= 0x80;
    wd11_cpu_state->regs.PS.C = tmp2;
    wd11_cpu_state->undAMbyteBYmode(reg, mode);
    wd11_cpu_state->putAMbyteBYmode(reg, mode, n1word, tmp);
    wd11_cpu_state->regs.PS.N = (tmp >> 7) & 1;
    wd11_cpu_state->regs.PS.Z = 0;
    if (tmp == 0)
      wd11_cpu_state->regs.PS.Z = 1;
    wd11_cpu_state->regs.PS.V = wd11_cpu_state->regs.PS.C ^ wd11_cpu_state->regs.PS.N;
    /* ??? */ wd11_cpu_state->regs.PS.V = 0;
    break;
  case 57:
    //      ROLB            ROTATE LEFT BYTE
    //      -------------------------------------------------------------
    //      FORMAT:         ROLB DST
    //      FUNCTION:       A l-bit left rotate is done on C-flag:(DST). Bit 7
    //                      of (DST) is shifted into the C-flag, and the C-flag
    //                      is shifted into (DST) bit 0.
    //      INDICATORS:     N = Set if (DST) bit 7 is set
    //                      Z = Set if (DST) = 0
    //                      V = Set to exclusive or of N and C flags
    //                      C = Set to value of bit shifted out of (DST) bit 7
    //
    do_each("ROLB");
    tmp = wd11_cpu_state->getAMbyteBYmode(reg, mode, n1word);
    tmp2 = ((tmp & 0x80) != 0);
    tmp = tmp << 1;
    if (wd11_cpu_state->regs.PS.C == 1)
      tmp |= 1;
    wd11_cpu_state->regs.PS.C = tmp2;
    wd11_cpu_state->undAMbyteBYmode(reg, mode);
    wd11_cpu_state->putAMbyteBYmode(reg, mode, n1word, tmp);
    wd11_cpu_state->regs.PS.N = (tmp >> 7) & 1;
    wd11_cpu_state->regs.PS.Z = 0;
    if (tmp == 0)
      wd11_cpu_state->regs.PS.Z = 1;
    wd11_cpu_state->regs.PS.V = wd11_cpu_state->regs.PS.C ^ wd11_cpu_state->regs.PS.N;
    break;
  case 58:
    //      TSTB            TEST BYTE
    //      -------------------------------------------------------------
    //      FORMAT:         TSTB DST
    //      OPERATION:      (DST) and (DST)
    //      FUNCTION:       The destination operand status sets the indicators,
    //      INDICATORS:     N = Set if (DST) bit 7 is set
    //                      Z = Set if (DST) = 0
    //                      V = Reset
    //                      C = Unchanged
    //
    do_each("TSTB");
    tmp = wd11_cpu_state->getAMbyteBYmode(reg, mode, n1word);
    wd11_cpu_state->regs.PS.N = (tmp >> 7) & 1;
    wd11_cpu_state->regs.PS.Z = 0;
    if (tmp == 0)
      wd11_cpu_state->regs.PS.Z = 1;
    wd11_cpu_state->regs.PS.V = 0;
    break;
  case 59:
    //      ASLB            ARITHMETIC SHIFT LEFT BYTE
    //      -------------------------------------------------------------
    //      FORMAT:         ASLB DST
    //      FUNCTION:       A l-bit left arithmetic shift is done on
    //      C-Flag:(DST)
    //                      A zero is shifted into (DST) bit 0, and (DST) bit 7
    //                      is shifted into the C-flag.
    //      INDICATORS:     N = set if (DST) bit 7 is set
    //                      Z = Set if (DST) = 0
    //                      V = Set to exclusive or of N and C flags
    //                      C = Set to value of bit shifted out of (DST) bit 7
    //
    do_each("ASLB");
    tmp = wd11_cpu_state->getAMbyteBYmode(reg, mode, n1word);
    tmp2 = (tmp >> 7) & 1;
    tmp = (tmp << 1) & 0xff;
    wd11_cpu_state->regs.PS.C = tmp2;
    wd11_cpu_state->undAMbyteBYmode(reg, mode);
    wd11_cpu_state->putAMbyteBYmode(reg, mode, n1word, tmp);
    wd11_cpu_state->regs.PS.N = (tmp >> 7) & 1;
    wd11_cpu_state->regs.PS.Z = 0;
    if (tmp == 0)
      wd11_cpu_state->regs.PS.Z = 1;
    wd11_cpu_state->regs.PS.V = wd11_cpu_state->regs.PS.C ^ wd11_cpu_state->regs.PS.N;
    break;
  case 60:
    //      SETB            SET BYTE TO ONES
    //      -------------------------------------------------------------
    //      FORMAT:         SETB DST
    //      OPERATION:      (DST) <- "FF"
    //      FUNCTION:       The destination byte operand is set to all ones
    //      INDICATORS:     N = Set
    //                      Z = Reset
    //                      V = Reset
    //                      C = Unchanged
    //
    do_each("SETB");
    tmp = -1;
    wd11_cpu_state->putAMbyteBYmode(reg, mode, n1word, tmp);
    wd11_cpu_state->regs.PS.N = 1;
    wd11_cpu_state->regs.PS.Z = 0;
    wd11_cpu_state->regs.PS.V = 0;
    break;
  case 61:
    //      CLRB            CLEAR BYTE TO ZEROS
    //      -------------------------------------------------------------
    //      FORMAT:         CLRB DST
    //      OPERATION:      (DST) <- 0
    //      FUNCTION:       The destination byte operand is cleared to all
    //      zeros. INDICATORS:     N = Reset
    //                      Z = Set
    //                      V = Reset
    //                      C = Reset
    //
    do_each("CLRB");
    tmp = 0;
    wd11_cpu_state->putAMbyteBYmode(reg, mode, n1word, tmp);
    wd11_cpu_state->regs.PS.N = 0;
    wd11_cpu_state->regs.PS.Z = 1;
    wd11_cpu_state->regs.PS.V = 0;
    wd11_cpu_state->regs.PS.C = 0;
    break;
  case 62:
    //      ASRB            ARITHMETIC SHIFT RIGHT BYTE
    //      -------------------------------------------------------------
    //      FORMAT:         ASRB DST
    //      FUNCTION:       A l-bit right arithmetic shift is done on (DST):
    //                      C-flag. Bit 7 of (DST) is replicated.  Bit 0 of
    //                      (DST) is shifted into the C-flag.
    //      INDICATORS:     N = Set if (DST) bit 7 is set
    //                      Z = Set if (DST) = 0
    //                      V = Set to exclusive or of N and C flags
    //                      C = Set to value of bit shifted out of (DST) bit 0
    //
    // NOTE - comments (and expected result) of 'cpu' diagnostic test 576 and
    //        601 are not consistant with V = exclusive-or of N and C flags.
    //   "TSTCC 10005     ;** V should be set"
    //   "TSTCC 10010     ;** V should be set"
    //
    do_each("ASRB");
    tmp = wd11_cpu_state->getAMbyteBYmode(reg, mode, n1word);
    tmp2 = tmp & 1;
    tmp3 = tmp & 0x80;
    tmp = tmp >> 1;
    tmp = tmp | tmp3;
    wd11_cpu_state->regs.PS.C = tmp2;
    wd11_cpu_state->undAMbyteBYmode(reg, mode);
    wd11_cpu_state->putAMbyteBYmode(reg, mode, n1word, tmp);
    wd11_cpu_state->regs.PS.N = (tmp >> 7) & 1;
    wd11_cpu_state->regs.PS.Z = 0;
    if (tmp == 0)
      wd11_cpu_state->regs.PS.Z = 1;
    wd11_cpu_state->regs.PS.V = wd11_cpu_state->regs.PS.C ^ wd11_cpu_state->regs.PS.N;
    /* ??? */ wd11_cpu_state->regs.PS.V = 0;
    break;
  case 63:
    //      SWAD            SWAP DIGITS
    //      -------------------------------------------------------------
    //      FORMAT:         SWAD DST
    //      FUNCTION:       The two hex digits in the destination byte operand
    //                      are exchanged with each other,
    //      INDICATORS:     N = Set if (DST) bit 7 is set
    //                      Z = Set if (DST) = 0
    //                      V = Set if (DST) bit 7 is set
    //                      C = Reset
    //
    // NOTE - comments (and expected result) of 'cpu' diagnostic test 607 are
    //        not consistant with V = exclusive-or of N and C flags.
    //   "TSTCC 10010     ;** V should be set"
    //
    do_each("SWAD");
    tmp = wd11_cpu_state->getAMbyteBYmode(reg, mode, n1word);
    tmp2 = tmp >> 4;
    tmp3 = (tmp & 0x0f) << 4;
    tmp = tmp2 | tmp3;
    wd11_cpu_state->undAMbyteBYmode(reg, mode);
    wd11_cpu_state->putAMbyteBYmode(reg, mode, n1word, tmp);
    wd11_cpu_state->regs.PS.N = (tmp >> 7) & 1;
    wd11_cpu_state->regs.PS.Z = 0;
    if ((tmp & 0xff) == 0)
      wd11_cpu_state->regs.PS.Z = 1;
    /* ??? */ wd11_cpu_state->regs.PS.V = 0;
    wd11_cpu_state->regs.PS.C = 0;
    break;
  case 64:
    //      COMB            COMPLEMENT BYTE
    //      -------------------------------------------------------------
    //      FORMAT:         COMB DST
    //      OPERATION:      (DST) <- not (DST)
    //      FUNCTION:       The destination byte operand is one's complemented
    //      INDICATORS:     N = Set if (DST) bit 7 is set
    //                      Z = Set if (DST) = 0
    //                      V = Reset
    //                      C = Set
    //
    do_each("COMB");
    tmp = wd11_cpu_state->getAMbyteBYmode(reg, mode, n1word);
    tmp = (~tmp) & 0xff;
    wd11_cpu_state->undAMbyteBYmode(reg, mode);
    wd11_cpu_state->putAMbyteBYmode(reg, mode, n1word, tmp);
    wd11_cpu_state->regs.PS.N = (tmp >> 7) & 1;
    wd11_cpu_state->regs.PS.Z = 0;
    if (tmp == 0)
      wd11_cpu_state->regs.PS.Z = 1;
    wd11_cpu_state->regs.PS.V = 0;
    wd11_cpu_state->regs.PS.C = 1;
    break;
  case 65:
    //      NEGB            NEGATE BYTE
    //      -------------------------------------------------------------
    //      FORMAT:         NEGB DST
    //      OPERATION:      (DST) <- -(DST)
    //      FUNCTION:       The destination byte operand is two's complemented
    //      INDICATORS:     N = Set if (DST) bit 7 is set
    //                      Z = Set if (DST) = 0
    //                      V = Set if (DST) = "8000"
    //                      C = Reset if (DST) = 0
    //
    do_each("NEGB");
    tmp = wd11_cpu_state->getAMbyteBYmode(reg, mode, n1word);
    tmp = (-tmp) & 0xff;
    wd11_cpu_state->undAMbyteBYmode(reg, mode);
    wd11_cpu_state->putAMbyteBYmode(reg, mode, n1word, tmp);
    wd11_cpu_state->regs.PS.N = (tmp >> 7) & 1;
    wd11_cpu_state->regs.PS.V = 0;
    if (tmp == 0x80)
      wd11_cpu_state->regs.PS.V = 1;
    if (tmp == 0) {
      wd11_cpu_state->regs.PS.Z = 1;
      wd11_cpu_state->regs.PS.C = 0;
    } else {
      wd11_cpu_state->regs.PS.Z = 0;
      wd11_cpu_state->regs.PS.C = 1;
    }
    break;
  case 66:
    //      INCB            INCREMENT BYTE
    //      -------------------------------------------------------------
    //      FORMAT:         INCB DST
    //      OPERATION:      (DST) <- (DST) + 1
    //      FUNCTION:       The destination byte operand is incremnted by one
    //      INDICATORS:     N = Set if (DST) is set
    //                      Z = Set if (DST) = 0
    //                      V = Set if (DST) = "8000"
    //                      C = Set if Carry is generated from (DST) bit 7
    //
    do_each("INCB");
    tmp = wd11_cpu_state->getAMbyteBYmode(reg, mode, n1word);
    tmp2 = (tmp >> 7) & 1;
    tmp = (tmp + 1) & 0xff;
    wd11_cpu_state->undAMbyteBYmode(reg, mode);
    wd11_cpu_state->putAMbyteBYmode(reg, mode, n1word, tmp);
    wd11_cpu_state->regs.PS.N = (tmp >> 7) & 1;
    wd11_cpu_state->regs.PS.Z = 0;
    if (tmp == 0)
      wd11_cpu_state->regs.PS.Z = 1;
    wd11_cpu_state->regs.PS.V = 0;
    if (tmp == 0x80)
      wd11_cpu_state->regs.PS.V = 1;
    wd11_cpu_state->regs.PS.C = 0;
    if ((tmp2 == 1) && (wd11_cpu_state->regs.PS.N == 0))
      wd11_cpu_state->regs.PS.C = 1;
    break;
  case 67:
    //      DECB            DECREMENT BYTE
    //      -------------------------------------------------------------
    //      FORMAT:         DECB DST
    //      OPERATION:      (DST) <- (DST) - 1
    //      FUNCTION:       The destination byte operand is decremented by one
    //      INDICATORS:     N = Set if (DST) bit 7 is set
    //                      Z = Set if (DST) = 0
    //                      V = Set if (DST) = "7FFF"
    //                      C = Set if a borrow is generated from (DST) bit 7
    //
    do_each("DECB");
    tmp = wd11_cpu_state->getAMbyteBYmode(reg, mode, n1word);
    tmp2 = (tmp >> 7) & 1;
    tmp = (tmp - 1) & 0xff;
    wd11_cpu_state->undAMbyteBYmode(reg, mode);
    wd11_cpu_state->putAMbyteBYmode(reg, mode, n1word, tmp);
    wd11_cpu_state->regs.PS.N = (tmp >> 7) & 1;
    wd11_cpu_state->regs.PS.Z = 0;
    if (tmp == 0)
      wd11_cpu_state->regs.PS.Z = 1;
    wd11_cpu_state->regs.PS.V = 0;
    if (tmp == 0x7F)
      wd11_cpu_state->regs.PS.V = 1;
    wd11_cpu_state->regs.PS.C = 0;
    if ((tmp2 == 0) && (wd11_cpu_state->regs.PS.N == 1))
      wd11_cpu_state->regs.PS.C = 1;
    break;
  case 68:
    //
    //
    //      ------------- back to WORD OPS ------------------------------
    //
    //      LSTS            LOAD PROCESSOR STATUS
    //      -------------------------------------------------------------
    //      FORMAT:         LSTS DST
    //      FUNCTION:       The four indicators and the interrupt enable (I2)
    //                      are loaded from the destination operand.
    //      INDICATORS:     Set to the status of (DST) bits 0 - 3
    //                      NOTE: I2 will be (DST) bit 12.
    //
    do_each("LSTS");
    wd11_cpu_state->regs.gpr[8] = wd11_cpu_state->getAMwordBYmode(reg, mode, n1word);
    break;
  case 69:
    //      SSTS            STORE PROCESSOR STATUS
    //      -------------------------------------------------------------
    //      FORMAT:         SSTS DST
    //      FUNCTION:       The processor status word is fomd and stored in
    //      (DST), INDICATORS:     Unchanged
    //
    do_each("SSTS");
    wd11_cpu_state->putAMwordBYmode(reg, mode, n1word, wd11_cpu_state->regs.gpr[8]);
    break;
  case 70:
    //      ADC             ADD CARRY
    //      -------------------------------------------------------------
    //      FOBMAT:         ADC DST
    //      OPERATIQN:      (DST) <- (DST) + C-flag
    //      FUNCTION:       The carry flag is added to the destination operand
    //      INDICATOBS:     N = Set if (DST) bit 15 is set
    //                      Z = Set if (DST) = 0
    //                      V = Set to exclusive or of N and C flags
    //                      C = Set if a carry is generated from (DST) bit 15
    //
    // NOTE - comments (and expected result) of 'cpu' diagnostic test 517 and
    //        520 are not consistant with V = exclusive-or of N and C flags.
    //   "TSTCC 10005     ;** V should be set"
    //   "TSTCC 10010     ;** V should be set"
    //
    do_each("ADC");
    tmp = wd11_cpu_state->getAMwordBYmode(reg, mode, n1word);
    if (wd11_cpu_state->regs.PS.C == 1) {
      tmp = (tmp + 1) & 0xffff;
      wd11_cpu_state->regs.PS.C = 0;
      if (tmp == 0)
        wd11_cpu_state->regs.PS.C = 1;
    }
    wd11_cpu_state->undAMwordBYmode(reg, mode);
    wd11_cpu_state->putAMwordBYmode(reg, mode, n1word, tmp);
    wd11_cpu_state->regs.PS.N = (tmp >> 15) & 1;
    wd11_cpu_state->regs.PS.Z = 0;
    if (tmp == 0)
      wd11_cpu_state->regs.PS.Z = 1;
    wd11_cpu_state->regs.PS.V = wd11_cpu_state->regs.PS.C ^ wd11_cpu_state->regs.PS.N;
    /* ??? */ wd11_cpu_state->regs.PS.V = 0;
    break;
  case 71:
    //      SBC             SUBTBACT CARRY
    //      -------------------------------------------------------------
    //      FORMAT:         SBC DST
    //      OPERATION:      (DST) <- (DST) - C-flag
    //      FUNCTION:       The carry flag is subtracted from the destination
    //      operand INDICATORS:     N = Set if (DST) bit 15 is set
    //                      Z = Set if (DST) = 0
    //                      V = Set to exclusive or of N ahd C flags
    //                      C = Set if a borrow is generated from (DST) bit 15
    //
    do_each("SBC");
    tmp = wd11_cpu_state->getAMwordBYmode(reg, mode, n1word);
    tmp2 = (tmp >> 15) & 1;
    if (wd11_cpu_state->regs.PS.C == 1)
      tmp--;
    wd11_cpu_state->undAMwordBYmode(reg, mode);
    wd11_cpu_state->putAMwordBYmode(reg, mode, n1word, tmp);
    wd11_cpu_state->regs.PS.N = (tmp >> 15) & 1;
    wd11_cpu_state->regs.PS.Z = 0;
    if (tmp == 0)
      wd11_cpu_state->regs.PS.Z = 1;
    wd11_cpu_state->regs.PS.C = 0;
    if ((tmp2 == 0) && (wd11_cpu_state->regs.PS.N == 1))
      wd11_cpu_state->regs.PS.C = 1;
    wd11_cpu_state->regs.PS.V = wd11_cpu_state->regs.PS.C ^ wd11_cpu_state->regs.PS.N;
    break;
  default:
    assert("invalid return from fmt_7 lookup...");
    do_fmt_invalid();
    break;
  } /* end switch(op7) */

} /* end function do_fmt_7 */
