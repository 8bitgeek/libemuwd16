/* cpu-fmt11.c         (c) Copyright Mike Noel, 2001-2008            */
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

#include "cpu-fmt11.h"
#include <math.h>

/*-------------------------------------------------------------------*/
/* Structure definition for floating point data first word       */
/*-------------------------------------------------------------------*/
typedef struct {
#if AM_BYTE_ORDER == AM_BIG_ENDIAN
  uint16_t S : 1, /* sign                      */
      EXP : 8,    /* exponent                  */
      MSB1 : 7;   /* most sign. bits           */
#else
  uint16_t MSB1 : 7, /* most sign. bits           */
      EXP : 8,       /* exponent                  */
      S : 1;         /* sign                      */
#endif
} AFP1;

/*-------------------------------------------------------------------*/
/* Structure definition for floating point data          */
/*-------------------------------------------------------------------*/
typedef union {
#if AM_BYTE_ORDER == AM_BIG_ENDIAN
  uint64_t AFP_LL;
  struct {
    uint16_t AFP_highbits;
    AFP1 AFP_1;
    uint16_t AFP_2;
    uint16_t AFP_3;
  } words;
#else
  uint64_t AFP_LL;
  struct {
    uint16_t AFP_3;
    uint16_t AFP_2;
    AFP1 AFP_1;
    uint16_t AFP_highbits;
  } words;
#endif
} AFP;

/*-------------------------------------------------------------------*/
/* Subroutine to get Alpha float into gcc float (double)       */
/*-------------------------------------------------------------------*/
void afp_get(AFP *afp, double *d) {
  AFP afp_copy;
  double d_copy;
  int s, exp, nnn;

  afp_copy = *afp;

  if (afp_copy.words.AFP_1.EXP == 0) {
    *d = 0.0;
    return;
  }
  s = afp_copy.words.AFP_1.S;
  afp_copy.words.AFP_1.S = 0;
  exp = afp_copy.words.AFP_1.EXP - 128;
  afp_copy.words.AFP_1.EXP = 1;
  afp_copy.words.AFP_highbits = 0;
  d_copy = afp_copy.AFP_LL;
  if (s == 1)
    d_copy *= -1;
  d_copy = frexp(d_copy, &nnn); /* nnn is throw away */
  d_copy = ldexp(d_copy, exp);
  *d = d_copy;
}

/*-------------------------------------------------------------------*/
/* Subroutine to get gcc float (double) into Alpha float       */
/*-------------------------------------------------------------------*/
int afp_put(
    AFP *afp,
    double *d) { // makes afp from d; returns 0 if good, 1 if over, -1 if under
  AFP afp_copy;
  double d_copy;
  int s, exp;

  d_copy = *d;
  afp_copy = *afp;
  afp_copy.AFP_LL = 0;
  if (d_copy == 0) { // exp = -128 case here...
    *afp = afp_copy;
    return (0);
  }
  s = 0;
  if (d_copy < 0) {
    s = 1;
    d_copy *= -1;
  }
  d_copy = frexp(d_copy, &exp);
  afp_copy.AFP_LL = d_copy * pow(2, 40);
  afp_copy.words.AFP_1.EXP = exp + 128;
  afp_copy.words.AFP_1.S = s;
  *afp = afp_copy;
  if (exp > 127)
    return (1);
  if (exp < -127) // -128 case above...
    return (-1);
  return (0);
}

/*-------------------------------------------------------------------*/
/* MACRO - trace opcode                */
/*-------------------------------------------------------------------*/
#define do_each(opc)                                                           \
  if (wd16_cpu_state->regs.tracing)                                                            \
    wd16_cpu_state->trace_fmt11(opc, sind, sreg, s, dind, dreg, d);

/*-------------------------------------------------------------------*/
/* MACRO - standard floating point error trap          */
/*-------------------------------------------------------------------*/
#define FP_trap                                                                \
  wd16_cpu_state->regs.SP -= 2;                                                                \
  wd16_cpu_state->putAMword((unsigned char *)&wd16_cpu_state->regs.PS, wd16_cpu_state->regs.SP);                               \
  wd16_cpu_state->regs.SP -= 2;                                                                \
  wd16_cpu_state->putAMword((unsigned char *)&wd16_cpu_state->regs.PC, wd16_cpu_state->regs.SP);                               \
  wd16_cpu_state->getAMword((unsigned char *)&wd16_cpu_state->regs.PC, 0x3E);

