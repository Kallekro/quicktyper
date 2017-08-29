#include <iostream>
#include <chrono>
#include <vector>
#include <random>
#include <fstream>
#include <stdio.h>

typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::microseconds microseconds;

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

enum Difficulty {EASY, MEDIUM, HARD, NONE};

class Word {
public:
  void initialize(std::string text, int posy, int posx, int speed, unsigned int lastMove, bool verticalScrolling) {
    _text = text;
    _posx = posx;
    _posy = posy;
    _speed = speed;
    _lastMove = lastMove;
    _verticalScrolling = verticalScrolling;
  }

  bool update(unsigned int newtime, int placeOfDeath) {
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

  bool checkWordMatch(std::string current_typed_word) {
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
 
   std::string getText () {
     return _text;
   }
 
   void moveWord () {
     if (_verticalScrolling) ++_posy;
     else ++_posx;
   }
 
   void printSelf () {
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
 
   int speed() {
     return _speed;
   }    

   int getTypedLetterCount () {
     return _typed_letter_count;
   }  

   int getLastTypedLetterCount () {
     return _last_typed_letter_count;
   }
   
 private:
   std::string _text;
   int _posy, _posx;
   int _speed;
   unsigned int _lastMove;
   int _typed_letter_count = 0;
   int _last_typed_letter_count = 0;
   bool _verticalScrolling;
 
 };

class GameManager {
public:
  void initialize(int max_y, int max_x, std::string filename) {
    _max_y = max_y;
    _max_x = max_x;

    chooseGameMode();

    switch (_difficulty) {
      case EASY:
        _speed_range[0] = 150;
        _speed_range[1] = 300;      
        _spawn_time_interval_default = _dtime * 2000;
        _spawn_time_decay = 10;
        break;
      case MEDIUM:
        _speed_range[0] = 60;
        _speed_range[1] = 200;      
        _spawn_time_interval_default = _dtime * 1500;
        _spawn_time_decay = 15;
        break;
      case HARD:
        _speed_range[0] = 40;
        _speed_range[1] = 176;      
        _spawn_time_interval_default = _dtime * 1000;
        _spawn_time_decay = 20;
        break;
    }

    _spawn_time_interval = _spawn_time_interval_default;
    _last_spawn_time = -_spawn_time_interval;

    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    randomEngine = std::mt19937_64(seed);
    

    loadWords(filename);

    getHighscore();
  }

  void update() {
    std::vector<Word> newWords;
    for (int i=0; i < _words.size(); i++) {
      if (!_words[i].update(_time, _placeOfDeath)) {
        newWords.push_back(_words[i]);
      }
      else { // Word reached bottom of screen
        bool targetFlag = true;
        for (int j=0; j < _words.size(); j++) {
          if (i != j &&_words[j].getTypedLetterCount() >= _words[i].getTypedLetterCount()) {
            targetFlag = false;
          }
        }
        if (targetFlag) {
          _current_typed_word = "";
        }
        _lives--;
        if (_lives <= 0) {
          _dead = true;
        }
      }
    }
    _words = newWords;
    if (_time - _last_spawn_time > _spawn_time_interval) {
      spawnNewWord();
      _last_spawn_time = _time;
    }
  }

  void loadWords (std::string filename, bool debug = false) {
    if (debug) {
      std::string words[10] = {"FOR", "WHILE", "VECTOR", "DELTA", "MAIN", "PROGRAMMING", "PYTHON", "GOOGLE", "TUTORIAL", "HELP"};
      for (int i=0; i<10; i++) {
        _possibleWords.push_back(words[i]);
      }
      return;
    }  
    std::ifstream input(filename);
    std::string line;
    while (std::getline(input, line)) {
      if (line.length() > 1) {
        std::string cleanWord = "";
        for (int i=0; i<line.length(); i++) {
          int lowerAsciiVal = (int)toupper(line[i]);
          if (lowerAsciiVal > 64 && lowerAsciiVal < 91) cleanWord += toupper(line[i]); 
        }
        _possibleWords.push_back(cleanWord);
      }
    }
    input.close();
  }

  void getHighscore() {
    std::ifstream input("_highscore.txt");
    int skip;
    switch (_difficulty) {
      case MEDIUM:
        skip = 1;      
        break;
      case HARD:
        skip = 2;
        break;
      default:
        skip = 0;
        break;
    }
    if (input.good()) {
      std::string highscore_tmp;
      while (skip >= 0) {
        getline(input, highscore_tmp);
        skip--;
      }  
      try {
        _highscore = std::stoi(highscore_tmp);
      }
      catch (std::invalid_argument& ex) {
        _highscore = 0;
      }
    } else {
      _highscore = 0;
    }
    input.close();
  }

  void saveHighscore() {
    int skip;
    switch (_difficulty) {
      case MEDIUM:
        skip = 1;      
        break;
      case HARD:
        skip = 2;
        break;
      default:
        skip = 0;
        break;
    }

    std::ifstream infile ("_highscore.txt");

    std::string finalStr = "";      
    if (!infile.good()) {
      for (int i=0; i<3; i++) {
        if (i == skip) finalStr += std::to_string(_highscore) + '\n';
        else finalStr += "0" + '\n';
      }  
    }
    else {
      std::string tempStr;
      while (infile >> tempStr) {
        if (skip == 0) tempStr = std::to_string(_highscore);
        skip--; 
        finalStr += tempStr + '\n';
      }
      infile.close();
    }
    std::ofstream outfile ("_highscore.txt");
    outfile << finalStr;
    outfile.close();
  }

  void spawnNewWord () {
    // For random word
    std::uniform_int_distribution<int> wordDistribution(0, _possibleWords.size()-1);
    std::string randomWord = _possibleWords[wordDistribution(randomEngine)];

    // For random pos
    int r_xpos, r_ypos;
    if (_vertScroll) {
      std::uniform_int_distribution<int> posDistribution(0, _max_x - randomWord.length());
      r_xpos = posDistribution(randomEngine);
      r_ypos = 0;
    }  
    else {
      std::uniform_int_distribution<int> posDistribution(0, _max_y-5);
      r_xpos = 0;
      r_ypos = posDistribution(randomEngine);
    }  
     
    // For random speed
    std::uniform_int_distribution<int> speedDistribution(_speed_range[0], _speed_range[1]);

    Word word;
    word.initialize(randomWord, r_ypos, r_xpos, _dtime*speedDistribution(randomEngine), _time, _vertScroll);
    _words.push_back(word);
  }

  bool checkAllWordsMatch (char newchar = '\0') {
    std::vector<Word> newWords;
    bool letterMatch = false;
    for (int i=0; i<_words.size(); i++) {
      if (!_words[i].checkWordMatch(_current_typed_word)) {
        newWords.push_back(_words[i]);
      } 
      else {
        letterMatch = true;
        _current_typed_word = "";
        _score += 50;
        if (_score % 150 == 0) {
          if (_spawn_time_interval > _dtime) _spawn_time_interval -= _spawn_time_decay*_dtime;
        }
      }
    }
    _words = newWords;
    if (!letterMatch) {
      int highest_letter_count = 0;
      Word highest_LC_word;
      for (int i=0; i<_words.size(); i++) {
        if (_words[i].getTypedLetterCount() > highest_letter_count){
          highest_letter_count = _words[i].getTypedLetterCount();
          highest_LC_word = _words[i];
        }
      }
      if (highest_LC_word.getLastTypedLetterCount() != highest_LC_word.getTypedLetterCount() && 
          highest_LC_word.getText()[highest_letter_count-1] == newchar) 
      {
        letterMatch = true;
      }    
    }
    
    return letterMatch;
  }

  void displayEndScreen () {
    clear();
    std::string msg1 = "YOU DIED";
    mvprintw(_max_y/2-1, _max_x/2 - msg1.length()/2, "%s", msg1.c_str());
    std::string msg2 = "FINAL SCORE:"; 
    mvprintw(_max_y/2 + 1, _max_x/2 - (msg2.length() + std::to_string(_score).length() + 1)/2, "%s %d", msg2.c_str(), _score);
    if (_score > _highscore) {
      std::string msg3 = "NEW HIGHSCORE:";
      std::string msg3b = "OLD HIGHSCORE:";
      mvprintw(_max_y/2 + 2, _max_x/2 - (msg3.length() + std::to_string(_score).length() + 1)/2, "%s %d", msg3.c_str(), _score);
      mvprintw(_max_y/2 + 3, _max_x/2 - (msg3b.length() + std::to_string(_highscore).length() + 1)/2, "%s %d", msg3b.c_str(), _highscore);
      
      _highscore = _score;
      saveHighscore();
    }
    else {
      std::string msg3 = "HIGHSCORE:";
      mvprintw(_max_y/2 + 2, _max_x/2 - (msg3.length() + std::to_string(_highscore).length() + 1)/2, "%s %d", msg3.c_str(), _highscore);
    }
    std::string msg4 = "PRESS ENTER TO START AGAIN"; 
    mvprintw(_max_y/2 + 5, _max_x/2 - msg4.length()/2, "%s", msg4.c_str());
    
    std::vector<Word> emptyVec;
    _words = emptyVec;
    _spawn_time_interval = _spawn_time_interval_default;
    _last_spawn_time = -_spawn_time_interval;
    _current_typed_word = "";
    _score = 0;
    _lives = 3;
    _dead = false;
    
    int continueInput;
    while ((continueInput = getch()) != int('\n')) {
    }
  }

  int wrapAroundArray (int n, int k) {
    if (k >= n) return 0;
    else if (k < 0) return n-1;
    else return k;
  }        

  void printMenus (std::string *menus[], int *indexes, int curr_menu_group) {
    int xposAdjuster = 15;
    int ypos;
    for (int k=0; k < 3; k++) {
      // Draw pointer to menu group      
      if (k == curr_menu_group) {
        mvprintw(_max_y/2 - 3, _max_x/2 - xposAdjuster - 1, "||");
      }
      // Draw each menu group
      std::string *strArr = menus[k];
      int i = 0;      
      ypos = _max_y / 2 - 1;
      while (strArr[i] != "0") {
        // Draw with colors if chosen menu      
        if (indexes[k] == i) attron(COLOR_PAIR(1));      
        mvprintw(ypos, _max_x / 2 - xposAdjuster - strArr[i].length()/2, strArr[i].c_str()); 
        if (indexes[k] == i) attroff(COLOR_PAIR(1));      
        ypos += 2;
        i++;
      }
      xposAdjuster -= 15;
    }
    std::string startMsg = "Use arrow keys to select desired settings";
    std::string finalMsg = "Press Enter to continue.";      
    mvprintw(_max_y / 2 - 6, _max_x / 2 - startMsg.length()/2, startMsg.c_str());
    mvprintw(_max_y / 2 + 6, _max_x / 2 - finalMsg.length()/2, finalMsg.c_str());
  }
  
  void chooseGameMode() {
    std::string menus_diff[4] = {"Easy", "Medium", "Hard", "0"};
    std::string menus_mode[3] = {"Forgiving", "Not forgiving", "0"};
    std::string menus_dir[3] = {"Horizontal", "Vertical", "0"};
    std::string *all_menus[3] = {menus_diff, menus_mode, menus_dir};

    int menu_indexes[3] = {1, 0, 0};

    int curr_menu_group = 0;
    bool menuChosen = false;
    int input_c;
    nodelay(stdscr, FALSE);

    printMenus(all_menus, menu_indexes, curr_menu_group);

    while ((input_c = getch()) != KEY_F(1)) {
      clear();

      int idx;
      switch (input_c) {
        case 10: // Enter key
          menuChosen = true;        
          break;
        case KEY_LEFT:
          curr_menu_group = wrapAroundArray(3, curr_menu_group - 1);
          break;
        case KEY_RIGHT:  
          curr_menu_group = wrapAroundArray(3, curr_menu_group + 1);
          break;
        case KEY_UP:
          idx = menu_indexes[curr_menu_group];
          if (curr_menu_group == 0) menu_indexes[curr_menu_group] = wrapAroundArray(3, idx-1); 
          else menu_indexes[curr_menu_group] = wrapAroundArray(2, idx-1); 
          break;
        case KEY_DOWN:
          idx = menu_indexes[curr_menu_group];
          if (curr_menu_group == 0) menu_indexes[curr_menu_group] = wrapAroundArray(3, idx+1); 
          else menu_indexes[curr_menu_group] = wrapAroundArray(2, idx+1); 
          break;
        default:  
          break;
      }
      if (menuChosen) break;
      printMenus(all_menus, menu_indexes, curr_menu_group);
    }
    
    switch (menu_indexes[0]) {
      case 0:
        _difficulty = EASY;
        break; 
      case 1:
        _difficulty = MEDIUM;
        break;
      case 2:
        _difficulty = HARD;
        break;
      default:
        _difficulty = NONE;
        break;
    }

    if (menu_indexes[1] == 0) _forgivingMode = true;
    else _forgivingMode = false;
    
    if (menu_indexes[2] == 1) { 
      _vertScroll = true;
      _placeOfDeath = _max_y - 4;
    }  
    else {
      _vertScroll = false;
      _placeOfDeath = _max_x - 1;
    }

  } 

  void gameLoop () {
    nodelay(stdscr, TRUE);
    _last_update_time = Clock::now();      
    unsigned int frameTime = 0;
    int input_c;
    while (input_c != KEY_F(1)) { // Press F1 key to quit game loop
      Clock::time_point startOfFrame = Clock::now();
      unsigned int elapsed_time = std::chrono::duration_cast<microseconds>(startOfFrame - _last_update_time).count();
      if (frameTime > 0 && frameTime < 2000) {
        while (elapsed_time < 2000 - frameTime ) {
          startOfFrame = Clock::now();
          elapsed_time = std::chrono::duration_cast<microseconds>(startOfFrame - _last_update_time).count();
        }
      }

      getyx (stdscr, _cursor_y, _cursor_x); // Store the cursors current position

      bool charAdded = false;
      if ((input_c = getch()) == ERR) {
        // If there was no input
        
      } else {
        // If there was any input
              
        switch (input_c) {
          case int('\n'): // Enter key
            move (_cursor_y, _cursor_x); // Enter key automatically moves cursor down, so reset position
            refresh();
            break;
          case ALT_BACKSPACE: // Backspace key
            if (_current_typed_word.length() > 0) {
              _current_typed_word.pop_back();
            }  
            charAdded = true;
            break;
          case 27: // Escape
            _current_typed_word = "";
            break;
          case int(' '):
            if (_current_typed_word.length() > 0) _current_typed_word += char(input_c);
            charAdded = true;
            break;
          default:
            if (input_c > 96 && input_c < 123) {
              input_c -= 32; // subtract to get lower
            }
            if (input_c > 64 && input_c < 91) {
              _current_typed_word += char(input_c);
              charAdded = true;
            }
            break;
        }
      }
      
      clear(); // Clear the screen
      
      _time += _dtime;

      // Draw seperating line
      move(_max_y - 3, 0);
      for (int i=0; i<_max_x; i++) {
        addch('-');
      }
      
      if (charAdded ) {
        // Forgiving mode means you can't type an invalid word
        if (_forgivingMode) {
          // Returns true if current word matches anything on screen
          // note: contains other logic as well
          if (!checkAllWordsMatch(_current_typed_word.back())) {
            if (_current_typed_word.length() > 0)  _current_typed_word.pop_back();
            
          }
        } 
        else {
          checkAllWordsMatch();
        }
      }
      // Update game logic
      update();

      // Draw score and highscore
      mvprintw(_max_y-2, 0, "Score: %d", _score);
      mvprintw(_max_y-1, 0, "Highscore: %d", _highscore);

      // Draw life
      std::string lifemsg = "LIFE:";
      mvprintw(_max_y-2, _max_x - (lifemsg.length() + std::to_string(_lives).length() + 1), "%s %d", lifemsg.c_str(), _lives);

      // Debug string
      //mvprintw(0, 0, "DEBUG: %d", _debug_string);
      
      // Draw current word being typed
      mvprintw(_max_y-1, _max_x/2 - _current_typed_word.length()/2, "%s", _current_typed_word.c_str());

      // Send changes to screen
      refresh(); 

      if (_dead) {
        displayEndScreen();
      }

      Clock::time_point endOfFrame = Clock::now();
      frameTime = std::chrono::duration_cast<std::chrono::microseconds>(endOfFrame - startOfFrame).count();
      
      _last_update_time = Clock::now();
    }
  }

private:
  std::vector<Word> _words;
  std::vector<std::string> _possibleWords;//

  int _max_y, _max_x;
  int _cursor_y, _cursor_x;
  
  unsigned int _dtime = 1;
  unsigned int _time = 0;

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

  unsigned int _score = 0;
  unsigned int _highscore;
  
  int _lives = 3;
  bool _dead = false;
  
  bool _forgivingMode;

  std::string _debug_string = "";
   
protected:  
  std::mt19937_64 randomEngine;
};

int main(int argc, char *argv[]) {
  //Word newword;
  initscr();

  keypad(stdscr, TRUE);

  nodelay(stdscr, TRUE); // Set getch() to be non-blocking
  noecho();
  int max_y, max_x;
  getmaxyx(stdscr, max_y, max_x); // Store maximum x and y of screen

  start_color();
  init_pair(1, COLOR_RED, COLOR_BLACK);

  GameManager gameManager;
  gameManager.initialize(max_y, max_x, "words.txt");
  gameManager.gameLoop();
 
  endwin();
}
