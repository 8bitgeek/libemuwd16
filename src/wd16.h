/* am100.h       (c) Copyright Mike Noel, 2001-2008                  */
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

#ifndef __WESTERN_DIGITAL_WD16_H__
#define __WESTERN_DIGITAL_WD16_H__

#include <ctype.h>
#include <errno.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <assert.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define AM_LITTLE_ENDIAN __ORDER_LITTLE_ENDIAN__
#define AM_BIG_ENDIAN __ORDER_BIG_ENDIAN__
#define AM_BYTE_ORDER __BYTE_ORDER__

/* Platform-independent storage operand definitions */
typedef uint8_t HWORD[2];
typedef uint8_t FWORD[4];
typedef uint8_t DWORD[8];

/*-------------------------------------------------------------------*/
/* Structure definition for PS context                               */
/*-------------------------------------------------------------------*/
typedef struct _PS {                    /* Status Word        */

#if AM_BYTE_ORDER == AM_BIG_ENDIAN
  uint16_t pwrfail : 1,                 /* 1=power is failing        */
      buserror : 1,                     /* 1=problem on the bus      */
      parityerr : 1,                    /* 1=problem with memory     */
      I2 : 1,                           /* 1=Interrupt enabled       */
      haltjmp2 : 1,                     /* - hardware jumper set     */
      haltjmp1 : 1,                     /* - hardware jumper set     */
      pwrjmp2 : 1,                      /* - hardware jumper set     */
      pwrjmp1 : 1,                      /* - hardware jumper set     */
      ALU : 4,                          /* not relevent to pgmr?     */
      N : 1,                            /* 1=MSB is set              */
      Z : 1,                            /* 1=zero                    */
      V : 1,                            /* 1=overflow                */
      C : 1;                            /* 1=carry                   */
#else
  uint16_t C : 1,                       /* 1=carry                   */
      V : 1,                            /* 1=overflow                */
      Z : 1,                            /* 1=zero                    */
      N : 1,                            /* 1=MSB is set              */
      ALU : 4,                          /* not relevent to pgmr?     */
      pwrjmp1 : 1,                      /* - hardware jumper set     */
      pwrjmp2 : 1,                      /* - hardware jumper set     */
      haltjmp1 : 1,                     /* - hardware jumper set     */
      haltjmp2 : 1,                     /* - hardware jumper set     */
      I2 : 1,                           /* 1=Interrupt enabled       */
      parityerr : 1,                    /* 1=problem with memory     */
      buserror : 1,                     /* 1=problem on the bus      */
      pwrfail : 1;                      /* 1=power is failing        */
#endif

} TPS;

/*-------------------------------------------------------------------*/
/* Structure definition for CPU register context                     */
/*-------------------------------------------------------------------*/
typedef struct _REGS {                  /* Processor registers       */
  uint64_t instcount;                   /* Instruction counter       */
  uint16_t *gpr;                        /* addressing of registers   */
  int16_t *spr;                         /* addressing of signed regs */
  uint16_t R0;                          /* aka gpr[0]                */
  uint16_t R1;                          /* aka gpr[1]                */
  uint16_t R2;                          /* aka gpr[2]                */
  uint16_t R3;                          /* aka gpr[3]                */
  uint16_t R4;                          /* aka gpr[4]                */
  uint16_t R5;                          /* aka gpr[5]                */
  uint16_t SP;                          /* aka gpr[6]                */
  uint16_t PC;                          /* aka gpr[7]                */
  TPS PS;                               /* aka gpr[8]                */
                                        /*                           */
  int trace;                            /* XCT trace flag            */
                                        /*                           */
  int halting;                          /* halting flag              */
  int BOOTing;                          /* monitor BOOTing flag      */
  int stepping;                         /* single stepping flag      */
  int tracing;                          /* inst. tracing flag        */
  int utrace;                           /* user space trace flag     */
  uint16_t utR0;                        /* utrace JOBCUR initial     */
  uint16_t utRX;                        /* utrace JOBCUR match       */
  uint16_t utPC;                        /* utrace MEMBAS match       */
  int waiting;                          /* waiting flag              */
  int intpending;                       /* interrupt pending flag    */
  int whichint[9];                      /* which int is pending?     */
                                        /* 0=nv, 1-8 vectored        */
  unsigned char LED;                    /* Diagnostic LED            */

} REGS;

/*-------------------------------------------------------------------*/
/* memory accesss callback typedefs                                  */
/*-------------------------------------------------------------------*/

// void getAMbyte(unsigned char *chr, long address);
// void putAMbyte(unsigned char *chr, long address);
typedef void (*get_put_byte_callback_t)(unsigned char *chr, long address);
typedef void (*get_put_word_callback_t)(unsigned char *chr, long address);

// void getAMword(unsigned char *chr, long address);
// void putAMword(unsigned char *chr, long address);
typedef void (*get_put_byte_callback_t)(unsigned char *chr, long address);
typedef void (*get_put_word_callback_t)(unsigned char *chr, long address);

// uint16_t  getAMaddrBYmode(int regnum, int mode, int offset);
// uint16_t  getAMwordBYmode(int regnum, int mode, int offset);
typedef uint16_t (*get_put_word_by_mode_callback_t)(int regnum, int mode, int offset);

// uint8_t   getAMbyteBYmode(int regnum, int mode, int offset);
typedef uint8_t (*get_byte_by_mode_callback_t)(int regnum, int mode, int offset);

// void   undAMwordBYmode(int regnum, int mode);
// void   undAMbyteBYmode(int regnum, int mode);
typedef void (*und_by_mode_callback_t)(int regnum, int mode);

