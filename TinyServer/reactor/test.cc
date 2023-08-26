#include"reactor.hpp"


int main()
{
    Reactor* reactor=Reactor::getInstant();

    reactor->init();
    reactor->run();
    return 0;
}
