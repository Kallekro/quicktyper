#pragma once
#include <stdio.h>

class Word {
private:
  std::string _text;      
  int _posy, _posx;
  int _speed;
  unsigned int _lastMove;
  int _typed_letter_count;
  int _last_typed_letter_count;
  bool _verticalScrolling;
public:
  Word(std::string text, int posy, int posx, int speed, unsigned int lastMove, bool verticalScrolling);
  bool update(unsigned int newtime, int placeOfDeath);
  bool checkWordMatch(std::string current_typed_word);
  std::string getText();
  void moveWord();
  void printSelf();
  int speed();
  int getTypedLetterCount();
  int getLastTypedLetterCount();
};