// void   putAMwordBYmode(int regnum, int mode, int offset, uint16_t theword);
typedef void (*put_word_by_mode_callback_t)(int regnum, int mode, int offset, uint16_t theword);

// void   putAMbyteBYmode(int regnum, int mode, int offset, uint8_t thebyte);
typedef void (*put_byte_by_mode_callback_t)(int regnum, int mode, int offset, uint8_t thebyte);

// void   trace_fmt1(char *opc, int mask);
// void   trace_fmt2(char *opc, int reg);
// void   trace_fmt3(char *opc, int arg);
// void   trace_fmt4_svca(char *opc, int arg);
// void   trace_fmt4_svcb(char *opc, int arg);
// void   trace_fmt4_svcc(char *opc, int arg);
// void   trace_fmt5(char *opc, int dest);
typedef void (*trace_fmt_A_callback_t)(char *opc, int mask);

// void   trace_fmt6(char *opc, int count, int reg);
// void   trace_fmt8(char *opc, int sreg, int dreg);
typedef void (*trace_fmt_B_callback_t)(char *opc, int count, int reg);

// void   trace_fmt7(char *opc, int dmode, int dreg, uint16_t n1word);
typedef void (*trace_fmt_C_callback_t)(char *opc, int dmode, int dreg, uint16_t n1word);

// void   trace_fmt9(char *opc, int sreg, int dmode, int dreg, uint16_t n1word);
// void   trace_fmt9_jsr(char *opc, int sreg, int dmode, int dreg, uint16_t n1word);
// void   trace_fmt9_lea(char *opc, int sreg, int dmode, int dreg, uint16_t n1word);
typedef void (*trace_fmt_D_callback_t)(char *opc, int sreg, int dmode, int dreg, uint16_t n1word);

// void   trace_fmt9_sob(char *opc, int sreg, int dmode, int dreg);
typedef void (*trace_fmt_E_callback_t)(char *opc, int sreg, int dmode, int dreg);

// void   trace_fmt10(char *opc, int smode, int sreg, int dmode, int dreg, uint16_t n1word);
typedef void (*trace_fmt_F_callback_t)(char *opc, int smode, int sreg, int dmode, int dreg, uint16_t n1word);

// void   trace_fmt11(char *opc, int sind, int sreg, double s, int dind, int dreg, double d);
typedef void (*trace_fmt_G_callback_t)(char *opc, int sind, int sreg, double s, int dind, int dreg, double d);

// void   trace_Interrupt(int i);
typedef void (*trace_fmt_H_callback_t)(int i);

// void   trace_fmtInvalid(void);
typedef void (*trace_fmt_I_callback_t)(void);

typedef struct _wd16_cpu_state_t
{
  REGS regs;

  uint16_t oldPCs[256];       /* table of prior PC's */
  unsigned oldPCindex;        /* pointer to next entry in prior PC's table */
  uint16_t op, opPC;          /* current opcode (base) and its location */
  char cpu4_svcctxt[16];      /* my SVCCs starts LO and go up, or */
                              /*          starts HI and go down.. */

  pthread_mutex_t intlock_t;  /* interrupt lock */
  pthread_t cpu_t;            /* cpu thread */

  /* trace callbacks */

  trace_fmt_A_callback_t          trace_fmt1;
  trace_fmt_A_callback_t          trace_fmt2;
  trace_fmt_A_callback_t          trace_fmt3;
  trace_fmt_A_callback_t          trace_fmt4_svca;
  trace_fmt_A_callback_t          trace_fmt4_svcb;
  trace_fmt_A_callback_t          trace_fmt4_svcc;
  trace_fmt_A_callback_t          trace_fmt5;
  trace_fmt_B_callback_t          trace_fmt6;
  trace_fmt_C_callback_t          trace_fmt7;
  trace_fmt_B_callback_t          trace_fmt8;
  trace_fmt_D_callback_t          trace_fmt9;
  trace_fmt_D_callback_t          trace_fmt9_jsr;
  trace_fmt_D_callback_t          trace_fmt9_lea;
  trace_fmt_E_callback_t          trace_fmt9_sob;
  trace_fmt_F_callback_t          trace_fmt10;
  trace_fmt_G_callback_t          trace_fmt11;
  trace_fmt_H_callback_t          trace_Interrupt;
  trace_fmt_I_callback_t          trace_fmtInvalid;

  /* memory access callbacks */

  get_put_byte_callback_t         getAMbyte;
  get_put_byte_callback_t         putAMbyte;
  get_put_word_callback_t         getAMword;
  get_put_word_callback_t         putAMword;
  get_put_word_by_mode_callback_t getAMaddrBYmode;
  get_put_word_by_mode_callback_t getAMwordBYmode;
  get_byte_by_mode_callback_t     getAMbyteBYmode;
  und_by_mode_callback_t          undAMwordBYmode;
  und_by_mode_callback_t          undAMbyteBYmode;
  put_word_by_mode_callback_t     putAMwordBYmode;
  put_byte_by_mode_callback_t     putAMbyteBYmode;

} wd16_cpu_state_t;

extern wd16_cpu_state_t wd16_cpu_state;

void do_fmt_invalid(void);
void execute_instruction(void);
void perform_interrupt(void);
void cpu_thread(void);
void cpu_stop(void);

/*-------------------------------------------------------------------*/
/* misc                                                              */
/*-------------------------------------------------------------------*/
void config_memdump(uint16_t where, uint16_t fsize);

#ifdef __cplusplus
}
#endif

#endif
