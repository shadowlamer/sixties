#ifndef __GENERATOR_H
#define __GENERATOR_H

#define GEN_BASE_FREQ 50000
#define GEN_LO_FREQ_DIVIDER 4
#define GEN_BASE_RCR 2
#define GEN_BASE_PERIOD 64
#define GEN_NUM_FREQS 1
#define SIN_BUF_LENGTH 64

void generator_init();
void generator_start();
void generator_stop();
#endif
