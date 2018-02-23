# save this as: makefile
# usage: to compile myprog.c:
#  make -f mymakefile.mk  myprog
# will compile simple programs that don't require other files
# to compile. 

# C compiler name
CC=gcc

# loggging
LOGIT=2>&1 | tee -a make.log

# flags for C like -s or -Wall
# you add what you want here
CFLAGS=-g

# linker objects like anotherfile.o or somefile.a
# you add what you want here
LNKFILES=

#libraries to read
# left blank you add what you want here
LNKLIB=

# Link with math library
#
LDFLAGS=-lm

# compile C code
frac-order : frac-order.c
	@ echo "Make frac-order"
	@ rm -f make.log
	$(CC) $(CFLAGS) frac-order.c -o frac-order $(LDFLAGS) $(LNKFILES) $(LNKLIB)
	@rm -f $*.o
	@ echo "Successful make of frac-order"
