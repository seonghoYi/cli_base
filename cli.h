#ifndef _CLI_H_
#define _CLI_H_

#include "config_def.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#ifdef _USE_MODULE_CLI


int cliInit(void);
int cliDeinit(void);
int cliOpen(void);
int cliClose(void);

int cliSpinOnce(void);




#endif

#endif
