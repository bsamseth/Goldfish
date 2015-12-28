#ifndef UCI_H
#define UCI_H

#include "types.h"
#include "position.h"
#include "move.h"
#include "movegen.h"


class UCI {
  public:
  std::string ENGINE_NAME = "Goldfish";
  std::string AUTHOR = "Bendik Samseth";

  Position pos;

  UCI();
  void startCommunication();
};



#endif
