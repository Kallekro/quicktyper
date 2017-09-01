
#
#OBJECTS=Word.cpp GameManager.cpp main.cpp

#ifeq ($(detected_OS), Windows)
#  LDFLAGS=-lpdcurses
#else
#  LDFLAGS=-lncurses -std=c++11

#main: main.cpp
#  $(CC) $(OBJECTS) -o main $(LDFLAGS)

#ifeq ($(detected_OS), Windows)
#  delCMD:=del main.exe /Q /F
#else
#  delCMD:=rm -f main.exe 
#
#.PHONY: clean
#clean: main 
#	$(delCMD)

CC=g++

main: main.cpp
	g++ -g Word.cpp GameManager.cpp main.cpp -lpdcurses -o quicktyper 

linux_main: main.cpp
	g++ -g Word.cpp GameManager.cpp main.cpp -lcurses -std=c++11 -o quicktyper

.PHONY: clean

clean: 
	del quicktyper.exe /Q /F

linux_clean:
	rm -f quicktyper.out
