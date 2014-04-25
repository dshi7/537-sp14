#include "cs537.h"
#include "request.h"

pthread_mutex_t mutex;
pthread_cond_t not_empty;
pthread_cond_t not_full;

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
pthread_mutex_t *work_locks;

typedef struct _queue 
{
  node_t *head;
  node_t *tail;
} queue;

queue buf_wait_queue;

void  queue_info (queue *wait_queue) 
{
  node_t *tmp;
  printf("queue : ");
  for ( tmp=wait_queue->head; tmp->next != NULL; tmp = tmp->next )
    printf("%8d  ", tmp->next->val);
  printf("\n");
}

void  queue_init (queue *wait_queue) 
{
  //  Create a sentinel
  node_t *new_node = malloc(sizeof(node_t));
  new_node->next = NULL;
  wait_queue->head = new_node;
  wait_queue->tail = new_node;
  buffer_work_num = 0;
  buffer_wait_num = 0;
}

void   queue_push (queue *wait_queue, int val) 
{

  struct timeval  arrival;

  gettimeofday (&arrival, NULL);

  if ( sched_alg_code==0 ) {
    //  FIFO  or dynamic request
    node_t *new_node = malloc(sizeof(node_t));
    new_node->val = val;
    new_node->st_size = 0;
    new_node->stat_req_arrival = (int)(arrival.tv_sec/1000 + arrival.tv_usec*1000);
    new_node->stat_req_birth = buffer_head_hist;
    //  pre-process the request
    requestPreProcess (new_node);
    new_node->next = NULL;
    wait_queue->tail->next = new_node;
    wait_queue->tail = wait_queue->tail->next;
  }
  else if ( sched_alg_code==1  ) { //  SFF static request
    node_t *new_node = malloc(sizeof(node_t));
    new_node->val = val;
    new_node->stat_req_arrival = (int)(arrival.tv_sec/1000 + arrival.tv_usec*1000);
    new_node->stat_req_birth = buffer_head_hist;
    //
    node_t *tmp = wait_queue->head;
    //  pre-process the request
    requestPreProcess (new_node);
    while ( tmp->next!=NULL && (tmp->next->st_size <= new_node->st_size) )
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
    new_node->stat_req_arrival = (int)(arrival.tv_sec/1000 + arrival.tv_usec*1000);
    new_node->stat_req_birth = buffer_head_hist;
    //
    node_t *tmp = wait_queue->head;
    //  pre-process the request
    requestPreProcess (new_node);
    int  max_search = sff_bs_value - buffer_head_hist%sff_bs_value;
    int  cnt_search = 1;
    while ( cnt_search<max_search && tmp->next!=NULL && (tmp->next->st_size <= new_node->st_size) ) {
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

node_t queue_peek (queue *wait_queue) {
  node_t  ret_node;
  ret_node = *(wait_queue->head->next);
  return ret_node;
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
  if ( strcmp(schedalg, "SFF-BS")==0 ) 
    sched_alg_code = 2;
  if ( argc==6 )
    *sff_bs_value = atoi(argv[5]);
}

//  Producer functions
void  accept_request (int fd)
{
  queue_push (&buf_wait_queue, fd);
  ++ buffer_wait_num;
}

//  Consumer functions
void  handle_request (int th_index)
{
  node_t node = queue_peek(&buf_wait_queue);
  int connfd = node.val;

//  printf ("handle request : %d\n", connfd);

  if (connfd==-1) {
    printf("Error : request fd is -1.\n");
    return;
  }

  queue_pop(&buf_wait_queue);

  //  Stat-req-dispatch
  struct timeval pickup_time;
  gettimeofday (&pickup_time, NULL);
  int pickup_time_ms = (int)(pickup_time.tv_sec/1000 + pickup_time.tv_usec*1000);
  node.stat_req_dispatch = pickup_time_ms - node.stat_req_arrival;

  ++ stat_thread_count[th_index];
  if ( node.is_static )
    ++ stat_thread_static[th_index];
  else
    ++ stat_thread_dynamic[th_index];

  node.stat_req_death = buffer_head_hist;
  buffer_head_hist++;

  buffer_work_num++;
  buffer_wait_num--;

  if ( node.is_static )
    requestServeStatic (th_index, &node, node.filename, node.st_size);
  else
    requestServeDynamic (th_index, connfd, node.filename, node.cgiargs);

  Close (connfd);

  buffer_work_num--;
}

void  *Worker_thread_handle_request (void *t) 
{
  int th_index = *(int*)t;
  while (1) {
    pthread_mutex_lock (&work_locks[th_index]);
    while ( buffer_wait_num==0 )
      pthread_cond_wait (&not_empty, &work_locks[th_index]);
    handle_request (th_index);
    pthread_cond_signal (&not_full);
    pthread_mutex_unlock (&work_locks[th_index]);
  }
  return t;
}

void Master_thread_accept_request (int request_fd)
{
  pthread_mutex_lock (&mutex);
  while ( buffer_work_num+buffer_wait_num==max_buffers )
    pthread_cond_wait (&not_full, &mutex);
  accept_request (request_fd);
  pthread_cond_signal (&not_empty);
  pthread_mutex_unlock (&mutex);
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
  pthread_cond_init (&not_empty, NULL);
  pthread_cond_init (&not_full, NULL);
  
  // 
  // CS537: TODO create some worker threads using pthread_create ...
  //


  //  Create a pool of worker threads
  int rc, t;
  work_threads = (pthread_t*)calloc(max_threads, sizeof(pthread_t));
  work_locks = (pthread_mutex_t*)calloc(max_threads, sizeof(pthread_mutex_t));
  stat_thread_id = (int*)calloc(max_threads, sizeof(int));
  stat_thread_count = (int*)calloc(max_threads, sizeof(int));
  stat_thread_static = (int*)calloc(max_threads, sizeof(int));
  stat_thread_dynamic = (int*)calloc(max_threads, sizeof(int));

  //  0 : wait ; 1 : work
  for ( t=0; t<max_threads; t++ ) {

    pthread_mutex_init (&work_locks[t], NULL);

    rc = pthread_create (&work_threads[t], NULL, &Worker_thread_handle_request, (void*)&t);

    //  Initialize those usage statistics
    stat_thread_id[t] = t;
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


