#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdlib.h>
#include <iostream>
#include <atomic>

using namespace std;

// with race condition
int *ptr;
// without race condition;
atomic<int> *counter;

int main() {
    const char *name = "counter";
    int fd;

    fd = shm_open(name, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        perror("failed to create shared memory");
        exit(EXIT_FAILURE);
    }
    if ((ftruncate(fd, sizeof(ptr))) == -1) {
        perror("failed to set size of the shared memory");
        exit(EXIT_FAILURE);
    }

    counter = (atomic<int> *) mmap(NULL, sizeof(counter), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (counter == MAP_FAILED) {
        perror("mapping failed");
    }

    for (int i = 0; i < 1000; i++) {
        nanosleep((const struct timespec[]) {{0, 50000000L}}, NULL);
        // without race condition
        int oldValue = counter->load();
        cout << "counter: " << oldValue << endl;
        counter->fetch_add(1);
        // with race condition
//        cout << "counter: " << ptr[0] << endl;
//        ptr[0]++;
    }

    exit(EXIT_SUCCESS);
}