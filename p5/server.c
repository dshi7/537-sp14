#include "cs537.h"
#include "request.h"

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


int main(int argc, char *argv[])
{
  int listenfd, connfd, port, clientlen;
  int threads, buffers, sff_bs_value;
  char  schedalg[MAXLINE];
  struct sockaddr_in clientaddr;
  pthread_t *work_threads;
  int *buf_pool;

  getargs(&port, &threads, &buffers, schedalg, &sff_bs_value, argc, argv);

  //  create work_threads
  work_threads = (pthread_t*)calloc(threads, sizeof(pthread_t));
  //  create buffer pool
  buf_pool = (int*)calloc(buffers, sizeof(int));

  return 0;

  // 
  // CS537: TODO create some worker threads using pthread_create ...
  //

  listenfd = Open_listenfd(port);
  while (1) {
    clientlen = sizeof(clientaddr);
    connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);

    // 
    // CS537: In general, don't handle the request in the main thread.
    // TODO In multi-threading mode, the main thread needs to
    // save the relevant info in a buffer and have one of the worker threads
    // do the work.
    // 
    requestHandle(connfd);

    Close(connfd);
  }

}






