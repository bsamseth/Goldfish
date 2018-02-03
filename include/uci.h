#ifndef UCI_H
#define UCI_H

#include "position.h"
#include "search.h"


class UCI {
    public:
        std::string ENGINE_NAME = "Goldfish";
        std::string AUTHOR = "Bendik Samseth";

        Position pos;
        Searcher searcher;

        UCI();
        void startCommunication();
};



#endif
