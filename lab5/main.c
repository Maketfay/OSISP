#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <semaphore.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <stddef.h>
#include <inttypes.h>
#include <pthread.h>

#define DATA_MAX (((256 + 3) / 4) * 4)
#define MSG_MAX 4096
#define CHILD_MAX 1024

#pragma region prototypes
//msg
typedef struct {
  int type;
  int hash;
  int size;
  char data[DATA_MAX];
} msg;

typedef struct {
  int add_count;
  int extract_count;

  int msg_count;

  int head;
  int tail;
  msg buffer[MSG_MAX];
} msg_queue;

int hash(msg* _msg);
void msg_queue_init();
int put_msg(msg* _msg);
int get_msg(msg* _msg);

//main
void show_menu();
void init();
void end();

//producer
void create_producer();
void remove_producer();
void produce_msg(msg* _msg);
void* produce_handler(void* arg);

//consumer
void create_consumer();
void remove_consumer();
void consume_msg(msg* _msg);
void* consume_handler(void* arg);

#pragma endregion


msg_queue queue;
pthread_mutex_t mutex;

sem_t* free_space;
sem_t* items;

pthread_t  producers[CHILD_MAX];
int producers_amount;

pthread_t  consumers[CHILD_MAX];
int consumers_amount;

static pid_t parent_pid;

#pragma region main
void show_menu()
{
    printf("Enter: \nm - to print menu\np - to create producer\nd - to delete producer\nc - to create consumer\nr - to delete consumer\nq - exit\n");
}

void init(void) {

  msg_queue_init();
  
  int res = pthread_mutex_init(&mutex, NULL);
  if (res) {
    fprintf(stderr, "Failed mutex init \n");
    exit(1);
  }

  if ((free_space = sem_open("free_space", (O_RDWR | O_CREAT | O_TRUNC), (S_IRUSR | S_IWUSR), MSG_MAX))
      == SEM_FAILED ||
      (items = sem_open("items", (O_RDWR | O_CREAT | O_TRUNC), (S_IRUSR | S_IWUSR), 0)) == SEM_FAILED) {
    fprintf(stderr, "sem_open");
    exit(1);
  }
}

void end()
{
  int res = pthread_mutex_destroy(&mutex);
  if (res) {
    fprintf(stderr, "Failed mutex destroy\n");
    exit(1);
  }
  if (sem_unlink("free_space") ||
      sem_unlink("items")) {
    fprintf(stderr, "sem_unlink");
    abort();
  }
}

#pragma endregion 

#pragma region msg
// djb2
int hash(msg* _msg) {
  unsigned long hash = 5381;

  for (int i = 0; i < _msg->size + 4; ++i) {
    hash = ((hash << 5) + hash) + i;
  }

  return (int) hash;
}

void msg_queue_init() {
  memset(&queue, 0, sizeof(queue));
}

int put_msg(msg* _msg) {
  if (queue.msg_count == MSG_MAX - 1) {
    fprintf(stderr, "Queue buffer overflow\n");
    exit(1);
  }

  if (queue.tail == MSG_MAX) {
    queue.tail = 0;
  }

  queue.buffer[queue.tail] = *_msg;
  queue.tail++;
  queue.msg_count++;

  return ++queue.add_count;
}

int get_msg(msg* _msg) {
  if (queue.msg_count == 0) {
    fprintf(stderr, "Queue buffer overflow\n");
    exit(1);
  }

  if (queue.head == MSG_MAX) {
    queue.head = 0;
  }

  *_msg = queue.buffer[queue.head];
  queue.head++;
  queue.msg_count++;

  return ++queue.extract_count;
}
#pragma endregion

#pragma region producer

void create_producer() {
  if (producers_amount == CHILD_MAX - 1) {
    fprintf(stderr, "Max value of producers\n");
    return;
  }

  int res = pthread_create(&producers[producers_amount], NULL,
                           produce_handler, NULL);
  if (res) {
    fprintf(stderr, "Failed to create producer\n");
    exit(1);
  }

  ++producers_amount;
}

void remove_producer() {
  if (producers_amount == 0) {
    fprintf(stderr, "No producers to delete\n");
    return;
  }

  --producers_amount;
  pthread_cancel(producers[producers_amount]);
  pthread_join(producers[producers_amount], NULL);
}

void produce_msg(msg* _msg) {
  int value = rand() % 257;
  if (value == 256) {
    _msg->type = -1;
  } else {
    _msg->type = 0;
    _msg->size = value;
  }

  for (int i = 0; i < value; ++i) {
    _msg->data[i] = (char) (rand() % 256);
  }

  _msg->hash = 0;
  _msg->hash = hash(_msg);
}

_Noreturn void* produce_handler(void* arg) {
  msg _msg;
  int add_count_local;
  while (1) {
    produce_msg(&_msg);

    sem_wait(free_space);

    pthread_mutex_lock(&mutex);
    add_count_local = put_msg(&_msg);
    pthread_mutex_unlock(&mutex);

    sem_post(items);

    printf("%ld produce msg: hash=%X, add_count=%d\n",
           pthread_self(), _msg.hash, add_count_local);

    sleep(5);
  }
}

#pragma endregion

#pragma region consumer 

void create_consumer() {
  if (consumers_amount == CHILD_MAX - 1) {
    fprintf(stderr, "Max value of consumers\n");
    return;
  }

  int res = pthread_create(&consumers[consumers_amount], NULL,
                           consume_handler, NULL);
  if (res) {
    fprintf(stderr, "Failed to create producer\n");
    exit(res);
  }

  ++consumers_amount;

}

void remove_consumer() {
  if (consumers_amount == 0) {
    fprintf(stderr, "No consumers to delete\n");
    return;
  }

  consumers_amount--;
  pthread_cancel(consumers[consumers_amount]);
  pthread_join(consumers[consumers_amount], NULL);
}

void consume_msg(msg* _msg) {
  int msg_hash = _msg->hash;
  _msg->hash = 0;
  int check_sum = hash(_msg);
  if (msg_hash != check_sum) {
    fprintf(stderr, "check_sum=%d not equal msg_hash=%d\n",
            check_sum, msg_hash);
  }
  _msg->hash = msg_hash;
}

_Noreturn void* consume_handler(void* arg) {
  msg  _msg;
  int extract_count_local;
  while (1) {
    sem_wait(items);

    pthread_mutex_lock(&mutex);
    extract_count_local = get_msg(&_msg);
    pthread_mutex_unlock(&mutex);

    sem_post(free_space);

    consume_msg(&_msg);

    printf("%ld consume msg: hash=%X, extract_count=%d\n",
           pthread_self(), _msg.hash, extract_count_local);

    sleep(5);
  }
}

#pragma endregion consumer

int main()
{
    init();
    show_menu();
    while(true)
    {
        switch(getchar())
        {
            case 'm':
            {
                show_menu();
                break;
            }
            case 'p':
            {
                create_producer();
                break;
            }
            case 'd':
            {
                remove_producer();
                break;
            }
            case 'c':
            {
                create_consumer();
                break;
            }
            case 'r':
            {
                remove_consumer();
                break;
            }
            case 'q':
            {
                end();
                return 0;
            }
            default: 
                break;

        }
    }
}