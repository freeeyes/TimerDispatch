include makefile.define

default:all

PATS1 = LinuxTimer_Test.o ../Common/TimerThread.o ../Common/TimerInfo.o ../Common/Time_Value.o

LIB_BASE_OBJS1 = LinuxTimer_Test.o TimerThread.o TimerInfo.o Time_Value.o

LIB_BASE1 = LinuxTimer_Test

all:$(LIB_BASE1) makefile

$(LIB_BASE1):$(PATS1)
	$(CC) -rdynamic -o $(LIB_BASE1) $(LIB_BASE_OBJS1) $(LIBS)

clean:
	rm -rf *.o  $(LIB_BASE1) $(LIB_BASE_OBJS1)
cl:
	rm -rf *.o 
