CC=g++
LIBSOCKET=-lnsl
CCFLAGS=-Wall -g
SRV=server
CLT=client
SRVC=server.cpp header.cpp
CLTC=client.cpp header.cpp

all: $(SRV) $(CLT)

$(SRV): $(SRVC) 
	$(CC) -o $(SRV) $(CCFLAGS) $(LIBSOCKET) $(SRVC)

$(CLT):	$(CLTC)
	$(CC) -o $(CLT) $(CCFLAGS) $(LIBSOCKET) $(CLTC)

clean:
	rm -f *.o *~
	rm -f $(SRV) $(CLT)

