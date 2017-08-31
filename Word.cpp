#include <iostream>
#include <chrono>
#include <vector>
#include <random>
#include <fstream>
#include <stdio.h>

#include "Word.h"


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


Word::Word(std::string text, int posy, int posx, int speed, unsigned int lastMove, bool verticalScrolling) {
  _text = text;
  _posx = posx;
  _posy = posy;
  _speed = speed;
  _lastMove = lastMove;
  _verticalScrolling = verticalScrolling;
  _typed_letter_count = 0;
  _last_typed_letter_count = 0;
}


bool Word::update(unsigned int newtime, int placeOfDeath) {
  if (newtime - _lastMove > _speed) {
    moveWord();
    _lastMove = newtime;
    if ((_verticalScrolling && _posy == placeOfDeath) 
        || (!_verticalScrolling && _posx + _text.length() - 1 == placeOfDeath)) {
      printSelf();
      return true;
    }
  }
  printSelf();
  return false;
}

bool Word::checkWordMatch(std::string current_typed_word) {
  int matchCount = 0;
  for (int i=0; i<_text.length(); i++) {
    if (toupper(_text[i]) == toupper(current_typed_word[i])) {
      matchCount++;
    }
    else break;
  }
  if (matchCount == _text.length()) {
    _typed_letter_count = 0;
    return true;
  }
  else {
    _last_typed_letter_count = _typed_letter_count;
    _typed_letter_count = matchCount; 
    return false;
  }  
}

std::string Word::getText () {
  return _text;
}

void Word::moveWord () {
  if (_verticalScrolling) ++_posy;
  else ++_posx;
}

void Word::printSelf () {
  move(_posy, _posx);
  attron(COLOR_PAIR(1));
  for (int i = 0; i < _typed_letter_count; i++) {
    addch(toupper(_text[i]));
  }
  attroff(COLOR_PAIR(1));
  for (int i = _typed_letter_count; i < _text.length(); i++) {
    addch(toupper(_text[i]));
  }
}

int Word::speed() {
  return _speed;
}    

int Word::getTypedLetterCount () {
  return _typed_letter_count;
}  

int Word::getLastTypedLetterCount () {
  return _last_typed_letter_count;
}

