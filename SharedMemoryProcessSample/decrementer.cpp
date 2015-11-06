#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdlib.h>
#include <iostream>
#include <atomic>

using namespace std;

// with race condition
//int *ptr;
// without race condition;
atomic<int> *counter;

int main() {
    const char *name = "counter";
    int fd;

    fd = shm_open(name, O_RDWR, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        cout << "failed to create shared memory " << endl;
        exit(EXIT_FAILURE);
    }

    counter = (atomic<int> *) mmap(NULL, sizeof(counter), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (counter == MAP_FAILED) {
        std::cout << "mapping failed";
    }

    for (int i = 0; i < 1000; i++) {
        nanosleep((const struct timespec[]) {{0, 50000000L}}, NULL);
        // without race condition
        int oldValue = counter->load();
        cout << "counter: " << oldValue << endl;
        counter->fetch_sub(1);
        // with race condition
//        cout << "counter: " << ptr[0] << endl;
//        ptr[0]--;
    }

    cout << "final counter: " << counter->load() << endl;

    shm_unlink(name);

    exit(EXIT_SUCCESS);
}