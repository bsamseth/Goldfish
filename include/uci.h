#ifndef UCI_H
#define UCI_H

#include "position.h"
#include "search.h"
#include <thread>


class UCI {
    public:
        std::string ENGINE_NAME = "Goldfish";
        std::string AUTHOR = "Bendik Samseth";

        Position pos;
        Searcher searcher;
        std::thread *search_thread;

        UCI();
        void startCommunication();
};



#endif
