#include "cs537.h"
#include "request.h"

pthread_mutex_t mutex;
pthread_cond_t cond;

int max_threads;
int max_buffers;
int buffer_work_num;
int buffer_wait_num;
int buffer_head_hist = 0; //  for SFF-BS
int sff_bs_value;

int sched_alg_code;  
//  0 : FIFO
//  1 : SFF
//  2 : SFF-BS
//  -1: invalid

//  store worker threads in array
pthread_t *work_threads;
int *work_threads_status; 

//  usage statistics in part 3
unsigned int *stat_thread_id;
unsigned int *stat_thread_count;
unsigned int *stat_thread_static;
unsigned int *stat_thread_dynamic;

void  status_info (int *work_threads_status, int max_threads) {
  int i;
  printf("status : ");
  for (i=0; i<max_threads; i++)
    printf("%8d", work_threads_status[i]);
  printf("\n");
}

void  buffer_usage (void) {
  printf ("Buffer Usage : %d waiting and %d running\n", buffer_wait_num, buffer_work_num);
}

typedef struct _node_t {
  int val;
  int size;
  struct _node_t *next;
} node_t;

typedef struct _queue {
  node_t *head;
  node_t *tail;
} queue;

queue buf_wait_queue;

void  queue_info (queue *wait_queue) {
  node_t *tmp;
  printf("queue : ");
  for ( tmp=wait_queue->head; tmp->next != NULL; tmp = tmp->next )
    printf("%8d  ", tmp->next->val);
  printf("\n");
}

void  queue_init (queue *wait_queue) {
  //  Create a sentinel
  node_t *new_node = malloc(sizeof(node_t));
  new_node->next = NULL;
  wait_queue->head = new_node;
  wait_queue->tail = new_node;
  buffer_work_num = 0;
  buffer_wait_num = 0;
}

void   queue_push (queue *wait_queue, int val) {
  if ( sched_alg_code==0 ) {
    //  FIFO  or dynamic request
    node_t *new_node = malloc(sizeof(node_t));
    new_node->val = val;
    new_node->size = 0;
    new_node->next = NULL;
    wait_queue->tail->next = new_node;
    wait_queue->tail = wait_queue->tail->next;
  }
  else if ( sched_alg_code==1  ) { //  SFF static request
    node_t *new_node = malloc(sizeof(node_t));
    new_node->val = val;
    new_node->size = requestSize (val);
    //
    node_t *tmp = wait_queue->head;
    while ( tmp->next!=NULL && (tmp->next->size < new_node->size) )
      tmp = tmp->next;  //  tmp is not NULL
    // insert here and break
    new_node->next = tmp->next;
    tmp->next = new_node;
    if (tmp->next->next==NULL)
      wait_queue->tail = tmp->next;
  }
  else if ( sched_alg_code==2  ) { //  SFF-BS static request
    node_t *new_node = malloc(sizeof(node_t));
    new_node->val = val;
    new_node->size = requestSize (val);
    //
    node_t *tmp = wait_queue->head;
    int  max_search = sff_bs_value - buffer_head_hist%sff_bs_value;
    int  cnt_search = 1;
    while ( cnt_search<max_search && tmp->next!=NULL && (tmp->next->size < new_node->size) ) {
      tmp = tmp->next;  //  tmp is not NULL
      ++ cnt_search;
    }
    // insert here and break
    new_node->next = tmp->next;
    tmp->next = new_node;
    if (tmp->next->next==NULL)
      wait_queue->tail = tmp->next;
  }
}

int queue_peek (queue *wait_queue) {
  if ( wait_queue->head->next!=NULL )
    return  wait_queue->head->next->val;
  else
    return -1;
}

