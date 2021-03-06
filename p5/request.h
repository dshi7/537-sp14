#ifndef __REQUEST_H__

//  usage statistics in part 3
int *stat_thread_id;
int *stat_thread_count;
int *stat_thread_static;
int *stat_thread_dynamic;

typedef struct _node_t {
  int val;
  int is_static;
  char filename[8192];
  char cgiargs[8192];
  int st_size;
  int stat_req_arrival;
  int stat_req_dispatch;
  int stat_req_read;
  int stat_req_complete;
  int stat_req_birth;
  int stat_req_death;
  int stat_req_age;
  struct _node_t *next;
} node_t;

void requestServeStatic(int th_index, node_t *node, char *filename, int filesize); 

void requestServeDynamic(int th_index, int fd, char *filename, char *cgiargs);

void requestHandle (int th_index, node_t *node);

void requestPreProcess (node_t *node); 

int requestSize (int fd);

int isStaticRequest (int fd);

#endif
