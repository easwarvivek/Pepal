SRCS=$(wildcard *.cc)
OBJS=$(subst .cc,.o,$(SRCS))

CXXFLAGS=-g3 -O -Wall

all: libPBC.a NDA crypto

COMMON_OBJS=Pairing.o G.o G1.o G2.o GT.o Zr.o PPPairing.o crypto.o
#COMMON_OBJS=Pairing.o G.o G1.o G2.o GT.o Zr.o PPPairing.o 

libPBC.a: $(COMMON_OBJS)
	ar rcs $@ $^ 

NDA: NDA.o libPBC.a
	g++ -L. -L/usr/lib -g -o $@ $^ -lpbc -lgmp -lcryptopp -lpthread 

crypto: crypto.o
	g++ -c  crypto.h

clean:
	-rm -f $(OBJS)

depend:
	makedepend -Y -- $(CXXFLAGS) -- $(SRCS) 2>/dev/null


# DO NOT DELETE

G.o: G.h Pairing.h Zr.h PBCExceptions.h
G1.o: G1.h G.h Pairing.h Zr.h PBCExceptions.h
G2.o: G2.h G.h Pairing.h Zr.h PBCExceptions.h
GT.o: GT.h G.h Pairing.h Zr.h PBCExceptions.h
NDA.o: crypto.h cryptopp/osrng.h cryptopp/config.h cryptopp/cryptlib.h
NDA.o: cryptopp/stdcpp.h cryptopp/trap.h cryptopp/filters.h
NDA.o: cryptopp/cryptlib.h cryptopp/simple.h cryptopp/misc.h
NDA.o: cryptopp/smartptr.h cryptopp/secblock.h cryptopp/queue.h
NDA.o: cryptopp/algparam.h cryptopp/integer.h cryptopp/ccm.h
NDA.o: cryptopp/authenc.h cryptopp/modes.h cryptopp/strciphr.h
NDA.o: cryptopp/seckey.h cryptopp/argnames.h cryptopp/aes.h
NDA.o: cryptopp/rijndael.h cryptopp/sha.h cryptopp/iterhash.h cryptopp/hex.h
NDA.o: cryptopp/basecode.h cryptopp/filters.h PBC.h G1.h G.h Pairing.h Zr.h
NDA.o: G2.h GT.h PBCExceptions.h PPPairing.h memstream/memstream.h
NDA.o: Recipient.h Sender.h
PPPairing.o: PPPairing.h Pairing.h G1.h G.h Zr.h G2.h GT.h PBCExceptions.h
Pairing.o: Pairing.h G1.h G.h Zr.h G2.h GT.h PBCExceptions.h
Zr.o: Zr.h Pairing.h PBCExceptions.h
