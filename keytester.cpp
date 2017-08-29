#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#include <curses.h>
#else
#include <ncurses.h>
#endif
int main () {
  initscr();      
  keypad(stdscr, TRUE);
  nodelay(stdscr, FALSE);
  noecho();
  int c;
  while ((c = getch()) != KEY_F(1)) {
    clear();
    mvprintw(0,0, "%d", c); 
  }
  return 0;
}
