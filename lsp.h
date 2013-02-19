#pragma once


#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdbool.h>
#include <pthread.h>
#include <errno.h>
#include <strings.h>
#include "udp.h"

// Global Parameters. For both server and clients.

#define _EPOCH_LTH 2.0
#define _EPOCH_CNT 5;
#define _DROP_RATE 0.0;

void lsp_set_epoch_lth(double lth);
void lsp_set_epoch_cnt(int cnt);
void lsp_set_drop_rate(double rate);
