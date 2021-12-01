/* cpu-fmt4.c         (c) Copyright Mike Noel, 2001-2008             */
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

#include "am-ddb.h"
#include "cpu-fmt4.h"

#define do_each(opc)                                                           \
  if (regs.tracing) {                                                          \
    if (op4 == 1)                                                              \
      trace_fmt4_svca(opc, arg);                                               \
    else if (op4 == 2)                                                         \
      trace_fmt4_svcb(opc, arg);                                               \
    else                                                                       \
      trace_fmt4_svcc(opc, arg);                                               \
  }

void do_fmt_4() {
  int op4, arg;
  U16 tmpa, tmpb;

  //      FORMAT 4 OP CODES
  //
  //      SINGLE WORD - 6 BIT NUMERIC ARGUMENT
  //
  //      There are 3 op codes in this class representing op codes
  //      "0040" to "00FF". All 3 are supervisor calls. All 3 are one
  //      word op codes with a 6 bit numeric argument.
  //

  arg = op & 63;
  op4 = op >> 6;

  switch (op4) {
  case 1:
    //      SVCA            SUPERVISOR CALL "A"
    //      -------------------------------------------------------------
    //      FORMAT:         SVCA ARG
    //      OPERATION:      !SP, @SP <- PS
    //                      !SP, @SP <- PC
    //                      PC <- (loc "22") + (ARG*2)
    //                      PC <- PC + @PC
    //      FUNCTION:       PS and PC are pushed onto the stack. The
    //                      contents of location "22" plus twice the value
    //                      of the argument (which is always positive) is placed
    //                      in PC to get the table address. The contents
    //                      of the table address is added to PC to get the
    //                      final destination address. Each table entry is the
    //                      relative offset from the start of the desired
    //                      routine to itself.
    //      INDICATORS:     Unchanged
    //
    do_each("SVCA");
    if (!svca_assist(arg)) {
      regs.SP -= 2;
      putAMword((unsigned char *)&regs.PS, regs.SP);
      regs.SP -= 2;
      putAMword((unsigned char *)&regs.PC, regs.SP);
      getAMword((unsigned char *)&regs.PC, 0x22);
      regs.PC += arg * 2;
      getAMword((unsigned char *)&tmpa, regs.PC);
      regs.PC += tmpa;
    }
    break;
  case 2:
    //      SVCB            SUPERVISOR CALL "B"
    //      -------------------------------------------------------------
    //      FORMAT:         SVCB ARG
    //                      SVCC ARG
    //      OPERATION:      TMPA <- SP
    //                      !SP, @SP <- PS
    //                      !SP, @SP <- PC
    //                      TMPB <- SP
    //                      !SP, @SP <- TMPA
    //                      SAVE
    //                      R1 <- TMPB
    //                      R5 <- ARG*2
    //                      PC <- (loc "24") for SVCB
    //                      PC <- (loc "26") for SVCC
    //      FUNCTION:       PS and PC are pushed onto the stack. The value
    //                      of SP at the start of op code execution is then
    //                      pushed followed by registers R5 to R0. The address
    //                      of the saved PC is placed in Rl, and twice the value
    //                      of the 6-bit positive argument is placed in R5. PC
    //                      is loaded from location "24" for SVCB or "26" for
    //                      SVCC.
    //      INDICATORS:     Unchanged
    //
    do_each("SVCB");
    if (!svcb_assist(arg)) {
      tmpa = regs.SP;
      regs.SP -= 2;
      putAMword((unsigned char *)&regs.PS, regs.SP);
      regs.SP -= 2;
      putAMword((unsigned char *)&regs.PC, regs.SP);
      tmpb = regs.SP;
      regs.SP -= 2;
      putAMword((unsigned char *)&tmpa, regs.SP);
      regs.SP -= 2;
      putAMword((unsigned char *)&regs.R5, regs.SP);
      regs.SP -= 2;
      putAMword((unsigned char *)&regs.R4, regs.SP);
      regs.SP -= 2;
      putAMword((unsigned char *)&regs.R3, regs.SP);
      regs.SP -= 2;
      putAMword((unsigned char *)&regs.R2, regs.SP);
      regs.SP -= 2;
      putAMword((unsigned char *)&regs.R1, regs.SP);
      regs.SP -= 2;
      putAMword((unsigned char *)&regs.R0, regs.SP);
      regs.R1 = tmpb;
      regs.R5 = arg * 2;
      getAMword((unsigned char *)&regs.PC, 0x24);
    }
    break;
  case 3:
    //      SVCC            SUPERVISOR CALL "C"
    //      -------------------------------------------------------------
    //      see SVCB above.  SVCC is like SVCB but final PC loc is "26"
    //                       for SVCC instead of "24" as for SVCB.
    //
    do_each("SVCC");
    if (!svcc_assist(arg)) {
      tmpa = regs.SP;
      regs.SP -= 2;
      putAMword((unsigned char *)&regs.PS, regs.SP);
      regs.SP -= 2;
      putAMword((unsigned char *)&regs.PC, regs.SP);
      tmpb = regs.SP;
      regs.SP -= 2;
      putAMword((unsigned char *)&tmpa, regs.SP);
      regs.SP -= 2;
      putAMword((unsigned char *)&regs.R5, regs.SP);
      regs.SP -= 2;
      putAMword((unsigned char *)&regs.R4, regs.SP);
      regs.SP -= 2;
      putAMword((unsigned char *)&regs.R3, regs.SP);
      regs.SP -= 2;
      putAMword((unsigned char *)&regs.R2, regs.SP);
      regs.SP -= 2;
      putAMword((unsigned char *)&regs.R1, regs.SP);
      regs.SP -= 2;
      putAMword((unsigned char *)&regs.R0, regs.SP);
      regs.R1 = tmpb;
      regs.R5 = arg * 2;
      getAMword((unsigned char *)&regs.PC, 0x26);
    }
    break;
  default:
    assert("cpu-fmt4.c - invalid return from fmt_4 lookup");
    do_fmt_invalid();
  } /* end switch(op4) */

} /* end function do_fmt_4 */

