
include ../../../../../GDALmake.opt

OBJ = \
	io.o \
    r2000.o

O_OBJ =	$(foreach file,$(OBJ),../../../o/$(file))

CPPFLAGS	:=	-I. -I.. -I../.. -I../../.. $(CPPFLAGS) -std=c++11

default:	$(O_OBJ:.o=.$(OBJ_EXT))

../../../o/%.$(OBJ_EXT):	%.cpp
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

clean:
	rm -f *.o $(O_OBJ)
	rm -f *~
