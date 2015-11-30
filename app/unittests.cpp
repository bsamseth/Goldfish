#include <iostream>
#include <string>
#include <vector>
#include <stdio.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>


#include "types.h"
#include "move.h"
#include "position.h"
#include "bitboards.h"
#include "movegen.h"


using std::cout;
using std::endl;
using std::string;
using std::vector;
using Bitboards::prettyString;


int main(int argc, char *argv[])
{
  Square mySquare = SQ_G3;
  cout << "mySquare = " << SquareName[mySquare] << endl;
  return 0;
}