//
// these are the functions for 'hardware assist' of AMOS services
// they return 'true'  if they handled the call,
//             'false' if the AMOS monitor service routine is to be used
//

int svca_assist(int arg) {
  if (arg == 9) { // turn off user trace on exit...
    if (regs.utrace) {
      getAMword((unsigned char *)&regs.utRX, 0x4E); // JOBCUR
      if (regs.utR0 == regs.utRX) {
        regs.tracing = false;
        regs.utrace = false;
      }
    }
  }
  return (false);
}

int svcb_assist(int arg) { return (false); }

int svcc_assist(int arg) {

  if (cpu4_svcctxt[0] == 'h')
    arg = 63 - arg;

  if (arg == 0) { // entry to virtual disk driver
    vdkdvr();
    return (true);
  }
  if (arg == 1) { // turn tracing off
    regs.tracing = false;
    regs.stepping = false;
    return (true);
  }
  if (arg == 2) { // turn tracing on
    regs.tracing = true;
    return (true);
  }
  if (arg == 4) { // turn user tracing on
    if (!regs.utrace)
      getAMword((unsigned char *)&regs.utR0, 0x4E); // JOBCUR
    getAMword((unsigned char *)&regs.utPC, 0x46);   // MEMBAS
    regs.utrace = true;
    regs.tracing = true;
    regs.R0 = regs.utR0;
    return (true);
  }
  if (arg == 5) { // turn user tracing off
    regs.utrace = false;
    regs.tracing = false;
    regs.stepping = false;
    return (true);
  }
  if (arg == 6) { // turn tracing on with stepping
    regs.tracing = true;
    regs.stepping = true;
    fprintf(stderr, "\n\rYou have entered single step mode.  ");
    fprintf(stderr, "ALT-S to step.  ");
    fprintf(stderr, "ALT-R to resume..\n\r");
    return (true);
  }
  if (arg == 7) { // snap JOBBAS thru JOBSIZ to trace
    U16 LINK, R0, SIZE;
    getAMword((unsigned char *)&R0, 0x4E);      // JOBCUR
    getAMword((unsigned char *)&LINK, R0 + 12); // JOBBAS
    getAMword((unsigned char *)&SIZE, R0 + 14); // JOBSIZ
    fprintf(stderr, "\n\r<><>SVCC 7 memory dump<><>\n\r");
    config_memdump(LINK, SIZE);
    return (true);
  }
  // if (arg == 8) { // special snap of particular memory block to trace
  //      U16 LINK, R0, SIZE;
  //      getAMword((unsigned char *)&R0,   0x4E);  // JOBCUR
  //      getAMword((unsigned char *)&LINK, R0+12); // JOBBAS
  //      fprintf(stderr,"\n\r<><>SVCC 8 memory dump<><>\n\r");
  //        getAMword((unsigned char *)&SIZE, LINK); LINK += SIZE; // s.b.
  //        link=464e getAMword((unsigned char *)&SIZE, LINK); LINK += SIZE; //
  //        s.b. link=4858 getAMword((unsigned char *)&SIZE, LINK); LINK +=
  //        SIZE; // s.b. link=4a62 getAMword((unsigned char *)&SIZE, LINK);
  //        LINK += SIZE; // s.b. link=4c6c getAMword((unsigned char *)&SIZE,
  //        LINK); // s.b. LINK= 4c6c, SIZE = 20e
  //      config_memdump(LINK, SIZE);
  //      return(true);
  //      }
  if (arg == 9) { // shutdown!
    regs.halting = true;
    return (true);
  }
  return (false);
}
