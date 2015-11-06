#include <iostream>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "read_write_lock.h"

using namespace std;

struct shared_values {
    int val = 0;
    atomic<int> readers{0};
    int writers = 0;
    int write_requests = 0;
};

int main() {
    const char *name = "/value";
    pid_t pid;
    int fd;
    shared_values *shared_values1;

    fd = shm_open(name, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        perror("failed to create shared memory");
        exit(EXIT_FAILURE);
    }
    if ((ftruncate(fd, sizeof(shared_values1))) == -1) {
        perror("failed to set size of the shared memory");
        exit(EXIT_FAILURE);
    }

    shared_values1 = (shared_values *) mmap(NULL, sizeof(shared_values1), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shared_values1 == MAP_FAILED) {
        perror("mapping failed");
    }
    cout << "ptr " << shared_values1 << " is allocated in shared memory" << endl;
    read_write_lock read_write_lock("sem", 2, &shared_values1->readers, &shared_values1->writers,
                                    &shared_values1->write_requests);

    pid = fork();
    if (pid < 0) {
        perror("fork failed\n");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        while (shared_values1->val < 4) {
            nanosleep((const struct timespec[]) {{0, 4L}}, NULL);
            read_write_lock.lock_read();
            cout << "The reader " << getpid() << " reads " << shared_values1->val << endl;
            read_write_lock.unlock_read();
        }
    } else {

        pid = fork();
        if (pid < 0) {
            perror("fork failed\n");
            exit(EXIT_FAILURE);
        }
        if (pid == 0) {
            while (shared_values1->val < 4) {
                nanosleep((const struct timespec[]) {{0, 60L}}, NULL);
                read_write_lock.lock_read();
                cout << "The reader " << getpid() << " reads " << shared_values1->val << endl;
                read_write_lock.unlock_read();
            }
        }

        // parent process
        while (shared_values1->val < 4) {
            nanosleep((const struct timespec[]) {{0, 80L}}, NULL);
            read_write_lock.lock_write();
            shared_values1->val++;
            cout << "The writer " << getpid() << " writes " << shared_values1->val << endl;
            read_write_lock.unlockWrite();
        }
    }

    while (pid == waitpid(-1, 0, NULL)) {
        if (errno == ECHILD)
            break;
    }

    shm_unlink(name);

    exit(EXIT_SUCCESS);
}