#include <sys/timerfd.h>
#include <sys/select.h>
#include <unistd.h>
#include <iostream>
#include <thread>

using namespace std;

int main() {
    auto fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if(fd == -1) {
        std::cerr << "error creating interval fd" << std::endl;
        exit(-1);
    }
    
    itimerspec interval{{0, 1000000}, {0, 1000000}};
    if(timerfd_settime(fd, 0, &interval, nullptr) == -1) {
        std::cerr << "error setting interval fd" << std::endl;
        exit(-1);
    }


    auto func = [=]{
        auto countMoreTicks = 0;
        uint64_t countTicks = 0;
        fd_set rfds;
        auto maxFd = fd + 1;
        while(1) {
            FD_ZERO(&rfds);
            FD_SET(fd, &rfds);
            auto ret = select(maxFd, &rfds, nullptr, nullptr, nullptr);
            cout<<"ret = "<<ret<<endl;
            if(ret == -1) {
                std::cerr << "failed to select" << std::endl;
                exit(-1);
            }
            
            if(FD_ISSET(fd, &rfds)) {
                uint64_t ticks;
                if(read(fd, &ticks, sizeof(ticks)) == -1) {
                    std::cerr << "failed to read timerfd" << std::endl;
                    exit(-1);
                }
                
                    cout<<"ticks = "<<ticks<<endl;
                if(ticks > 1) {
                    countMoreTicks++;
                }
                countTicks += ticks;
            }
        }
        };
    thread t(func);
    t.join();
}