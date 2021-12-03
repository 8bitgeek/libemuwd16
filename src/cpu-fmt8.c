/* cpu-fmt8.c         (c) Copyright Mike Noel, 2001-2008             */
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

#include "cpu-fmt8.h"

#define do_each(opc)                                                           \
  if (wd11_cpu_state->regs.tracing)                                                            \
    trace_fmt8(opc, sreg, dreg);

void do_fmt_8(wd11_cpu_state_t* wd11_cpu_state) {
  int op8, sreg, dreg;
  uint16_t t16;
  uint8_t t8;

  //       FORMAT 8 OP CODES
  //       DOUBLE OPS - SINGLE WORD - SM0 AND DM0 ONLY
  //

  //      There are 8 op codes in this class representing op codes
  //      "0E00" to "0FFF". Only addressing mode 0 is allowed for both the
  //      source and destination. All are one word op codes, and all are block
  //      move instructions. The last 4 can be used as pseudo DMA ops in some
  //      hardware configurations. In all cases the source register contains
  //      the address of the first word or byte of memory to be moved, and the
  //      destination register contains the address of the first word or byte
  //      of memory to receive the data being moved. The number of words or
  //      bytes being moved is contained in R0. The count ranges from 1-65536
  //      (0 = 65536) words or bytes. The count in R0 is an unsigned positive
  //      integer. None of the indicators are altered by these op codes.
  //
  //      Each of these op codes is interruptable at the end of each word
  //      or byte transfer. If no interrupt requests are active the transfers
  //      continue. PC is not incremented to the next op code until the
  //      op code is completed. This allows for complete interruptability
  //      as long as register integrity is maintained during the interrupt.

  op8 = (wd11_cpu_state->op >> 6) - 55; /* 1-8 */
  dreg = wd11_cpu_state->op & 7;
  sreg = (wd11_cpu_state->op >> 3) & 7;

  switch (op8) {
  case 1:
    //      MBWU            MOVE BLOCK OF WORDS UP
    //      -------------------------------------------------------------
    //      FORMAT:         MBWU SRC, DST
    //      FUNCTION:       The word string beginning with the word addressed
    //                      by the source register is moved to successively
    //                      increasing word addresses as specified by the
    //                      destination register. The source and destination
    //                      registers are each incremented by two after each
    //                      word is transferred. R0 is decremnted by one after
    //                      each transfer, and transfers continue until R0 = 0.
    //      INDICATORS:     Unchanged.
    //
    do_each("MBWU");
    do {
      t16 = wd11_cpu_state->getAMwordBYmode(sreg, 1, 0);
      wd11_cpu_state->putAMwordBYmode(dreg, 1, 0, t16);
      wd11_cpu_state->regs.gpr[sreg] += 2;
      wd11_cpu_state->regs.gpr[dreg] += 2;
      wd11_cpu_state->regs.gpr[0] -= 1;
    } while ((wd11_cpu_state->regs.gpr[0] != 0) & !(wd11_cpu_state->regs.PS.I2 & wd11_cpu_state->regs.intpending));
    if (wd11_cpu_state->regs.gpr[0] != 0)
      wd11_cpu_state->regs.PC -= 2; // then do it again!
    break;
  case 2:
    //      MBWD            MOVE BLOCK OF WORDS DOWN
    //      -------------------------------------------------------------
    //      FORMAT:         MBWD SRC, DST
    //      FUNCTION:       The word string beginning with the word addressed
    //                      by the source register is moved to successively
    //                      decreasing word addresses as specified by the
    //                      destination register. The source and destination
    //                      registers are each decremented by two after each
    //                      word is transferred. R0 is decremented by one after
    //                      each transfer, and transfers continue until R0 = 0.
    //      INDICATORS:     Unchanged.
    //
    do_each("MBWD");
    do {
      t16 = wd11_cpu_state->getAMwordBYmode(sreg, 1, 0);
      wd11_cpu_state->putAMwordBYmode(dreg, 1, 0, t16);
      wd11_cpu_state->regs.gpr[sreg] -= 2;
      wd11_cpu_state->regs.gpr[dreg] -= 2;
      wd11_cpu_state->regs.gpr[0] -= 1;
    } while ((wd11_cpu_state->regs.gpr[0] != 0) & !(wd11_cpu_state->regs.PS.I2 & wd11_cpu_state->regs.intpending));
    if (wd11_cpu_state->regs.gpr[0] != 0)
      wd11_cpu_state->regs.PC -= 2; // then do it again!
    break;
  case 3:
    //      MBBU            MOVE BLOCK OF BYTES UP
    //      -------------------------------------------------------------
    //      FORMAT:         MBBU SRC, DST
    //      FUNCTION:       The byte string beginning with the byte addressed by
    //                      the source register is moved to successively
    //                      increasing byte addresses as specified by the
    //                      destination register. The source and destination
    //                      registers are each incremsnted by one after each
    //                      byte is transferred. R0 is decremented by one after
    //                      each transfer, and transfers continue until R0 = 0.
    //      IXDICATORS:     Unchanged.
    //
    //
    do_each("MBBU");
    do {
      t8 = wd11_cpu_state->getAMbyteBYmode(sreg, 1, 0);
      wd11_cpu_state->putAMbyteBYmode(dreg, 1, 0, t8);
      wd11_cpu_state->regs.gpr[sreg] += 1;
      wd11_cpu_state->regs.gpr[dreg] += 1;
      wd11_cpu_state->regs.gpr[0] -= 1;
    } while ((wd11_cpu_state->regs.gpr[0] != 0) & !(wd11_cpu_state->regs.PS.I2 & wd11_cpu_state->regs.intpending));
    if (wd11_cpu_state->regs.gpr[0] != 0)
      wd11_cpu_state->regs.PC -= 2; // then do it again!
    break;
  case 4:
    //      MBBD            MOVE BLOCK OF BYTES DOWN
    //      -------------------------------------------------------------
    //      FORMAT:         MBBD SRC, DST
    //      FUNCTION:       The byte string beginning with the byte addressed by
    //                      the source register is moved to successively
    //                      decreasing byte addresses as specified by the
    //                      destination register. The source register,
    //                      destination register, and R0 are each decremented by
    //                      one after each byte is transferred. Transfers
    //                      continue until R0 = 0.
    //      INDICATORS:     Unchanged.
    //
    do_each("MBBD");
    do {
      t8 = wd11_cpu_state->getAMbyteBYmode(sreg, 1, 0);
      wd11_cpu_state->putAMbyteBYmode(dreg, 1, 0, t8);
      wd11_cpu_state->regs.gpr[sreg] -= 1;
      wd11_cpu_state->regs.gpr[dreg] -= 1;
      wd11_cpu_state->regs.gpr[0] -= 1;
    } while ((wd11_cpu_state->regs.gpr[0] != 0) & !(wd11_cpu_state->regs.PS.I2 & wd11_cpu_state->regs.intpending));
    if (wd11_cpu_state->regs.gpr[0] != 0)
      wd11_cpu_state->regs.PC -= 2; // then do it again!
    break;
  case 5:
    //      MBWA            MOVE BLOCK OF WORDS TO ADDRESS
    //      -------------------------------------------------------------
    //      FORMAT:         MBWA SRC, DST
    //      FUNCTION:       Same as MBWU except that the destination register is
    //                      never incremented.
    //      INDICATORS:     Unchanged.
    //
    do_each("MBWA");
    do {
      t16 = wd11_cpu_state->getAMwordBYmode(sreg, 1, 0);
      wd11_cpu_state->putAMwordBYmode(dreg, 1, 0, t16);
      wd11_cpu_state->regs.gpr[sreg] += 2;
      wd11_cpu_state->regs.gpr[0] -= 1;
    } while ((wd11_cpu_state->regs.gpr[0] != 0) & !(wd11_cpu_state->regs.PS.I2 & wd11_cpu_state->regs.intpending));
    if (wd11_cpu_state->regs.gpr[0] != 0)
      wd11_cpu_state->regs.PC -= 2; // then do it again!
    break;
  case 6:
    //      MBBA            MOVE BLOCK OF BYTES TO ADDRESS
    //      -------------------------------------------------------------
    //      FORMAT:         MBBA SRC, DST
    //      FUNCTION:       Same as MBBU except that the destination register is
    //                      never incremented.
    //      INDICATORS:     Unchanged.
    //
    do_each("MBBA");
    do {
      t8 = wd11_cpu_state->getAMbyteBYmode(sreg, 1, 0);
      wd11_cpu_state->putAMbyteBYmode(dreg, 1, 0, t8);
      wd11_cpu_state->regs.gpr[sreg] += 1;
      wd11_cpu_state->regs.gpr[0] -= 1;
    } while ((wd11_cpu_state->regs.gpr[0] != 0) & !(wd11_cpu_state->regs.PS.I2 & wd11_cpu_state->regs.intpending));
    if (wd11_cpu_state->regs.gpr[0] != 0)
      wd11_cpu_state->regs.PC -= 2; // then do it again!
    break;
  case 7:
    //      MABW            MOVE ADDRESS TO BLOCK OF WORDS
    //      -------------------------------------------------------------
    //      FORMAT:         MABW SRC, DST
    //      FUNCTION:       Same as MBWU except that the source register is
    //      never
    //                      incremented.
    //      INDICATORS:     Unchanged.
    //
    do_each("MABW");
    do {
      t16 = wd11_cpu_state->getAMwordBYmode(sreg, 1, 0);
      wd11_cpu_state->putAMwordBYmode(dreg, 1, 0, t16);
      wd11_cpu_state->regs.gpr[dreg] += 2;
      wd11_cpu_state->regs.gpr[0] -= 1;
    } while ((wd11_cpu_state->regs.gpr[0] != 0) & !(wd11_cpu_state->regs.PS.I2 & wd11_cpu_state->regs.intpending));
    if (wd11_cpu_state->regs.gpr[0] != 0)
      wd11_cpu_state->regs.PC -= 2; // then do it again!
    break;
  case 8:
    //      MABB            MOVE ADDRESS TO BLOCK OF BYTES
    //      -------------------------------------------------------------
    //      FORMAT:         MABB SRC, DST
    //      FUNCTION:       Same as MBBU except that the source register is
    //      never
    //                      incremented.
    //      INDICATORS:     Unchanged.
    //
    do_each("MABB");
    do {
      t8 = wd11_cpu_state->getAMbyteBYmode(sreg, 1, 0);
      wd11_cpu_state->putAMbyteBYmode(dreg, 1, 0, t8);
      wd11_cpu_state->regs.gpr[dreg] += 1;
      wd11_cpu_state->regs.gpr[0] -= 1;
    } while ((wd11_cpu_state->regs.gpr[0] != 0) & !(wd11_cpu_state->regs.PS.I2 & wd11_cpu_state->regs.intpending));
    if (wd11_cpu_state->regs.gpr[0] != 0)
      wd11_cpu_state->regs.PC -= 2; // then do it again!
    break;
  default:
    assert("invalid return from fmt_8 lookup...");
    do_fmt_invalid();
    break;
  } /* end switch(op8) */

} /* end function do_fmt_8 */
