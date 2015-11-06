#include <semaphore.h>
#include <iostream>
#include <mutex>
#include <atomic>
//
// Created by sia on 11/6/15.
//

#ifndef READERWRITERSAMPLE_READWRITELOCK_H
#define READERWRITERSAMPLE_READWRITELOCK_H

using namespace std;

class read_write_lock {
private:
    sem_t *semaphore;
    int value;
public:
    atomic<int> *readers;
    int *writers;
    int *write_requests;

    read_write_lock(const char *name, int value, atomic<int> *readers, int *writers, int *write_requests);

    void lock_read();

    void unlock_read();

    void lock_write();

    void unlockWrite();
};


#endif //READERWRITERSAMPLE_READWRITELOCK_H
