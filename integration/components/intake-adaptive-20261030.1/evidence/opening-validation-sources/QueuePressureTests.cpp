#include <Research/FlightRecorder/Queue.h>
#include <iostream>
#include <stdexcept>
unsigned checks=0;
void check(bool ok){++checks;if(!ok)throw std::runtime_error("queue assertion");}
int main(){try{
    Qfr::Queue<int> q(8);check(q.tryPush(1,4,4));check(!q.tryPush(2,1,4));
    check(q.tryPush(3,4,8));check(!q.tryPush(4,1,8));check(q.stalls==0);
    unsigned rejected=0;for(unsigned i=0;i<100000;++i)if(!q.tryPush(5,1,8))++rejected;
    check(rejected==100000&&q.stalls==0);int v=0;check(q.pop(v)&&v==1);check(q.pop(v)&&v==3);
    check(!q.tryPush(6,9,8));q.close();check(!q.tryPush(7,1,8));check(!q.pop(v));
    Qfr::Queue<int> failed(8);failed.fail();check(!failed.tryPush(1,1,8));
    std::cout<<"PASS "<<checks<<" queue-pressure assertions;100000 nonblocking rejections\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
