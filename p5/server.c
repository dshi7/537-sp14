#include "cs537.h"
#include "request.h"

pthread_mutex_t mutex;
pthread_cond_t cond;

int max_threads;
int max_buffers;
int buffer_demand;  
int sff_bs_value;

//  store worker threads in array
pthread_t *work_threads;
int *work_threads_status; 

typedef struct _node_t {
  int val;
  struct _node_t *next;
} node_t;

typedef struct _queue {
  node_t *head;
  node_t *tail;
} queue;

queue buf_wait_queue;

void  queue_init (queue &wait_queue) {
  //  Create a sentinel
  node_t *new_node = malloc(sizeof(node_t));
  new_node->next = NULL;
  wait_queue->head = new_node;
  wait_queue->tail = new_node;
}

void   queue_push (queue &wait_queue, int val) {
  node_t *new_node = malloc(sizeof(node_t));
  new_node->val = val;
  new_node->next = NULL;
  wait_queue->tail->next = new_node;
  wait_queue->tail = new_node;
}

int queue_peek (queue &wait_queue) {
  return  wait_queue->head->next->val;
}

void  pop (queue &wait_queue) {
  wait_queue->head->next = wait_queue->head->next->next;
}


// 
// server.c: A very, very simple web server
//
// To run:
//  server <portnum (above 2000)>
//
// Repeatedly handles HTTP requests sent to this port number.
// Most of the work is done within routines written in request.c
//

// CS537: TODO make it parse the new arguments too
void getargs(int *port, int *threads, int *buffers, char *schedalg, int *sff_bs_value, int argc, char *argv[])
{
  printf("%d\n", argc );
  if (argc < 5 || argc > 6) {
    fprintf(stderr, "Usage: %s [portnum] [threads] [buffers] [schedalg] [N (for SFF-BS only)]\n", argv[0]);
    exit(1);
  }
  *port = atoi(argv[1]);
  *threads = atoi(argv[2]);
  *buffers = atoi(argv[3]);
  strcpy (schedalg, argv[4]);
  if ( argc == 5 )
    return;
  else if ( argc == 6 && strcmp(schedalg, "SFF-BS")==0 )
    *sff_bs_value = atoi(argv[5]);
  else {
    fprintf(stderr, "Usage: %s [portnum] [threads] [buffers] [schedalg] [N (for SFF-BS only)]\n", argv[0]);
    exit(1);
  }
}

//  Producer functions
void  accept_request (int request_fd)
{
  if ( head==-1 ) {
    buffer_pool[0] = request_fd;
    head = 0; tail = 1;
  }
  else {
    buffer_pool[tail] = request_fd;
    tail = (tail+1)%max_buffers;
  }
  buffer_demand++;
}

void  Master_thread_accept_request (int request_fd)
{
  pthread_mutex_lock (&mutex);
  while ( buffer_demand==max_buffers )
    pthread_cond_wait (&cond, &mutex);
  accept_request (request_fd);
  pthread_cond_broadcast (&cond);
  pthread_mutex_unlock (&mutex);
}

//  Consumer functions
void  handle_request (void)
{
  
}

void  Worker_thread_handle_request (void) {
  pthread_mutex_lock (&mutex);
  while ( buffer_demand==0 )
    pthread_cond_wait (&cond, &mutex);
  handle_request ();
  pthread_cond_broadcast (&cond);
  pthread_mutex_unlock (&mutex);
}

int main(int argc, char *argv[])
{
  int listenfd, connfd, port, clientlen;
  
  char  schedalg[MAXLINE];
  struct sockaddr_in clientaddr;

  getargs(&port, &max_threads, &max_buffers, schedalg, &sff_bs_value, argc, argv);

  //  Initialize the buffer pool
  buffer_pool = calloc(max_buffers, sizeof(int));
  head = -1;
  tail = -1;
  buffer_demand = 0;

  //  Create a pool of worker threads
  int rc, t;
  work_threads = (pthread_t*)calloc(max_threads, sizeof(pthread_t));
  for ( t=0; t<max_threads; t++ ) {
    rc = pthread_create (&work_threads[t], NULL, Worker_thread_handle_request, NULL);
    if (rc) {
      fprintf(stderr, "ERROR : return code from pthread_create() is %d\n", rc);
      exit(-1);
    }
  }

  //  Initialize mutex and condition variable objects
  pthread_mutex_init (&mutex, NULL);
  pthread_cond_init (&cond, NULL);
  

  //  Create a consumer thread

  // 
  // CS537: TODO create some worker threads using pthread_create ...
  //

  listenfd = Open_listenfd(port);

  pthread_cond_t myconvar = PTHREAD_COND_INITIALIZER;

  while (1) {
    clientlen = sizeof(clientaddr);
    connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);

    Master_thread_accept_request (connfd);

    // 
    // CS537: In general, don't handle the request in the main thread.  TODO In
    // multi-threading mode, the main thread needs to save the relevant info in
    // a buffer and have one of the worker threads do the work.
    // 
    requestHandle(connfd);

    Close(connfd); }

}






