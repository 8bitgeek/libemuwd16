/* cpu-fmt4.h    (c) Copyright Mike Sharkey, 2021                    */
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
#ifndef __CPU_FMT4_H__
#define __CPU_FMT4_H__

#include "wd16.h"

#ifdef __cplusplus
extern "C"
{
#endif


void do_fmt_4(void);
int svca_assist(int arg);
int svcb_assist(int arg);
int svcc_assist(int arg);

void trace_fmt4_svca(char *opc, int arg);
void trace_fmt4_svcb(char *opc, int arg);
void trace_fmt4_svcc(char *opc, int arg);

#ifdef __cplusplus
}
#endif

#endif