# [Name]      Makefile
# [Form]      make -f Makefile
# [Gerenal]   UNIX communication library
# [History]   1998/02/09	 Creating V1.0  AI SOFTWARE 

CC      = c++
LINKER  = $(CC) -o
LINT	= lint -c
RM	= /bin/rm -f
AR	= ar cru

LIBS    =

LIBFILE	= 

CFLAGS	= -I$(PROP_TOPDIR)/include -I$(PROP_TOPDIR)/lib/include 

SRC	= AIEUComm.c AIUComm.c AIcom_Tool.c AIcom_Log.c AIcom_profile.c cJSON.c AIcom_PV.c

OBJ	= AIEUComm.o AIUComm.o AIcom_Tool.o AIcom_Log.o AIcom_profile.o cJSON.o AIcom_PV.o

COMLIB	= $(PROP_TOPDIR)/lib/lib/lib_aawant.a 

.c.o:
	$(CC) -c $(CFLAGS) $*.c

all: $(COMLIB)

clean:
	@$(RM) *.o $(COMLIB)

$(COMLIB):  $(SRC) $(OBJ) 
			@echo "Loading $(COMLIB) ... "
			@$(RM) $(COMLIB)
			@$(AR) $(COMLIB) $(OBJ)
			@echo "done"

depend:
	  depend MAKEFILE=Makefile $(CFLAGS) $(SRCS)

# DO NOT DELETE THIS LINE -- make depend depends on it.
