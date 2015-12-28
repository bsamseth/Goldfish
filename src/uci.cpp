#include <string>
#include <iostream>
#include <sstream>
#include <cassert>
#include <algorithm>
#include <cmath>

#include "types.h"
#include "position.h"
#include "move.h"
#include "movegen.h"
#include "uci.h"

using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::getline;

UCI::UCI() {}

void UCI::startCommunication() {
  string input = "";

  while (true) {
    getline(cin, input);

    if (input == "uci") {
      // use uci, only option
      cout << "id name " << ENGINE_NAME << endl;
      cout << "id author " << AUTHOR << endl;

      // send all options that can be set, TODO

      // sent all params and is ready
      cout << "uciok" << endl;
      
    }
    else if (input == "isready") {
      // are we ready? Guess so.
      cout << "readyok" << endl;
    }

    else if (input == "ucinewgame"){
      // do init.
      pos = Position();
    }

    else if (input.substr(0, 8) == "position") {
      string word;
      std::istringstream iss(input);
      while (iss >> word) {
	if (word == "position")
	  continue;

	else if (word == "startpos")
	  pos.setFromFEN(STARTING_FEN);

      }
    }

    else if (input == "print")
      cout << pos.str() << endl;

    else if (input == "quit")
      break;
  }
}
