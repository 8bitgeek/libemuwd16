#/* makefile           (c) Copyright Mike Noel, 2001-2006             */
#/* ----------------------------------------------------------------- */
#/*                                                                   */
#/* This software is an emulator for the Alpha-Micro AM-100 computer. */
#/* It is copyright by Michael Noel and licensed for non-commercial   */
#/* hobbyist use under terms of the "Q public license", an open       */
#/* source certified license.  A copy of that license may be found    */
#/* here:       http://www.otterway.com/am100/license.html            */
#/*                                                                   */
#/* There exist known serious discrepancies between this software's   */
#/* internal functioning and that of a real AM-100, as well as        */
#/* between it and the WD-1600 manual describing the functionality of */
#/* a real AM-100, and even between it and the comments in the code   */
#/* describing what it is intended to do! Use it at your own risk!    */
#/*                                                                   */
#/* Reliability aside, it isn't the intent of the copyright holder to */
#/* use this software to compete with current or future Alpha-Micro   */
#/* products, and no such competing application of the software will  */
#/* be supported.                                                     */
#/*                                                                   */
#/* Alpha-Micro and other software that may be run on this emulator   */
#/* are not covered by the above copyright or license and must be     */
#/* legally obtained from an authorized source.                       */
#/*                                                                   */
#/* ----------------------------------------------------------------- */

TARGET   = libwd16.a

VERSION  = 1.0

C        = gcc

# CFLAGS   = -O1 -Wall -DVERSION=$(VERSION) -ggdb

CFLAGS   = -Os -I./src -DVERSION=$(VERSION) -Wno-unused-result 			\
									-Wno-pointer-to-int-cast 	\
									-Wno-format 				\
									-Wno-discarded-qualifiers 	\
									-Wno-int-to-pointer-cast

LFLAGS	 = -lm -lpthread -lncurses -lpanel -lmenu

OBJS     = src/wd16.o \
	   		src/cpu-fmt1.o \
	   		src/cpu-fmt2.o \
	   		src/cpu-fmt3.o \
	   		src/cpu-fmt4.o \
	   		src/cpu-fmt5.o \
	   		src/cpu-fmt6.o \
	   		src/cpu-fmt7.o \
	   		src/cpu-fmt8.o \
	   		src/cpu-fmt9.o \
	   		src/cpu-fmt10.o \
	   		src/cpu-fmt11.o \
	   		src/instruction-type.o
	  
HEADERS  = src/wd16.h src/am-ddb.h

$(TARGET): $(OBJS)
	ar rcs $(TARGET) $(OBJS)

$(OBJS): %.o: %.c $(HEADERS) makefile
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -f src/*.o $(TARGET)

