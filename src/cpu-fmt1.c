/* cpu-fmt1.c         (c) Copyright Mike Noel, 2001-2008             */
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

#include "cpu-fmt1.h"

#define do_each(opc)                                                           \
  if (wd11_cpu_state->regs.tracing)                                                            \
    trace_fmt1(opc, mask);

void do_fmt_1(wd11_cpu_state_t* wd11_cpu_state) {
  unsigned tmp;
  uint16_t mask, oldmask, newop;

  //      FORMAT 1 OP CODES
  //
  //      Single word - no arguments
  //
  //      There are 16 op codes in this class representing op codes "0000" to
  //      "000F". Each is a one word op code with no arguments with the
  //      exception of the SAVS op code which is a two word op code. Word two of
  //      the SAVS op code is the I/O priority interrupt mask.
  //
  switch (wd11_cpu_state->op) {
  case 0:
    //      NOP             NO OPERATION
    //      -------------------------------------------------------------
    //      FORMAT:         NOP
    //      FUNCTION:       No operations are performed
    //      INDICATORS:     Unchanged
    //
    do_each("NOP");
    break;
  case 1:
    //      RESET           I/O RESET
    //      -------------------------------------------------------------
    //      FORMAT:         RESET
    //      FUNCTION:       An I/O reset pulse is transmitted
    //      INDICATORS:     Unchanged
    //
    do_each("RESET");
    break;
  case 2:
    //      IEN             INTERRUPT ENABLE
    //      -------------------------------------------------------------
    //      FORMAT:         IEN
    //      FUNCTION:       The interrupt enable (12) flag is set.  Allows
    //                      one more instruction so execute before inter-
    //                      rupts are recognized.
    //      INDICATORS:     Unchanged
    //
    do_each("IEN");
    wd11_cpu_state->regs.PS.I2 = 1;
    break;
  case 3:
    //      IDS             INTERRUPT DISABLE
    //      -------------------------------------------------------------
    //      FORMAT:         IDS
    //      FUNCTION:       The interrupt enable (I2) flag is reset.
    //                      This instruction can honor interrupts, but
    //                      the I2 bit in the PS that is stored on the stack
    //                      is reset if an interrupt occurs.*
    //      INDICATORS:     Unchanged
    //
    //      *NOTE: on some machines I2 will be set or reset during the IEN or
    //             IDS . If so the change will be valid immediately, not one op
    //             code later.
    //
    do_each("IDS");
    wd11_cpu_state->regs.PS.I2 = 0;
    break;
  case 4:
    //      HALT            HALT
    //      -------------------------------------------------------------
    //      FORMAT:         HALT
    //      FUNCTION:       Tests the status of the Power Fail bit in the
    //                      external status register. If the bit is set it
    //                      is assumed that the HALT occured in a power fail
    //                      routine, and the following operations occur:
    //                      1) The interrupt enable (I2) flag is reset
    //                      2) The CPU waits until the Power Fail bit is reset
    //                      3) PC is fetched from location "16", and program
    //                         execution begins at this new location
    //                      If the power fail bit is reset then the CPU waits
    //                      until the halt switch (I3) is set. At that time
    //                      the selected halt option (see chapter 2) is executed
    //                      The interrupt enable flag is also reset.
    //      INDICATORS:     Unchanged
    //
    do_each("HALT");
    wd11_cpu_state->regs.PS.I2 = 0;
    wd11_cpu_state->regs.halting = 1;
    break;
  case 5:
    //      XCT             EXECUTE SINGLE INSTRUCTION
    //      -------------------------------------------------------------
    //      FORMAT:         XCT
    //      OPERATION:      PC <- @SP, SP ^
    //                      PS <- @SP, SP ^
    //                      Trace flag set, execute op code
    //                      !SP, @SP <- PS
    //                      !SP, @SP <- PC
    //                      Trace flag reset
    //                      PC <- (loc "20") if no error
    //                      PC <- (loc "1E") if error
    //      FUNCTION:       PC and PS are popped from the stack, but I2 is not
    //                      altered. The trace flag, which disables all inter-
    //                      rupts except I3, is set. The op code is executed
    //                      PS and PC are pushed back onto the stack, and PC
    //                      is fetched from location "20". The trace flag is
    //                      reset. If the program tries to execute a HALT, XCT,
    //                      BPT, or WFI the attempt is aborted, PS and PC are
    //                      pushed onto the stack, and PC is fetched from
    //                      location "1E" instead.  I2 is also reset.
    //      INDICATORS:     Depends upon executed op code
    //
    do_each("XCT");
    tmp = wd11_cpu_state->regs.PS.I2;
    getAMword((unsigned char *)&wd11_cpu_state->regs.PC, wd11_cpu_state->regs.SP);
    wd11_cpu_state->regs.SP += 2;
    getAMword((unsigned char *)&wd11_cpu_state->regs.PS, wd11_cpu_state->regs.SP);
    wd11_cpu_state->regs.SP += 2;

    getAMword((unsigned char *)&newop, wd11_cpu_state->regs.PC);
    if ((newop > 3) && (newop < 8)) /* HALT, XCT, BPT, or WFI */
                                    /* ???? */
    {
      wd11_cpu_state->regs.PC += 2; /* and stacked PS should be smashed too */
      wd11_cpu_state->regs.SP -= 2;
      putAMword((unsigned char *)&wd11_cpu_state->regs.PC, wd11_cpu_state->regs.SP);
      wd11_cpu_state->regs.SP -= 2;
      putAMword((unsigned char *)&wd11_cpu_state->regs.PC, wd11_cpu_state->regs.SP);
      getAMword((unsigned char *)&wd11_cpu_state->regs.PC, 0x1E);
      wd11_cpu_state->regs.PS.I2 = 0;
    } else { /* execute_instruction will refetch op */
      wd11_cpu_state->regs.PS.I2 = tmp;
      wd11_cpu_state->regs.trace = 1;
      execute_instruction();
      wd11_cpu_state->regs.trace = 0;
      wd11_cpu_state->regs.SP -= 2;
      putAMword((unsigned char *)&wd11_cpu_state->regs.PS, wd11_cpu_state->regs.SP);
      wd11_cpu_state->regs.SP -= 2;
      putAMword((unsigned char *)&wd11_cpu_state->regs.PC, wd11_cpu_state->regs.SP);
      getAMword((unsigned char *)&wd11_cpu_state->regs.PC, 0x20);
    }
    break;
  case 6:
    //      BPT             BREAKPOINT TRAP
    //      -------------------------------------------------------------
    //      FORMAT:         BPT
    //      OPERATION:      !SP, @SP <- PS
    //                      !SP, @SP <- PC
    //                      PC <- (loc "2c")
    //      FUNCTION:       PS and PC are pushed onto the stack. PC is
    //                      fetched from location "2C"
    //      INDICATORS:     Unchanged
    //
    do_each("BPT");
    wd11_cpu_state->regs.SP -= 2;
    putAMword((unsigned char *)&wd11_cpu_state->regs.PS, wd11_cpu_state->regs.SP);
    wd11_cpu_state->regs.SP -= 2;
    putAMword((unsigned char *)&wd11_cpu_state->regs.PC, wd11_cpu_state->regs.SP);
    getAMword((unsigned char *)&wd11_cpu_state->regs.PC, 0x2C);
    break;
  case 7:
    //      WFI             WAIT FOR INTERRUPT
    //      -------------------------------------------------------------
    //      FORMAT:         WFI
    //      FUNCTION:       The CPU loops internally without accessing
    //                      the data bus until an interrupt occurs. Program
    //                      execution continues with the op code that follows
    //                      the WFI after the interrupt has been serviced.
    //                      The interrupt enable flag is also set.
    //      INDICATORS:     Unchanged
    //
    do_each("WFI");
    if (wd11_cpu_state->regs.intpending != 1) {
      usleep(500);
      wd11_cpu_state->regs.PS.I2 = 0;
      wd11_cpu_state->regs.PC -= 2;
    }
    wd11_cpu_state->regs.PS.I2 = 1;
    break;
  case 8:
    //      RSVC            RETURN FROM SUPERVISOR CALL (B or C)
    //      -------------------------------------------------------------
    //      FORMAT:         RSVC
    //      OPERATION:      REST
    //                      SP^
    //                      RTT
    //      FUNCTION:       Registers R0 to R5, PC and PS are popped from
    //                      the stack with the saved SP bypassed.
    //      INDICATORS:     Set per PS bits 0 - 3
    //
    do_each("RSVC");
    getAMword((unsigned char *)&wd11_cpu_state->regs.R0, wd11_cpu_state->regs.SP);
    wd11_cpu_state->regs.SP += 2;
    getAMword((unsigned char *)&wd11_cpu_state->regs.R1, wd11_cpu_state->regs.SP);
    wd11_cpu_state->regs.SP += 2;
    getAMword((unsigned char *)&wd11_cpu_state->regs.R2, wd11_cpu_state->regs.SP);
    wd11_cpu_state->regs.SP += 2;
    getAMword((unsigned char *)&wd11_cpu_state->regs.R3, wd11_cpu_state->regs.SP);
    wd11_cpu_state->regs.SP += 2;
    getAMword((unsigned char *)&wd11_cpu_state->regs.R4, wd11_cpu_state->regs.SP);
    wd11_cpu_state->regs.SP += 2;
    getAMword((unsigned char *)&wd11_cpu_state->regs.R5, wd11_cpu_state->regs.SP);
    wd11_cpu_state->regs.SP += 2;
    wd11_cpu_state->regs.SP += 2;
    getAMword((unsigned char *)&wd11_cpu_state->regs.PC, wd11_cpu_state->regs.SP);
    wd11_cpu_state->regs.SP += 2;
    getAMword((unsigned char *)&wd11_cpu_state->regs.PS, wd11_cpu_state->regs.SP);
    wd11_cpu_state->regs.SP += 2;
    break;
  case 9:
    //      RRTT            RESTORE AND RETURN FROM TRAP
    //      -------------------------------------------------------------
    //      FORMAT :        RRTT
    //      OPERATION:      REST
    //                      RTT
    //      FUNCTION:       Registers R0 to R5, PC and PS are popped
    //                      from the stack.
    //      INDICATORS:     Set per PS bits 0 - 3
    //
    do_each("RRTT");
    getAMword((unsigned char *)&wd11_cpu_state->regs.R0, wd11_cpu_state->regs.SP);
    wd11_cpu_state->regs.SP += 2;
    getAMword((unsigned char *)&wd11_cpu_state->regs.R1, wd11_cpu_state->regs.SP);
    wd11_cpu_state->regs.SP += 2;
    getAMword((unsigned char *)&wd11_cpu_state->regs.R2, wd11_cpu_state->regs.SP);
    wd11_cpu_state->regs.SP += 2;
    getAMword((unsigned char *)&wd11_cpu_state->regs.R3, wd11_cpu_state->regs.SP);
    wd11_cpu_state->regs.SP += 2;
    getAMword((unsigned char *)&wd11_cpu_state->regs.R4, wd11_cpu_state->regs.SP);
    wd11_cpu_state->regs.SP += 2;
    getAMword((unsigned char *)&wd11_cpu_state->regs.R5, wd11_cpu_state->regs.SP);
    wd11_cpu_state->regs.SP += 2;
    getAMword((unsigned char *)&wd11_cpu_state->regs.PC, wd11_cpu_state->regs.SP);
    wd11_cpu_state->regs.SP += 2;
    getAMword((unsigned char *)&wd11_cpu_state->regs.PS, wd11_cpu_state->regs.SP);
    wd11_cpu_state->regs.SP += 2;
    break;
  case 10:
    //      SAVE            SAVE REGISTERS
    //      -------------------------------------------------------------
    //      FORMAT :        SAVE
    //      OPERATION:      !SP, @SP <- R5
    //                      !SP, @SP <- R4
    //                      !SP, @SP <- R3
    //                      !SP, @SP <- R2
    //                      !SP, @SP <- R1
    //                      !SP, @SP <- R0
    //      FUNCTION:       Registers R5 to R0 are pushed onto the stack.
    //      INDICATORS:     Unchanged.
    //
    do_each("SAVE");
    wd11_cpu_state->regs.SP -= 2;
    putAMword((unsigned char *)&wd11_cpu_state->regs.R5, wd11_cpu_state->regs.SP);
    wd11_cpu_state->regs.SP -= 2;
    putAMword((unsigned char *)&wd11_cpu_state->regs.R4, wd11_cpu_state->regs.SP);
    wd11_cpu_state->regs.SP -= 2;
    putAMword((unsigned char *)&wd11_cpu_state->regs.R3, wd11_cpu_state->regs.SP);
    wd11_cpu_state->regs.SP -= 2;
    putAMword((unsigned char *)&wd11_cpu_state->regs.R2, wd11_cpu_state->regs.SP);
    wd11_cpu_state->regs.SP -= 2;
    putAMword((unsigned char *)&wd11_cpu_state->regs.R1, wd11_cpu_state->regs.SP);
    wd11_cpu_state->regs.SP -= 2;
    putAMword((unsigned char *)&wd11_cpu_state->regs.R0, wd11_cpu_state->regs.SP);
    break;
  case 11:
    //      SAVS            SAVE STATUS
    //      -------------------------------------------------------------
    //      FORMAT:         SAVS MASK
    //      OPERATION:      SAVE
    //                      !SP, @SP <- (loc "2E")
    //                      (loc "2E") < (loc "2E") OR mask
    //                      MSKO
    //                      IEN
    //      FORMAT:         Registers R5 to R0 and the priority mask in location
    //                      "2E" are pushed onto the stack. The old and new
    //                      masks are OR'd together and placed in locatian "2E".
    //                      A mask out state code (see appendix D) is
    //                      transmitted and the interrupt enable (I2) flag is
    //                      set.
    //      INDICATORS:     Unchanged
    //
    getAMword((unsigned char *)&mask, wd11_cpu_state->regs.PC);
    wd11_cpu_state->regs.PC += 2;
    do_each("SAVS"); /* done here so 'mask' avail */
    wd11_cpu_state->regs.SP -= 2;
    putAMword((unsigned char *)&wd11_cpu_state->regs.R5, wd11_cpu_state->regs.SP);
    wd11_cpu_state->regs.SP -= 2;
    putAMword((unsigned char *)&wd11_cpu_state->regs.R4, wd11_cpu_state->regs.SP);
    wd11_cpu_state->regs.SP -= 2;
    putAMword((unsigned char *)&wd11_cpu_state->regs.R3, wd11_cpu_state->regs.SP);
    wd11_cpu_state->regs.SP -= 2;
    putAMword((unsigned char *)&wd11_cpu_state->regs.R2, wd11_cpu_state->regs.SP);
    wd11_cpu_state->regs.SP -= 2;
    putAMword((unsigned char *)&wd11_cpu_state->regs.R1, wd11_cpu_state->regs.SP);
    wd11_cpu_state->regs.SP -= 2;
    putAMword((unsigned char *)&wd11_cpu_state->regs.R0, wd11_cpu_state->regs.SP);
    getAMword((unsigned char *)&oldmask, 0x2E);
    wd11_cpu_state->regs.SP -= 2;
    putAMword((unsigned char *)&oldmask, wd11_cpu_state->regs.SP);
    oldmask = mask | oldmask;
    putAMword((unsigned char *)&oldmask, 0x2E);
    // --------------   mask0?
    wd11_cpu_state->regs.PS.I2 = 1;
    break;
  case 12:
    //      REST            RESTORE REGISTERS
    //      -------------------------------------------------------------
    //      FORMAT:         REST
    //      OPERATION:      R0 <- @SP, SP^
    //                      R1 <- @SP, SP^
    //                      R2 <- @SP, SP^
    //                      R3 <- @SP, SP^
    //                      R4 <- @SP, SP^
    //                      R5 <- @SP, SP^
    //      FUNCTION:       Registers R0 to R5 are popped from the stack,
    //      INDICATORS:     Unchanged
    //
    do_each("REST");
    getAMword((unsigned char *)&wd11_cpu_state->regs.R0, wd11_cpu_state->regs.SP);
    wd11_cpu_state->regs.SP += 2;
    getAMword((unsigned char *)&wd11_cpu_state->regs.R1, wd11_cpu_state->regs.SP);
    wd11_cpu_state->regs.SP += 2;
    getAMword((unsigned char *)&wd11_cpu_state->regs.R2, wd11_cpu_state->regs.SP);
    wd11_cpu_state->regs.SP += 2;
    getAMword((unsigned char *)&wd11_cpu_state->regs.R3, wd11_cpu_state->regs.SP);
    wd11_cpu_state->regs.SP += 2;
    getAMword((unsigned char *)&wd11_cpu_state->regs.R4, wd11_cpu_state->regs.SP);
    wd11_cpu_state->regs.SP += 2;
    getAMword((unsigned char *)&wd11_cpu_state->regs.R5, wd11_cpu_state->regs.SP);
    wd11_cpu_state->regs.SP += 2;
    break;
  case 13:
    //      RRTN            RESTORE AND RETURN FROM SUBROUTINE
    //      -------------------------------------------------------------
    //      FORMAT:         RRTN
    //      OPERATION:      REST
    //                      PC <- @SP, SP^
    //      FUNCTION:       Registers R0 to R5 and PC are popped
    //                      from the stack
    //      INDICATORS:     Unchanged
    //
    do_each("RRTN");
    getAMword((unsigned char *)&wd11_cpu_state->regs.R0, wd11_cpu_state->regs.SP);
    wd11_cpu_state->regs.SP += 2;
    getAMword((unsigned char *)&wd11_cpu_state->regs.R1, wd11_cpu_state->regs.SP);
    wd11_cpu_state->regs.SP += 2;
    getAMword((unsigned char *)&wd11_cpu_state->regs.R2, wd11_cpu_state->regs.SP);
    wd11_cpu_state->regs.SP += 2;
    getAMword((unsigned char *)&wd11_cpu_state->regs.R3, wd11_cpu_state->regs.SP);
    wd11_cpu_state->regs.SP += 2;
    getAMword((unsigned char *)&wd11_cpu_state->regs.R4, wd11_cpu_state->regs.SP);
    wd11_cpu_state->regs.SP += 2;
    getAMword((unsigned char *)&wd11_cpu_state->regs.R5, wd11_cpu_state->regs.SP);
    wd11_cpu_state->regs.SP += 2;
    getAMword((unsigned char *)&wd11_cpu_state->regs.PC, wd11_cpu_state->regs.SP);
    wd11_cpu_state->regs.SP += 2;
    break;
  case 14:
    //      RSTS            RESTORE STATUS
    //      -------------------------------------------------------------
    //      FORMAT:         RSTS
    //      OPERATION:      (loc "2E") <- @SP, SP^
    //                      MSKO
    //                      REST
    //                      RTT
    //      FUNCTION:       The priority mask is popped from the stack and
    //                      restored to locaton "2E". A MASK OUT state code
    //                      (See Appendix D) is transmitted. Registers R0
    //                      to R5, PC and PS are popped from the stack.
    //      INDICATORS:     Set per PS bits 0 - 3
    //
    do_each("RSTS");
    getAMword((unsigned char *)&mask, wd11_cpu_state->regs.SP);
    wd11_cpu_state->regs.SP += 2;
    putAMword((unsigned char *)&mask, 0x2E);
    // --------------   mask0?
    getAMword((unsigned char *)&wd11_cpu_state->regs.R0, wd11_cpu_state->regs.SP);
    wd11_cpu_state->regs.SP += 2;
    getAMword((unsigned char *)&wd11_cpu_state->regs.R1, wd11_cpu_state->regs.SP);
    wd11_cpu_state->regs.SP += 2;
    getAMword((unsigned char *)&wd11_cpu_state->regs.R2, wd11_cpu_state->regs.SP);
    wd11_cpu_state->regs.SP += 2;
    getAMword((unsigned char *)&wd11_cpu_state->regs.R3, wd11_cpu_state->regs.SP);
    wd11_cpu_state->regs.SP += 2;
    getAMword((unsigned char *)&wd11_cpu_state->regs.R4, wd11_cpu_state->regs.SP);
    wd11_cpu_state->regs.SP += 2;
    getAMword((unsigned char *)&wd11_cpu_state->regs.R5, wd11_cpu_state->regs.SP);
    wd11_cpu_state->regs.SP += 2;
    getAMword((unsigned char *)&wd11_cpu_state->regs.PC, wd11_cpu_state->regs.SP);
    wd11_cpu_state->regs.SP += 2;
    getAMword((unsigned char *)&wd11_cpu_state->regs.PS, wd11_cpu_state->regs.SP);
    wd11_cpu_state->regs.SP += 2;
    break;
  case 15:
    //      RTT             RETURN FROM TRAP
    //      -------------------------------------------------------------
    //      FORMAT:         RTT
    //      OPERATION:      PC <- @SP, SP^
    //                      PS <- @SP, SP^
    //      FUNCTION:       PC and PS are popped from stack
    //      INDICATORS:     N = Set per PS bit 3
    //                      Z - Set per PS bit 2
    //                      V = Set per PS bit 1
    //                      C = Set per PS bit 0
    //
    do_each("RTT");
    getAMword((unsigned char *)&wd11_cpu_state->regs.PC, wd11_cpu_state->regs.SP);
    wd11_cpu_state->regs.SP += 2;
    getAMword((unsigned char *)&wd11_cpu_state->regs.PS, wd11_cpu_state->regs.SP);
    wd11_cpu_state->regs.SP += 2;
    break;
  default:
    assert("cpu-fmt1.c - invalid return from fmt_1 lookup");
    do_fmt_invalid();
  } /* end switch(op) */

} /* end function do_fmt_1 */
