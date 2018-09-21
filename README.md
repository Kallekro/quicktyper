# quicktyper
A simple console game in which you type words on your screen.

## installation
You need the Unix library ncurses or the Windows library pdcurses.

Compile the game using following command:
```
g++ Word.cpp GameManager.cpp main.cpp -lcurses
```
If using windows you need the flag **-lpdcurses** and on Unix you need **-lcurses**.

You might have to use the flag -std=c++11 to force the compiler to C++11 syntax (which your compiler must support).

## ncurses install
To install ncurses for distributions with apt-get:
```
sudo apt install libncurses5-dev libncursesw5-dev
```
