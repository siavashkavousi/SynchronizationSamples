#include <iostream>
#include <unistd.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/wait.h>

using namespace std;

int main() {
    key_t shm_key;
    int shm_id;
    sem_t *semaphore1, *semaphore2;
    pid_t pid;
    int *val;

    shm_key = ftok("dev/null", 5);
    cout << "shm key for val " << shm_key << endl;
    if ((shm_id = shmget(shm_key, sizeof(int), 0644 | IPC_CREAT)) < 0) {
        perror("shmget\n");
        exit(EXIT_FAILURE);
    }

    val = (int *) shmat(shm_id, NULL, 0);
    *val = 0;
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
            sem_post(semaphore1);
            cout << "writer " << getpid() << " releases lock 1" << endl;
            sem_post(semaphore2);
            cout << "writer " << getpid() << " releases lock 2" << endl;
        }

        /* shared memory detach */
        shmdt(val);
        shmctl(shm_id, IPC_RMID, 0);
        /* cleanup semaphores */
        sem_destroy(semaphore1);
        sem_destroy(semaphore2);
    }

    while (pid == waitpid(-1, 0, NULL)) {
        if (errno == ECHILD)
            break;
    }

    exit(EXIT_SUCCESS);
}