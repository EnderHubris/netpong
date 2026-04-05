#include "rules.h"

int setTicker(int nMSecs){
    struct itimerval newTimeset;
    long nSec,nUSecs;

    nSec=nMSecs/1000;
    nUSecs=(nMSecs%1000)*1000L;

    newTimeset.it_interval.tv_sec=nSec;
    newTimeset.it_interval.tv_usec=nUSecs;
    newTimeset.it_value.tv_sec=nSec;
    newTimeset.it_value.tv_usec=nUSecs;

    // after the timer expires SIGALRM is sent to the process
    // this is the trigger for the signal(SIGALRM, foo);
    return setitimer(ITIMER_REAL,&newTimeset,NULL);
}
