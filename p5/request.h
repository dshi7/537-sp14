#ifndef __REQUEST_H__

//  usage statistics in part 3
unsigned int *stat_thread_id;
unsigned int *stat_thread_count;
unsigned int *stat_thread_static;
unsigned int *stat_thread_dynamic;

typedef struct _node_t {
  int val;
  int size;
  int stat_req_arrival;
  int stat_req_dispatch;
  int stat_req_read;
  int stat_req_complete;
  int stat_req_birth;
  int stat_req_death;
  int stat_req_age;
  struct _node_t *next;
} node_t;

void requestHandle (int th_index, node_t *node);

int requestSize (int fd);

int isStaticRequest (int fd);

#endif
