#ifndef NODECOMM_H
#define NODECOMM_H

#include "main.h"

void send_msg(int *msg, int to, int tag);
void receive_msg(int *holder, int from, int tag);

#endif // NODECOMM_H