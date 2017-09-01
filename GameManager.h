#pragma once

#include <chrono>
#include <vector>
#include <stdio.h>
#include <random>
#include "Word.h"

typedef std::chrono::high_resolution_clock Clock;

enum Difficulty {EASY, MEDIUM, HARD, STRESS, NONE};

class GameManager {
private:
  std::vector<Word> _words;
  int _words_max_size;
  std::vector<std::string> _possibleWords;

  int _max_y, _max_x;
  int _cursor_y, _cursor_x;

  unsigned int _dtime;
  unsigned int _time;

  Clock::time_point _last_update_time;

  Difficulty _difficulty;

  int _speed_range[2];  

  unsigned int _spawn_time_interval_default;
  unsigned int _spawn_time_interval;
  unsigned int _last_spawn_time;
  unsigned int _spawn_time_decay;

  std::string _current_typed_word;
  int _placeOfDeath;
  bool _vertScroll;

  unsigned int _score;
  unsigned int _highscore;

  int _lives;

  bool _dead;
  bool _immortal;

  bool _forgivingMode;

  std::string _debug_string;

protected:
  std::mt19937_64 randomEngine;

public:
  GameManager(int max_x, int max_y, std::string filename);        
  void update();
  void loadWords(std::string filename, bool debug = false);
  void getHighscore();
  void saveHighscore();
  void spawnNewWord();
  bool checkAllWordsMatch(char newchar = '\0');
  void displayEndScreen();
  int wrapAroundArray(int n, int k);
  void printMenus (std::string *menus[], int *indexes, int curr_menu_group);
  void chooseGameMode();
  void gameLoop();
};
