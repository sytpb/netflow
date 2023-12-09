OBJS	 =  stream.o cdogio.o fifo.o cdogEar.o cdogNose.o NoseTcp.o NoseUdp.o cdogBase64.o cdog.o
TARGET   = cdog

CC       = gcc
LIBS     = -lpthread -liksemel -lpcap -static
LDFLAGS  = -L./lib -L.
DEFINES  = -DSER_DOG 
INCLUDE  = -I .
CFLAGS   = -g -Wall $(DEFINES) $(INCLUDE) 
CXXFLAGS = $(CFLAGS) 

all:$(TARGET)

$(TARGET):$(OBJS)
	$(CC) $(CXXFLAGS) -o $@ $(OBJS) $(LDFLAGS) $(LIBS)
	@echo

.c.o:
	$(CC) ${CXXFLAGS} -o $@ -c $<

clean:
	rm -rf *.o ${TARGET}
