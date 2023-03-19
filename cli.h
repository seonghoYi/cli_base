#ifndef _CLI_H_
#define _CLI_H_

#include "config_def.h"

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>


#ifdef _USE_MODULE_CLI

typedef struct
{
    int argc;
    char** argv;

} cliArgs_t;

bool cliInit(void);
bool cliDeinit(void);
bool cliOpen(void);
bool cliClose(void);

bool cliSpinOnce(void);




#endif

#endif
