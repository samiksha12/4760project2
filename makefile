CC	= g++
CFLAGS  = -g3 -Wall
TARGET1 = worker
TARGET2 = newoss 

OBJS1	= worker.o
OBJS2	= newoss.o
all:	$(TARGET1) $(TARGET2)

$(TARGET1):	$(OBJS1)
	$(CC) $(CFLAGS) -o $(TARGET1) $(OBJS1)

$(TARGET2):	$(OBJS2)
	$(CC) $(CFLAGS) -o $(TARGET2) $(OBJS2)

worker.o:	worker.cpp
	$(CC) $(CFLAGS) -c worker.cpp 

newoss.o:	newoss.cpp
	$(CC) $(CFLAGS) -c newoss.cpp


clean:
	/bin/rm -f *.o $(TARGET1) $(TARGET2)
