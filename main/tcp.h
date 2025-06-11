// tcp_client.h

#ifndef MAIN_TCP_CLIENT_H_
#define MAIN_TCP_CLIENT_H_

#define TCP_CLIENT_TASK_STACK_SIZE 4096
#define TCP_CLIENT_TASK_PRIORITY   5
#define TCP_CLIENT_TASK_CORE_ID    tskNO_AFFINITY

void start_tcp_client_task(void);

#endif /* MAIN_TCP_CLIENT_H_ */
