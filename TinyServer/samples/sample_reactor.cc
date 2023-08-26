#include <gtest/gtest.h>
#include"../reactor/reactor.hpp"

TEST(reactor,test)
{
    Reactor* r=Reactor::getInstant();

    r->init();
    
}