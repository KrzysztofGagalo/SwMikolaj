#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <time.h>
#include <wait.h>

#define NUM_REINDEERS 9    // Liczba reniferów
#define NUM_ELVES 10       // Liczba elfów
#define ELF_GROUP 3        // Liczba elfów potrzebnych, aby Santa mógł pomóc

// Struktura wiadomości do komunikacji IPC (Inter-Process Communication)
typedef struct {
    long type;            // Typ wiadomości
    char text[100];       // Treść wiadomości
} Message;



// Funkcja generująca wiadomości i symulująca przybycie elfów i reniferów
void generate_messages(int msg_id) {
    srand(time(NULL));  // Inicjalizacja generatora liczb losowych

    int elves_waiting = 0;   // Liczba oczekujących elfów
    int reindeers_ready = 0; // Liczba gotowych reniferów

    int elves_status[NUM_ELVES] = {0}; // Status elfów (0 - nie dotarł, 1 - czeka)
    int reindeers_status[NUM_REINDEERS] = {0}; // Status reniferów (0 - nie dotarł, 1 - czeka)

    while (1) {
        Message msg;          // Struktura wiadomości
        msg.type = 1;         // Ustalamy typ wiadomości na 1

        if (rand() % 2 == 0) {
            // Jeśli losowanie daje 0, to przybywa elf
            int elf_id;
            do {
                elf_id = rand() % NUM_ELVES; // Wybieramy losowego elfa
            } while (elves_status[elf_id] == 1); // Jeśli elf już czeka, wybieramy innego

            elves_status[elf_id] = 1;    // Ustawiamy status elfów
            elves_waiting++;              // Zwiększamy liczbę oczekujących elfów
            snprintf(msg.text, sizeof(msg.text), "Elf %d arrived", elf_id + 1);
            printf("%s (Total elves waiting: %d)\n", msg.text, elves_waiting);
        } else {
            // Jeśli losowanie daje 1, to przybywa renifer
            int reindeer_id;
            do {
                reindeer_id = rand() % NUM_REINDEERS; // Wybieramy losowego renifera
            } while (reindeers_status[reindeer_id] == 1); // Jeśli renifer już czeka, wybieramy innego

            reindeers_status[reindeer_id] = 1; // Ustawiamy status reniferów
            reindeers_ready++;                 // Zwiększamy liczbę gotowych reniferów
            snprintf(msg.text, sizeof(msg.text), "Reindeer %d arrived", reindeer_id + 1);
            printf("%s (Total reindeers ready: %d)\n", msg.text, reindeers_ready);
        }

        // Wysyłamy wiadomość
        msgsnd(msg_id, &msg, sizeof(msg.text), 0);

        // Sprawdzamy, czy można pomóc elfom
        if (elves_waiting == ELF_GROUP) {
            printf("Santa: Helping elves!\n");
            sleep(1); // Symulacja pomocy elfom
            elves_waiting = 0; // Resetujemy licznik elfów
            for (int i = 0; i < NUM_ELVES; i++) {
                elves_status[i] = 0; // Resetujemy status wszystkich elfów
            }
            printf("Santa: Finished helping elves.\n");
        }

        // Sprawdzamy, czy można przygotować sanie z reniferami
        if (reindeers_ready == NUM_REINDEERS) {
            printf("Santa: Preparing sleigh with reindeers!\n");
            sleep(2); // Symulacja przygotowywania sań
            reindeers_ready = 0; // Resetujemy licznik reniferów
            for (int i = 0; i < NUM_REINDEERS; i++) {
                reindeers_status[i] = 0; // Resetujemy status wszystkich reniferów
            }
            printf("Santa: Delivered toys and reindeers are ready to rest.\n");
        }

        sleep(rand() % 2 + 1); // Losowy czas oczekiwania przed następną akcją
    }
}

int main() {
    // Tworzymy kolejkę wiadomości IPC
    int msg_id = msgget(IPC_PRIVATE, IPC_CREAT | 0666); //msgget zwraca identyfikator kolejki

    // Tworzymy proces potomny
    if (fork() == 0) {
        generate_messages(msg_id); // Proces potomny generuje wiadomości
        exit(0);  // Zakończenie procesu potomnego
    }

    wait(NULL); // Czekamy na zakończenie procesu potomnego
    msgctl(msg_id, IPC_RMID, NULL); // Usuwamy kolejkę wiadomości po zakończeniu

    return 0; // Zakończenie programu
}