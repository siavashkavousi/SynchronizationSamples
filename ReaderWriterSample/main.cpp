#include <iostream>
#include <unistd.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/wait.h>

using namespace std;

int main() {
    const char *name = "/value";
    sem_t *semaphore1, *semaphore2;
    pid_t pid;
    int *val = 0;
    int fd;

    fd = shm_open(name, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        perror("failed to create shared memory");
        exit(EXIT_FAILURE);
    }
    if ((ftruncate(fd, sizeof(val))) == -1) {
        perror("failed to set size of the shared memory");
        exit(EXIT_FAILURE);
    }

    val = (int *) mmap(NULL, sizeof(val), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (val == MAP_FAILED) {
        perror("mapping failed");
    }
    cout << "ptr " << *val << " is allocated in shared memory" << endl;

    // initialize semaphores for shared processes
    semaphore1 = sem_open("p_sem_1", O_CREAT | O_EXCL, 0644, 1);
    semaphore2 = sem_open("p_sem_2", O_CREAT | O_EXCL, 0644, 1);

    sem_unlink("p_sem_1");
    sem_unlink("p_sem_2");
    cout << "semaphores initialized" << endl;

    pid = fork();
    if (pid < 0) {
        perror("fork failed\n");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        while (val[0] < 10) {
            sem_wait(semaphore1);
            cout << "reader " << getpid() << " acquires lock 1" << endl;
            cout << "reader " << getpid() << " reads " << val[0] << endl;
            sleep(1);
            sem_post(semaphore1);
            cout << "reader " << getpid() << " releases lock 1" << endl;
        }
    } else {

        pid = fork();
        if (pid < 0) {
            perror("fork failed\n");
            exit(EXIT_FAILURE);
        }
        if (pid == 0) {
            while (val[0] < 10) {
                sem_wait(semaphore2);
                cout << "reader " << getpid() << " acquires lock 2" << endl;
                cout << "reader " << getpid() << " reads " << val[0] << endl;
                sleep(1);
                sem_post(semaphore2);
                cout << "reader " << getpid() << " releases lock 2" << endl;
            }
        }

        // parent process
        while (val[0] < 10) {
            sem_wait(semaphore1);
            cout << "writer " << getpid() << " acquires lock 1" << endl;
            sem_wait(semaphore2);
            cout << "writer " << getpid() << " acquires lock 2" << endl;
            val[0]++;
            cout << "writer " << getpid() << " writes " << val[0] << endl;
            sleep(1);
            cout << "writer " << getpid() << " releases lock 1" << endl;
            sem_post(semaphore1);
            cout << "writer " << getpid() << " releases lock 2" << endl;
            sem_post(semaphore2);
        }
    }

    while (pid == waitpid(-1, 0, NULL)) {
        if (errno == ECHILD)
            break;
    }

    shm_unlink(name);
    /* cleanup semaphores */
    sem_destroy(semaphore1);
    sem_destroy(semaphore2);

    exit(EXIT_SUCCESS);
}