void  queue_pop (queue *wait_queue) {
  //  make sure tail->next is always NULL
  if (wait_queue->head->next!=NULL) {
    wait_queue->head->next = wait_queue->head->next->next;

    //  tricky!!!
    if (wait_queue->head->next == NULL)
      wait_queue->tail = wait_queue->head;
  }
  else
    printf ("Warning : pop an empty queue\n");
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
  if (argc == 1) {
    fprintf(stderr, "Usage: %s [portnum] [threads] [buffers] [schedalg] [N (for SFF-BS only)]\n", argv[0]);
    exit(1);
  }
  *port = atoi(argv[1]);
  *threads = atoi(argv[2]);
  *buffers = atoi(argv[3]);
  strcpy (schedalg, argv[4]);
  if ( strcmp(schedalg, "FIFO")==0 )
    sched_alg_code = 0;
  if ( strcmp(schedalg, "SFF")==0 )
    sched_alg_code = 1;
  if ( strcmp(schedalg, "SFF-BS")==0 ) {
    sched_alg_code = 2;
  if ( argc==6 )
    *sff_bs_value = atoi(argv[5]);
  }
}

//  Producer functions
void  accept_request (int request_fd)
{
//  printf ("accept request : %d\n", request_fd);
  queue_push (&buf_wait_queue, request_fd);
  ++ buffer_wait_num;
}

void  Master_thread_accept_request (int request_fd)
{
//  printf("request file descriptor: %d\n", request_fd);
  pthread_mutex_lock (&mutex);
  while ( buffer_work_num+buffer_wait_num==max_buffers )
    pthread_cond_wait (&cond, &mutex);
  accept_request (request_fd);
  pthread_cond_broadcast (&cond);
  pthread_mutex_unlock (&mutex);
}

//  Consumer functions
void  handle_request (void)
{
//  queue_info (&buf_wait_queue);

  int connfd = queue_peek(&buf_wait_queue);

//  printf ("handle request : %d\n", connfd);

  if (connfd==-1) {
    printf("Error : request fd is -1.\n");
    return;
  }

  queue_pop(&buf_wait_queue);
  buffer_head_hist++;

  buffer_work_num++;
  buffer_wait_num--;
  requestHandle (connfd);
  Close (connfd);
  buffer_work_num--;
}

void  *Worker_thread_handle_request (void *t) 
{
  while (1) {
    //  printf("thread %d starts\n", (int)t);
    pthread_mutex_lock (&mutex);
    //  printf("thread %d lock\n", (int)t);
    while ( buffer_wait_num==0 )
      pthread_cond_wait (&cond, &mutex);
    handle_request ();
    pthread_cond_broadcast (&cond);
    //  printf("thread %d unlock\n", (int)t);
    pthread_mutex_unlock (&mutex);
  }
  return t;
}

int main(int argc, char *argv[])
{
  int listenfd, connfd, port, clientlen;
  
  char  schedalg[MAXLINE];
  struct sockaddr_in clientaddr;

  getargs(&port, &max_threads, &max_buffers, schedalg, &sff_bs_value, argc, argv);

  queue_init(&buf_wait_queue);

  //  Initialize mutex and condition variable objects
  pthread_mutex_init (&mutex, NULL);
  pthread_cond_init (&cond, NULL);
  
  // 
  // CS537: TODO create some worker threads using pthread_create ...
  //


  //  Create a pool of worker threads
  int rc, t;
  work_threads = (pthread_t*)calloc(max_threads, sizeof(pthread_t));
  work_threads_status = (int*)calloc(max_threads, sizeof(int)); 
  stat_thread_id = (unsigned int*)calloc(max_threads, sizeof(unsigned int));
  stat_thread_count = (unsigned int*)calloc(max_threads, sizeof(unsigned int));
  stat_thread_static = (unsigned int*)calloc(max_threads, sizeof(unsigned int));
  stat_thread_dynamic = (unsigned int*)calloc(max_threads, sizeof(unsigned int));

  //  0 : wait ; 1 : work
  for ( t=0; t<max_threads; t++ ) {
    work_threads_status[t] = 0;
    rc = pthread_create (&work_threads[t], NULL, &Worker_thread_handle_request, (void*)&t);

    //  Initialize those usage statistics
    stat_thread_id[t] = work_threads[t];
    stat_thread_count[t] = 0;
    stat_thread_static[t] = 0;
    stat_thread_dynamic[t] = 0;
      
    if (rc) {
      fprintf(stderr, "ERROR : return code from pthread_create() is %d\n", rc);
      exit(-1);
    }
  }

  listenfd = Open_listenfd(port);

  while (1) {
    clientlen = sizeof(clientaddr);
    connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);
    Master_thread_accept_request (connfd);
  }

}


