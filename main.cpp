#include "GameManager.h" 

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#define WINDOWS_SYS
#define ALT_BACKSPACE 8 
#include <windows.h>
#include <curses.h>
#else
#define ALT_BACKSPACE 263 
#include <ncurses.h>
#include <unistd.h>
#endif

// TODO
// DONE Make levels of difficulty
// DONE  * Support different highscores
// The highscore file is not generated properly but works if already exist

// Avoid word collision

// ? BACKSPACE does not work on linux ?


int main(int argc, char *argv[]) {
  initscr();

  keypad(stdscr, TRUE);

  nodelay(stdscr, TRUE); // Set getch() to be non-blocking
  noecho();
  int max_y, max_x;
  getmaxyx(stdscr, max_y, max_x); // Store maximum x and y of screen

  start_color();
  init_pair(1, COLOR_RED, COLOR_BLACK);

  GameManager gameManager(max_y, max_x, "words.txt");

  gameManager.gameLoop();
 
  endwin();
}
