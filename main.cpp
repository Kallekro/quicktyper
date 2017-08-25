#include <iostream>
#include <chrono>
#include <vector>
#include <random>
#include <fstream>

typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::seconds milliseconds;

static int speeds[3] = {50, 100, 200};
bool onWindows = false;

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#include <windows.h>
#include <curses.h>
#define _sleep Sleep 
#else
#include <ncurses.h>
#include <unistd.h>
#endif

// TODO
// Make levels of difficulty
//   * Support different highscores

// ? BACKSPACE does not work on linux ?



class Word {
public:
  void initialize(std::string text, int posy, int posx, int speed, double lastMove, bool verticalScrolling=true) {
    _text = text;
    _posx = posx;
    _posy = posy;
    _speed = speed;
    _lastMove = lastMove;
    _verticalScrolling = verticalScrolling;
  }

  bool update(double newtime, int placeOfDeath) {
    if (newtime - _lastMove > _speed) {
      moveWord();
      _lastMove = newtime;
      if ((_verticalScrolling && _posy == placeOfDeath) || (!_verticalScrolling && _posx == placeOfDeath)) {
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
   double _lastMove;
   int _typed_letter_count = 0;
   int _last_typed_letter_count = 0;
   bool _verticalScrolling;
 
 };

class GameManager {
public:
  void initialize(int max_y, int max_x, std::string filename, bool forgivingMode, bool vertScroll=true) {
    _max_y = max_y;
    _max_x = max_x;

    _forgivingMode = forgivingMode;

    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    randomEngine = std::mt19937_64(seed);
    
    _vertScroll = vertScroll;
    
    if (_vertScroll) _placeOfDeath = max_y - 4;
    else _placeOfDeath = max_x - 1;

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
    if (input.good()) {
      std::string highscore_tmp;
      getline(input, highscore_tmp);
      _highscore = std::stoi(highscore_tmp);
    } else {
      _highscore = 0;
    }
    input.close();
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
    std::uniform_int_distribution<int> speedDistribution(30, 150);

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
        if (_score % 250 == 0) {
          if (_spawn_time_interval > _dtime) _spawn_time_interval -= _dtime*20;
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
      //_debug_string = highest_LC_word.getText() + ", " + highest_LC_word.getText()[highest_letter_count-1] + ", " + newchar;
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
      std::ofstream outfile ("_highscore.txt");
      outfile << _score << std::endl;
      outfile.close();
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
  
  void gameLoop () {
    int input_c;
    while (input_c != KEY_F(1)) { // Press F1 key to quit game loop
      Clock::time_point startOfFrame = Clock::now();

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
          case 8: // Backspace key
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

      // Draw score
      mvprintw(_max_y-1, 0, "Score: %d", _score);

      // Draw life
      std::string lifemsg = "LIFE:";
      mvprintw(_max_y-1, _max_x - (lifemsg.length() + std::to_string(_lives).length() + 1), "%s %d", lifemsg.c_str(), _lives);

      // Debug string
      // mvprintw(0, 0, "DEBUG: %s", _debug_string.c_str());
      
      // Draw current word being typed
      mvprintw(_max_y-1, _max_x/2 - _current_typed_word.length()/2, "%s", _current_typed_word.c_str());

      // Send changes to screen
      refresh(); 

      // Get time it took for frame to end. It was probably 0 
      Clock::time_point endOfFrame = Clock::now();
      float frameTime = std::chrono::duration_cast<milliseconds>(endOfFrame - startOfFrame).count();
      
      if (_dead) {
        displayEndScreen();
      }
      
      // Sleep for frame minimum - time frame took
      if (_dtime - frameTime > 0) {
        #if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
        _sleep(_dtime - frameTime); 
        #else
        usleep((_dtime - frameTime)*1000);
        #endif
      }  
       
    }
  }

private:
  std::vector<Word> _words;
  std::vector<std::string> _possibleWords;//

  int _max_y, _max_x;
  int _cursor_y, _cursor_x;
  
  double _dtime = 1;
  double _time = 0;
  int _spawn_time_interval_default = _dtime * 750;
  int _spawn_time_interval = _spawn_time_interval_default;
  int _last_spawn_time = -_spawn_time_interval;
  std::string _current_typed_word;
  int _placeOfDeath;
  bool _vertScroll;

  int _score = 0;
  int _highscore;
  
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
  gameManager.initialize(max_y, max_x, "words.txt", true, false);
  gameManager.gameLoop();
 
  endwin();
}
