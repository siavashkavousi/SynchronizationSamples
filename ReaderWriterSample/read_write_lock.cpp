//
// Created by sia on 11/6/15.
//

#include <fcntl.h>
#include "read_write_lock.h"

read_write_lock::read_write_lock(const char *name, int value, atomic<int> *readers, int *writers, int *write_requests) {
    this->value = value;
    this->readers = readers;
    this->writers = writers;
    this->write_requests = write_requests;
    semaphore = sem_open(name, O_CREAT | O_EXCL, 0644, value);
    sem_unlink(name);
}

void read_write_lock::lock_read() {
    while (writers[0] > 0 || write_requests[0] > 0);
    sem_wait(semaphore);
    if (++readers[0] == 1)
        cout << "The first reader acquires lock" << endl;
}

void read_write_lock::unlock_read() {
    if (--readers[0] == 0)
        cout << "The last reader releases lock" << endl;
    sem_post(semaphore);
}

void read_write_lock::lock_write() {
    write_requests++;

    while (readers[0] > 0 || writers[0] > 0);
    for (int i = 0; i < value; i++)
        sem_wait(semaphore);
    write_requests[0]--;
    writers[0]++;
    cout << "The writer acquires lock" << endl;
}


void read_write_lock::unlockWrite() {
    cout << "The writer releases lock" << endl;
    for (int i = 0; i < value; i++)
        sem_post(semaphore);
    writers[0]--;
}

