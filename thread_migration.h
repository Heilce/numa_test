#include <iostream>
#include <thread>
#include <set>
#include <mutex>
#include <string.h>

class MigrateThread {
private:
    static std::set<int> sharedSet;  // 共享的set变量
    static int globalCounter;  // 全局计数器
    static cpu_set_t cpuset;
    static bool initialized;

public:
    void initializeCpuSet() {
        CPU_ZERO(&cpuset);
        int test_core_id = 2;
        CPU_SET(test_core_id, &cpuset);
    }

    void addToSharedSet(int value) {
        // 向共享set中添加元素
        sharedSet.insert(value);
    }

    void xorGlobalCounter() {
        // 增加全局变量
        globalCounter ^= 1;
    }

    void manipulateThread(pthread_t this_tid) {
        if (!initialized) {
            initializeCpuSet();
            initialized = true;
        }
        if (sharedSet.count(this_tid)) return;
        addToSharedSet(this_tid);
        xorGlobalCounter();

        if (globalCounter & 1) {
            // 将该线程绑到numa1
            int rc = pthread_setaffinity_np(this_tid, sizeof(cpu_set_t), &cpuset);
            if (rc != 0) {
                fprintf(stderr, "pthread_setaffinity error: %s\n", strerror(rc));
            }
        }
    }
};

bool MigrateThread::initialized = false;
int MigrateThread::globalCounter = 0;
std::set<int> MigrateThread::sharedSet;
cpu_set_t MigrateThread::cpuset;