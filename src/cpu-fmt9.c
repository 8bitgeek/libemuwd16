/* cpu-fmt9.c         (c) Copyright Mike Noel, 2001-2008             */
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

#include "cpu-fmt9.h"

#define do_each(opc)                                                           \
  if (regs.tracing) {                                                          \
    if (op9 == 0)                                                              \
      trace_fmt9_jsr(opc, sreg, dmode, dreg, n1word);                          \
    else if (op9 == 1)                                                         \
      trace_fmt9_lea(opc, sreg, dmode, dreg, n1word);                          \
    else if (op9 == 3)                                                         \
      trace_fmt9_sob(opc, sreg, dmode, dreg);                                  \
    else                                                                       \
      trace_fmt9(opc, sreg, dmode, dreg, n1word);                              \
  }

void do_fmt_9() {
  int op9, sreg, splus, dmode, dreg, doffset, i, count;
  uint16_t n1word, tmp, tmp2;
  uint32_t big;

  //      FORMAT 9 OP CODES
  //      DOUBLE OPS - ONE OR TWO WORDS - SM0, DM0 to DM7
  //
  //      There are 8 op codes in this class representing op codes
  //      "7000" to "7FFF". Source mode 0 addressing only is allowed, but
  //      destination modes 0 - 7 are allowed for all op codes except 3: JSR and
  //      LEA with DM0 will cause an illegal instruction format trap (see
  //      chapter 2), and SOB is a special format unique to itself. It is
  //      included here only because its destination field is 6 bits long. SOB
  //      is a branch instruction. Its 6 bit destination field is a positive
  //      word offset from PC, which points to the op code that follows,
  //      backwards to the desired address. Forward branching is not allowed.
  //      SOB is always a one word op code, and it is used for fast loop
  //      control. All other op codes are one word long for DM0 to DM5
  //      addressing and two words long for DM6 or DM7 addressing. The rules for
  //      PC relative addressing with DM6 or DM7 are the same as they are for
  //      the format 7 op codes. Preliminary decoding of all these op codes
  //      except SOB presets the indicator flags as follows: N = 1, Z = 0, V =
  //      0, C = 1.
  //

  op9 = (op >> 9) & 7; /* 0-7 */
  dreg = op & 7;
  dmode = (op >> 3) & 7;
  sreg = (op >> 6) & 7;

  if (op9 != 3)
    if (dmode > 5) {
      getAMword((unsigned char *)&n1word, regs.PC);
      regs.PC += 2;
    }

  switch (op9) {
  case 0:
    //      JSR             JUMP TO SUBROUTINE
    //      -------------------------------------------------------------
    //      FORMAT:         JSR REG, DST
    //      OPERATION:      !SP, @SP <- REG
    //                      REG <- PC
    //                      PC  <- DST
    //      FUNCTION:       The linkage register is pushed onto the stack; PC,
    //                      which points to the op code that follows, is placed
    //                      in the linkage register; and the destination address
    //                      is placed in PC. DM0 is illegal. The assembler
    //                      recognizes the format "CALL DST" as being
    //                      equivalent to "JSR PC, DST".
    //      INDICATORS:     Preset
    //
    do_each("JSR");
    regs.PS.N = 1;
    regs.PS.Z = 0;
    regs.PS.V = 0;
    regs.PS.C = 1;
    if (dmode == 0) {
      do_fmt_invalid();
      break;
    }
    /* see app c */ tmp = getAMaddrBYmode(dreg, dmode, n1word);
    regs.SP -= 2;
    putAMword((unsigned char *)&regs.gpr[sreg], regs.SP);
    regs.gpr[sreg] = regs.PC;
    /* see app c */ regs.PC = tmp;
    break;
  case 1:
    //      LEA             LOAD EFFECTIVE ADDRESS
    //      -------------------------------------------------------------
    //      FORMAT:         LEA REG, DST
    //      OPERATION:      REG <- DST
    //      FUNCTION:       The destination address is placed into the source
    //                      register. DM@ is illegal. The assembler recognizes
    //                      the format "JMP DST" as being eguivalent to "LEA
    //                      PC,DST".
    //      INDICATORS:     Preset
    //
    do_each("LEA");
    regs.PS.N = 1;
    regs.PS.Z = 0;
    regs.PS.V = 0;
    regs.PS.C = 1;
    if (dmode == 0) {
      do_fmt_invalid();
      break;
    }
    regs.gpr[sreg] = getAMaddrBYmode(dreg, dmode, n1word);
    break;
  case 2:
    //      ASH             ABITHMETIC SHIFT
    //      -------------------------------------------------------------
    //      FORMAT:         ASH REG, DST
    //      FUNCTION:       The source register is shifted arithmtically with
    //                      the number of bits and direction specified by the
    //                      destination operand. If (DST) = 0 no shifttig
    //                      occurs. If (DST) = -X then REG is shifted right
    //                      arithamtically X bits as in an SSRA. If (DST) = +X
    //                      then REG is shifted left arithmetically X bits as in
    //                      an SSLA. Only an 8 bit destination operand is used.
    //                      Thus, DST is a byte address. For DM0 only the lower
    //                      byte of the destination register is used.
    //      INDICATORS:     Preset if (DST) = 0. Othemise:
    //                      N = Set if REG bit 15 is set
    //                      Z = Set if REG = 0
    //                      V = Set to exclusive or of N and C flags
    //                      C = Set to the value of the last bit shifted out of
    //                      REG
    //
    do_each("ASH");
    regs.PS.N = 1;
    regs.PS.Z = 0;
    regs.PS.V = 0;
    regs.PS.C = 1;
    /* ??? */ tmp = getAMwordBYmode(dreg, dmode, n1word);
    tmp = tmp & 255;
    if (tmp > 128) // SSRA
    {
      for (i = 0, count = 256 - tmp; i < count; i++) {
        tmp2 = (regs.gpr[sreg] >> 15) & 1;
        regs.PS.C = regs.gpr[sreg] & 1;
        regs.gpr[sreg] = regs.gpr[sreg] >> 1;
        if (tmp2 == 1)
          regs.gpr[sreg] |= 0x8000;
      }
      regs.PS.N = (regs.gpr[sreg] >> 15) & 1;
      regs.PS.Z = 0;
      if (regs.gpr[sreg] == 0)
        regs.PS.Z = 1;
      regs.PS.V = regs.PS.C ^ regs.PS.N;
    } else if (tmp > 0) // SSLA
    {
      for (i = 0, count = tmp; i < count; i++) {
        regs.PS.C = (regs.gpr[sreg] >> 15) & 1;
        regs.gpr[sreg] = regs.gpr[sreg] << 1;
      }
      regs.PS.N = (regs.gpr[sreg] >> 15) & 1;
      regs.PS.Z = 0;
      if (regs.gpr[sreg] == 0)
        regs.PS.Z = 1;
      regs.PS.V = regs.PS.C ^ regs.PS.N;
    }
    break;
  case 3:
    //      SOB             SUBTBACT ONE AND BRANCH (IF <> 0)
    //      -------------------------------------------------------------
    //      FORMAT:         SOB REG, DST
    //      OPERATION:      REG <- REG-1
    //                      IF REG <> 0, PC <- PC - (OFFSET *2)
    //      FUNCTION:       The source register is decremented by one. If the
    //                      result is not zero then twice the value of the des-
    //                      tination offset is subtracted from PC.
    //      INDICATORS:     Unchanged
    //
    do_each("SOB");
    if (--regs.gpr[sreg] != 0) {
      doffset = ((dmode << 3) + dreg) << 1;
      regs.PC -= doffset;
      if (regs.PC == opPC)
        if ((regs.PS.I2 == 1) && (regs.gpr[sreg] > 300)) { //
          // if this is an interrupt enabled branch to self
          // then cheat!  Wait a 1/100 a second and adjust
          // register to make him think he looped for it...
          //  ---- 4.5 monitor loop count is 400 ----
          //
          usleep(10000);
          if (regs.gpr[sreg] <= 2000)
            regs.gpr[sreg] = 1; // ~ 1500 for 2 mhz
          else                  // ~ 2500 for 3.3 mhz
            regs.gpr[sreg] -= 2000;
        }
    }
    break;
  case 4:
    //      XCH             EXCHANGE
    //      -------------------------------------------------------------
    //      FORMAT:         XCH REG, DST
    //      OPEBATION:      REG <--> (DST)
    //      FUNCTION:       The source register and destination contents are
    //                      exchanged with each other.
    //      INDICATORS:     Preset
    //
    do_each("XCH");
    regs.PS.N = 1;
    regs.PS.Z = 0;
    regs.PS.V = 0;
    regs.PS.C = 1;
    tmp = getAMwordBYmode(dreg, dmode, n1word);
    undAMwordBYmode(dreg, dmode);
    putAMwordBYmode(dreg, dmode, n1word, regs.gpr[sreg]);
    regs.gpr[sreg] = tmp;
    break;
  case 5:
    //      ASHC            ABITHMETIC SHIFT COMBINED
    //      -------------------------------------------------------------
    //      FORMAT:         ASHC REG, DST
    //      FUNCTION:       Exactly the same as ASH except that the shift is
    //      done
    //                      on REG+l:REG. All other comments apply.
    //      INDICATORS:     Preset if (DST) = 0. Otherwise:
    //                      N = Set if REG+1 bit 15 is set
    //                      Z = Set if REG+l: REG = 0
    //                      V = Reset
    //                      C = Set to the value of the last bit shifted out
    //
    // NOTE - comments (and expected result) of 'cpu' diagnostic test 1016 are
    //        not consistant with V = Reset.
    //   "TSTCC 10007     ;** V should not be set"
    //
    do_each("ASHC");
    regs.PS.N = 1;
    regs.PS.Z = 0;
    regs.PS.V = 0;
    regs.PS.C = 1;
    splus = (sreg + 1) % 8;
    /* ??? */ tmp = getAMwordBYmode(dreg, dmode, n1word);
    tmp = tmp & 255;
    if (tmp > 128) // SSRA
    {
      for (i = 0, count = 256 - tmp; i < count; i++) {
        regs.PS.C = regs.gpr[sreg] & 1;
        regs.gpr[sreg] = regs.gpr[sreg] >> 1;
        tmp2 = regs.gpr[splus] & 1;
        if (tmp2 == 1)
          regs.gpr[sreg] |= 0x8000;
        tmp2 = (regs.gpr[splus] >> 15) & 1;
        regs.gpr[splus] = regs.gpr[splus] >> 1;
        if (tmp2 == 1)
          regs.gpr[splus] |= 0x8000;
      }
      regs.PS.N = (regs.gpr[splus] >> 15) & 1;
      regs.PS.Z = 0;
      if ((regs.gpr[sreg] == 0) & (regs.gpr[splus] == 0))
        regs.PS.Z = 1;
      regs.PS.V = 0;
    } else if (tmp > 0) // SSLA
    {
      for (i = 0, count = tmp; i < count; i++) {
        regs.PS.C = (regs.gpr[splus] >> 15) & 1;
        regs.gpr[splus] = regs.gpr[splus] << 1;
        regs.gpr[splus] |= (regs.gpr[sreg] >> 15) & 1;
        regs.gpr[sreg] = regs.gpr[sreg] << 1;
      }
      regs.PS.N = (regs.gpr[splus] >> 15) & 1;
      regs.PS.Z = 0;
      if ((regs.gpr[sreg] == 0) & (regs.gpr[splus] == 0))
        regs.PS.Z = 1;
      /* ???????? */ regs.PS.V = regs.PS.Z & regs.PS.C;
    }
    break;
  case 6:
    //      MUL             MULTIPLY
    //      -------------------------------------------------------------
    //      FORMAT:         MUL REG, DST
    //      OPERATION:      REG+1:REG <- REG *(DST)
    //      FUNCTION:       An unsigned multiply is performed on the source
    //                      register and the destination operand. The unsigned
    //                      32 bit result is placed in REG+l:REG.
    //      INDICATORS:     N = Set if REG+l bit 15 is set
    //                      Z = Set if REG+l:REG = 0
    //                      v - Reset
    //                      C = Indeterminate
    //
    do_each("MUL");
    regs.PS.N = 1;
    regs.PS.Z = 0;
    regs.PS.V = 0;
    regs.PS.C = 1;
    splus = (sreg + 1) % 8;
    tmp = getAMwordBYmode(dreg, dmode, n1word);
    big = tmp * regs.gpr[sreg];
    regs.gpr[splus] = big >> 16;
    regs.gpr[sreg] = big & 0xffff;
    regs.PS.N = (regs.gpr[splus] >> 15) & 1;
    regs.PS.Z = 0;
    if ((regs.gpr[sreg] == 0) & (regs.gpr[splus] == 0))
      regs.PS.Z = 1;
    break;
  case 7:
    //      DIV             DIVIDE
    //      -------------------------------------------------------------
    //      FORMAT:         DIV REG, DST
    //      OPERATION:      REG   <- [REG+1:REG/(DST)]
    //                      REG+1 <- REMAINDER
    //      FUNCTION:       An unsigned divide is perfomed on the 32 bit source
    //                      operand REG+l:REG and the destination operand. The
    //                      unsigned result is placed in REG, and the unsigned
    //                      remainder is placed in REG+l.  No divide occurs and
    //                      the V-flag is set if REG+l is greater than or equal
    //                      to (DST) since the result will not fit into 16 bits.
    //                      If the divisor is zero both the V and C flags are
    //                      set.
    //      INDICATORS:     If no division error:
    //                      N = Set if REG bit 15 is set
    //                      Z = Set if REG = 0
    //                      V = Reset
    //                      C = Indeterminate
    //                      If division error:
    //                      N = Reset
    //                      Z = Reset
    //                      V = Set
    //                      C = Set if (DST) = 0
    //
    do_each("DIV");
    regs.PS.N = 1;
    regs.PS.Z = 0;
    regs.PS.V = 0;
    regs.PS.C = 1;
    splus = (sreg + 1) % 8;
    tmp = getAMwordBYmode(dreg, dmode, n1word);
    if (tmp == 0) // devide by zero...
    {
      regs.PS.V = 1;
      regs.PS.N = 0;
      regs.PS.C = 1;
    } else if (regs.gpr[splus] >= tmp) // overflow
    {
      regs.PS.V = 1;
      regs.PS.N = 0;
      regs.PS.C = 0;
    } else // do the devide...
    {
      big = (regs.gpr[splus] << 16) + regs.gpr[sreg];
      regs.gpr[sreg] = big / tmp;
      regs.gpr[splus] = big % tmp;
      regs.PS.N = (regs.gpr[sreg] >> 15) & 1;
      regs.PS.Z = 0;
      if (regs.gpr[sreg] == 0)
        regs.PS.Z = 1;
    }
    break;
  default:
    assert("invalid return from fmt_9 lookup...");
    do_fmt_invalid();
    break;
  } /* end switch(op9) */

} /* end function do_fmt_9 */
