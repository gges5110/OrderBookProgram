all: Pricer.exe

Pricer.exe: Pricer.o Pricer.h
	 g++ -o Pricer.exe Pricer.o -I.

Pricer.o: Pricer.cpp Pricer.h
	 g++ -c Pricer.cpp Pricer.h -std=c++11 -I.

clean:
	 rm Pricer.o Pricer.exe
