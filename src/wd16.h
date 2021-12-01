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
#ifndef __WD16_H__
#define __WD16_H__

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <pthread.h>
#include <setjmp.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#include <menu.h>
#include <ncurses.h>
#include <panel.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/mtio.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/utsname.h>

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


char instruction_type[65536];  /* maps all words to inst format type */

REGS regs;

uint16_t oldPCs[256];       /* table of prior PC's */
unsigned oldPCindex;        /* pointer to next entry in prior PC's table */
uint16_t op, opPC;          /* current opcode (base) and its location */
char cpu4_svcctxt[16];      /* my SVCCs starts LO and go up, or */
                            /*          starts HI and go down.. */

pthread_mutex_t intlock_t;  /* interrupt lock */
pthread_t cpu_t;            /* cpu thread */

/*-------------------------------------------------------------------*/
/* in cpu-fmt?.c                                                     */
/*-------------------------------------------------------------------*/


void build_decode(void);
void do_fmt_invalid(void);
void execute_instruction(void);
void perform_interrupt(void);
void cpu_thread(void);
void cpu_stop(void);

/*-------------------------------------------------------------------*/
/* in priority.c                                                     */
/*-------------------------------------------------------------------*/
void set_pri_low(void);
void set_pri_normal(void);
void set_pri_high(void);

/*-------------------------------------------------------------------*/
/* in trace.c                                                        */
/*-------------------------------------------------------------------*/
void trace_Interrupt(int i);
void trace_fmtInvalid(void);



#ifdef __cplusplus
}
#endif

#endif
