//#pragma once
#ifndef _PIPE_H_
#define _PIPE_H_
#include <stdint.h>
#define BUFSIZE 4096  

int simc_fifo_init();
int simc_fifo_proc(uint32_t pc);
int simc_fifo_queue_proc(uint32_t pc);

#endif