/*-------------------------------------------------------------------*/
/* Fmt 11 entry for floating point instructions          */
/*-------------------------------------------------------------------*/
void do_fmt_11(wd16_cpu_state_t* wd16_cpu_state) {
  int op11, sind, smode, sreg, dind, dmode, dreg, oflg;
  uint16_t saddr, daddr;
  AFP afp_s, afp_d, afp_r;
  double s, d, r;

  //      FORMAT 11 OP CODES
  //
  //      DOUBLE OPS - ONE WORD - FLOATING POINT.
  //
  //      There are 16 OP Codes in this class representing OP "F000" to
  //      "FFFF". Only five are currently defined. They reside in the third
  //      microm along with the Format 8 OP Codes. The remaining 11 OP Codes
  //      are mapped to the fourth microm for future expansion or customised
  //      user OP Codes. All are one word long. TWo source and destination
  //      addressing modes are available. These two modes, FP0 and FP1, are
  //      unique to these OP Codes. Each consists of a 3-bit Register
  //      Designation and a 1 bit indirect flag preceeding the register
  //      designator. For FP0 the indirect bit is 0, and FP1 it is one. Both the
  //      source and destination fields have both addressing modes. The modes
  //      are defined as follows:
  //
  //      FP0     The designated register contains the address of the operand.
  //
  //      FP1     The designated register contains the address of the address
  //              of the operand.
  //
  //      FP0     is the same as standard addressing mode 1, and FP1 the same
  //      as standard addressing mode 7 with an offset of zero.
  //
  //      The computed address is the address of the first word of a 3 word
  //      floating point operand. The first word contains the sign, exponent,
  //      and high byte of the mantissa. The next higher address contains the
  //      middle two bytes of the mantissa, and the next higher address after
  //      that contains the lowest two bytes of the mantissa. This format is
  //      half way between single and double precision floating point formats,
  //      and it represents the most efficient use of microprocessor ROM and
  //      register space. The complete format is as follows:
  //
  //      1. A 1 bit sign for the entire number which is zero for positive.
  //
  //      2. An 8 bit base-two exponent in excess-128 notation with a range of
  //      +127, -128. The only legal number with an exponent of -128 is
  //      true zero (all zeros).
  //
  //      3. A 40 bit mantissa with the MSB implied.
  //
  //      Since every operand is assumad to be normalised upon entry end every
  //      result is normalized before storage in the destination addresses,
  //      and since a normalised mantissa has a MSB equal to one, then only 39
  //      bits need to be stored. The MSB is implied to be a one, and the
  //      bit position it normally occupies is taken over by the exponent to
  //      increase its range by a factor of two. The full format of a floating
  //      point operand is a follows:
  //
  //      LOCATION X:     bit 15 = sign, bit 14-7 exponent, bit 6-0 mantissa
  //      (high) LOCATION X+2:   bit 15-0 mantissa (middle) LOCATION X+4:   bit
  //      15-0 mantissa (low)
  //
  //      True zero is represented by a field of 48 zeroes. In effect, the CPU
  //      considers any number with an exponent of all zeroes (-128) to be a
  //      zero during multiplication and division. For add and subtract the only
  //      legal number with an exponent of -128 is true zero. All others cause
  //      erroneous results. No registers are modified by any Format 11 OP Code.
  //      However, to make room internally for computations 4 registers are
  //      saved in memory locations "30" - "38" during the execution of FADD,
  //      FSDB, FMDL and.FDIV. These registers are retrieved at the completion
  //      of the OP Codes. The registers saved are: the destination address, SP,
  //      PC and R0. No Format 11 OP Code is interruptable (for obvious
  //      reasons). FMUL uses location "38" for temporary storage of partial
  //      results.
  //
  //      FLOATING POINT ERROR TRAPS
  //
  //      Location "3E" is defined as the floating point error trap PC. Whenever
  //      an overflow, underflow, or divide by zero occurs a standard trap
  //      call is executed with PS and PC pushed onto the stack, and PC fetched
  //      from location "3E". I2 is not altered. The remaining memory locations
  //      that are reserved for the floating point option ("3A and "3C") are
  //      not currently used. The status of the indicator flags and destination
  //      addresses during the 3 trap conditions are defined as follows:
  //
  //                      FOR UNDERFLOW (FADD, FSUB, FMUL, FDIV)
  //                      N = l Destination contains all zeroes
  //                      N = 0 (true zero).
  //                      V = l
  //                      C = 0
  //
  //                      FOR OVERFLOW (FADD, FSUB, FMUL)
  //                      N = 0 Destination not altered in any way.
  //                      Z = 0
  //                      V = l
  //                      C = 0
  //
  //                      FOR OVER FLOW (FDIV)
  //                      N = 0 Destination not altered if overflow detected
  //                      Z = 0 during exponent computation. Undefined
  //                      V = l otherwise. (Used to save unnormalized
  //                      C = 0 partial results during a divide).
  //
  //                      FOR DIVIDE BY ZERO (FDIV)
  //                      N = l Destination not altered in any way.
  //                      Z = 0
  //                      V = l
  //                      C = l
  //
  //      RESERVED TRAPS
  //
  //      If the third microm is in the system and the fourth is not then the
  //      last 11 floating point OP codes are the only ones that will cause a
  //      reserved OP code trap if executed. If the third microm is not in the
  //      system then all Format 8 and 11 OP Codes will cause a reserved OP code
  //      trap if executed. However, since the Format 8 OP Codes are interrupt-
  //      able the PC is not advance until the completion of the moves. In
  //      all other cases PC is advanced when the OP Code is fetched. For
  //      these reasons the PC that is saved onto the stack will point to the
  //      offending OP Code during a reserved OP Code trap if and only if
  //      the offending OP Code is a Format 8 OP Code. For the Format 11
  //      OP Codes the saved PC will point to the OP Code that follows the
  //      offending OP Code. If the User wishes to identify which Op Code
  //      caused the reserved OP Code trap he must not preceed a Format 8
  //      OP Code with a Format 11 OP Code or a literal that looks like a
  //      Format 11 OP Code.
  //
  //
  //      CAUTION: The same physical operand may be used as both the source and
  //               destination operand for any of the above floating point OP
  //               Codes with no abnormal results except two. They are:
  //               1) If an error trap occurs the operand will probably be
  //               altered. 2) An FSUB gives an answer of -2x, if x <> 0,
  //               instead of 0.
  //

  op11 = (wd16_cpu_state->op >> 8) & 15; /* 0-15, but 5-15 invalid */
  dreg = wd16_cpu_state->op & 7;
  dind = (wd16_cpu_state->op >> 3) & 1;
  sreg = (wd16_cpu_state->op >> 4) & 7;
  sind = (wd16_cpu_state->op >> 7) & 1;

  if (sind == 0)
    smode = 1;
  else
    smode = 7;
  saddr = wd16_cpu_state->getAMaddrBYmode(sreg, smode, 0);
  wd16_cpu_state->getAMword((unsigned char *)&afp_s.words.AFP_1, saddr);
  if (op11 == 1) {
    afp_s.words.AFP_1.S = ~afp_s.words.AFP_1.S;
    wd16_cpu_state->putAMword((unsigned char *)&afp_s.words.AFP_1, saddr);
  }
  wd16_cpu_state->getAMword((unsigned char *)&afp_s.words.AFP_2, saddr + 2);
  wd16_cpu_state->getAMword((unsigned char *)&afp_s.words.AFP_3, saddr + 4);
  afp_get(&afp_s, &s);
  if (dind == 0)
    dmode = 1;
  else
    dmode = 7;
  daddr = wd16_cpu_state->getAMaddrBYmode(dreg, dmode, 0);
  wd16_cpu_state->getAMword((unsigned char *)&afp_d.words.AFP_1, daddr);
  wd16_cpu_state->getAMword((unsigned char *)&afp_d.words.AFP_2, daddr + 2);
  wd16_cpu_state->getAMword((unsigned char *)&afp_d.words.AFP_3, daddr + 4);
  afp_get(&afp_d, &d);

  switch (op11) {
  case 1:
    //      FSUB            FLOATING POINT SUBTRACT
    //      -------------------------------------------------------------
    //      FORMAT:         FSUB SRC, DST
    //      OPERATION:      (DST) <- (DST) - (SRC)
    //      FUNCTION:       The source operand is subtracted from the
    //                      destinaticn operand. The result is normalized
    //                      and stored in place of the destination operand.
    //
    //      WARNING : THIS OP CODE COMPLIMENTS THE SIGN OF THE SOURCE OPERAND IN
    //      MEMORY AND DOES AN FADD.
    //
    //      INDICATORS:     (if no errors)
    //                      N = Set if the result sign is negative (set).
    //                      Z = Set if the result is zero
    //                      V = Reset
    //                      C = Reset
    //
    do_each("FSUB");
  case 0:
    //      FADD            FLOATING POINT ADD
    //      -------------------------------------------------------------
    //      FORMAT:         FADD SRC, DST
    //      OPERATION:      (DST) <- (DST) + (SRC)
    //      FUNCTION:       The source and destination operands are added
    //                      together, normalized, and the result is stored
    //                      in place of the destination operand.
    //      INDICATORS:     (if no errors)
    //                      N = Set if the result sign is negative (set).
    //                      Z = Set if the result is zero
    //                      V = Reset
    //                      C = Reset
    //
    if (op11 == 0)
      do_each("FADD");
    r = d + s;
    wd16_cpu_state->regs.PS.C = wd16_cpu_state->regs.PS.V = wd16_cpu_state->regs.PS.Z = wd16_cpu_state->regs.PS.N = 0;
    oflg = afp_put(&afp_r, &r);
    if (oflg > 0) {
      wd16_cpu_state->regs.PS.V = 1;
      FP_trap;
      break;
    }
    wd16_cpu_state->putAMword((unsigned char *)&afp_r.words.AFP_1, daddr);
    wd16_cpu_state->putAMword((unsigned char *)&afp_r.words.AFP_2, daddr + 2);
    wd16_cpu_state->putAMword((unsigned char *)&afp_r.words.AFP_3, daddr + 4);
    if (oflg < 0) {
      wd16_cpu_state->regs.PS.N = wd16_cpu_state->regs.PS.V = 1;
      FP_trap;
      break;
    }
    if (r < 0)
      wd16_cpu_state->regs.PS.N = 1;
    if (r == 0)
      wd16_cpu_state->regs.PS.Z = 1;
    break;
  case 2:
    //      FMUL            FLOATING POINT MULTIPLY
    //      -------------------------------------------------------------
    //      FORMAT:         FMUL SRC, DST
    //      OPERATION:      (DST) <- (DST) * (SRC)
    //      FUNCTION:       The source and destination operands are multiplied
    //                      together, nomalized, and the result is
    //                      stored in place of the destination operand.
    //      INDICATORS:     (if no errors)
    //                      N = Set if the sign of the result is negative (set).
    //                      Z = Set if the result is zero
    //                      V = Reset
    //                      C = Reset
    //
    do_each("FMUL");
    r = d * s;
    wd16_cpu_state->regs.PS.C = wd16_cpu_state->regs.PS.V = wd16_cpu_state->regs.PS.Z = wd16_cpu_state->regs.PS.N = 0;
    oflg = afp_put(&afp_r, &r);
    if (oflg > 0) {
      wd16_cpu_state->regs.PS.V = 1;
      FP_trap;
      break;
    }
    wd16_cpu_state->putAMword((unsigned char *)&afp_r.words.AFP_1, daddr);
    wd16_cpu_state->putAMword((unsigned char *)&afp_r.words.AFP_2, daddr + 2);
    wd16_cpu_state->putAMword((unsigned char *)&afp_r.words.AFP_3, daddr + 4);
    if (oflg < 0) {
      wd16_cpu_state->regs.PS.N = wd16_cpu_state->regs.PS.V = 1;
      FP_trap;
      break;
    }
    if (r < 0)
      wd16_cpu_state->regs.PS.N = 1;
    if (r == 0)
      wd16_cpu_state->regs.PS.Z = 1;
    break;
  case 3:
    //      FDIV            FLOATING POINT DIVIDE
    //      -------------------------------------------------------------
    //      FORMAT:         FDIV SRV, DST
    //      OPERATION:      (DST) <- (DST) / (SRC)
    //      FUNCTION:       The destination operand is divided by the source
    //                      operand. The result is normalized and stored in
    //                      place of the destination operand.
    //      INDICATORS:     (if no errors)
    //                      N = Set if the sign of the result is negative (set).
    //                      Z = Set if the result is zero
    //                      V = Reset
    //                      C = Reset
    //
    do_each("FDIV");
    if (s == 0) {
      wd16_cpu_state->regs.PS.C = wd16_cpu_state->regs.PS.V = wd16_cpu_state->regs.PS.N = 1;
      wd16_cpu_state->regs.PS.Z = 0;
      FP_trap;
      break;
    }
    r = d / s;
    oflg = afp_put(&afp_r, &r);
    wd16_cpu_state->regs.PS.C = wd16_cpu_state->regs.PS.V = wd16_cpu_state->regs.PS.Z = wd16_cpu_state->regs.PS.N = 0;
    if (oflg > 0) {
      wd16_cpu_state->regs.PS.V = 1;
      FP_trap;
      break;
    }
    wd16_cpu_state->putAMword((unsigned char *)&afp_r.words.AFP_1, daddr);
    wd16_cpu_state->putAMword((unsigned char *)&afp_r.words.AFP_2, daddr + 2);
    wd16_cpu_state->putAMword((unsigned char *)&afp_r.words.AFP_3, daddr + 4);
    if (oflg < 0) {
      wd16_cpu_state->regs.PS.N = wd16_cpu_state->regs.PS.V = 1;
      FP_trap;
      break;
    }
    if (r < 0)
      wd16_cpu_state->regs.PS.N = 1;
    if (r == 0)
      wd16_cpu_state->regs.PS.Z = 1;
    break;
  case 4:
    //      FCMP            FLOATING POINT COMPARE
    //      -------------------------------------------------------------
    //      FORMAT:         FCMP SRC, DST
    //      OPERATION:      (SRC) - (DST)
    //      FUNCTION:       The destination operand is compared to the source
    //                      operand, and the indicators are set to allow
    //                      a SIGNED conditional branch.
    //      INDICATORS:     N = Set if result is negative
    //                      Z = Set if result is zero
    //                      V = Set if arithmetic underflow occurs. *
    //                      C = Set if a borrow is generated. *
    //
    //      * NOTE : True if first words of both operands are not equal
    //
    do_each("FCMP");

    wd16_cpu_state->regs.PS.C = wd16_cpu_state->regs.PS.V = wd16_cpu_state->regs.PS.Z = wd16_cpu_state->regs.PS.N = 0;
    if (d == s)
      wd16_cpu_state->regs.PS.Z = 1;
    if (d > s) {
      wd16_cpu_state->regs.PS.N = 1;
      if (memcmp(&afp_d.words.AFP_1, &afp_s.words.AFP_1, 2) != 0)
        wd16_cpu_state->regs.PS.C = 1;
    }

    break;
  default:
    do_fmt_invalid();
    break;
  } /* end switch(op11) */

  wd16_cpu_state->putAMword((unsigned char *)&daddr, 0x30); // fill 'save area'...
  wd16_cpu_state->putAMword((unsigned char *)&wd16_cpu_state->regs.SP, 0x32);
  wd16_cpu_state->putAMword((unsigned char *)&wd16_cpu_state->regs.PC, 0x34);
  wd16_cpu_state->putAMword((unsigned char *)&wd16_cpu_state->regs.R0, 0x36);
  wd16_cpu_state->putAMword((unsigned char *)&saddr, 0x38); /* real doesn't def... */

} /* end function do_fmt_11 */
