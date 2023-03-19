#include "cli.h"
#include "term.h"

#ifdef _USE_MODULE_CLI

#define CLI_CMD_LEN_MAX         256
#define CLI_CMD_LIST_MAX        128
#define CLI_LINE_BUF_MAX        256
#define CLI_ARGS_LEN_MAX        128
#define CLI_HIST_LIST_MAX       128

/*          VT100          */
/*  ---------SP KEY 3 BYTES
* UP        0x1B 0x5B 0x41
* DOWN      0x1B 0x5B 0x42
* RIGHT     0x1B 0x5B 0x43
* LEFT      0x1B 0x5B 0x44
* F1        0x1B 0x4F 0x50
* F2        0x1B 0x4F 0x51
* F3        0x1B 0x4F 0x52
* F4        0x1B 0x4F 0x53
* HOME      0x1B 0x5B 0x48
* END       0x1B 0x5B 0x46
*/

/*  ---------SP KEY 4 BYTES
* INSERT    0x1B 0x5B 0x32 0x7E
* DELETE    0x1B 0x5B 0x33 0x7E
* PGUP      0x1B 0x5B 0x35 0x7E
* PGDW      0x1B 0x5B 0x36 0x7E
*/

/*  ---------SP KEY 5 BYTES
* F5        0x1B 0x5B 0x31 0x35 0x7E
* F6        0x1B 0x5B 0x31 0x37 0x7E
* F7        0x1B 0x5B 0x31 0x38 0x7E
* F8        0x1B 0x5B 0x31 0x39 0x7E
* F9        0x1B 0x5B 0x32 0x30 0x7E
* F10       0x1B 0x5B 0x32 0x31 0x7E
* F11       0x1B 0x5B 0x32 0x33 0x7E
* F12       0x1B 0x5B 0x32 0x34 0x7E
*/

#define CLI_KEY_ESC 0x1B
#define CLI_KEY_ESC2 '['
#define CLI_KEY_DEL 0x7F
#define CLI_KEY_ENTER '\n'
#define CLI_KEY_UP  0x41
#define CLI_KEY_DOWN 0x42
#define CLI_KEY_RIGHT 0x43
#define CLI_KEY_LEFT 0x44
#define CLI_KEY_HOME 0x48
#define CLI_KEY_END 0x46


enum
{
    CLI_STATE_NORMAL = 0U,
    CLI_STATE_SP1,
    CLI_STATE_SP2,
    CLI_STATE_SP3,
    CLI_STATE_SP4,
};


typedef struct
{
    int (* open)();
    int (* close)();
    int (* getch)();
    int (* putch)(int);
    int (* available)();

} cliDriver_t;

typedef struct 
{
    char cmdName[CLI_CMD_LEN_MAX];
    bool (*cmdFunc)(cliArgs_t*);
} cliCmd_t;


typedef struct
{
    char cmd_buffer[CLI_LINE_BUF_MAX];
    uint32_t buf_len;
    uint32_t count;
    uint32_t cursor;

} cliLine_t;


typedef struct
{
    cliDriver_t* p_driver;

    bool is_init;
    uint32_t state;
    uint32_t argc;
    char* argv[CLI_ARGS_LEN_MAX];

    cliLine_t cmdLine;

    uint32_t cmdHistIndex;
    uint32_t cmdHistLatest;
    cliLine_t cmdHistList[CLI_HIST_LIST_MAX];

    cliArgs_t cmdArgs;
    cliCmd_t cmdList[CLI_CMD_LIST_MAX];
    uint32_t cmdCount;
} cliNode_t;

static cliNode_t cli_node;
static cliDriver_t driver;

bool cliInstallDriver(cliDriver_t* p_driver);
bool cliParseArgs(void);
void cliAddLine(void);
void cliChangeLine(bool keyUp);

bool cliInit(void)
{
    bool ret = true;

    if (cli_node.is_init == true)
    {
        return ret;
    }

    memset(&cli_node, 0, sizeof(cli_node));

    cli_node.p_driver = &driver;
    cli_node.state = CLI_STATE_NORMAL;

    ret = cliInstallDriver(cli_node.p_driver);

    cli_node.cmdLine.buf_len = CLI_LINE_BUF_MAX;

    cli_node.is_init = 1;

    return ret;
}

bool cliDeinit(void)
{
    bool ret = true;

    cli_node.is_init = false;

    return ret;
}

bool cliInstallDriver(cliDriver_t* p_driver)
{
    bool ret = true;

    p_driver->open = termInit;
    p_driver->close = termDeinit;
    p_driver->getch = termGetch;
    p_driver->putch = termPutch;
    p_driver->available = termAvailable;

    return ret;
}

bool cliOpen(void)
{
    bool ret = true;

    if ((ret = cli_node.p_driver->open()) == true)
    {
        printf("term init failed\n");
    }

    return ret;
}

bool cliClose(void)
{
    bool ret = true;
    ret = cli_node.p_driver->close();

    return ret;
}

uint32_t cliWrite(char* p_data, unsigned int len)
{
    int i;

    for (i = 0; i < len; i++)
    {
        cli_node.p_driver->putch((int)p_data[i]);
    }

    return i;
}

uint32_t cliRead(void)
{
    int ch = EOF;

    ch = cli_node.p_driver->getch();

    return ch;
}

void cliPrintf(const char* fmt, ...)
{
    char buf[256];
    va_list arg;
    va_start(arg, fmt);
    int len = 0;

    len = vsnprintf(buf, 256, fmt, arg);
    va_end(arg);

    cliWrite(buf, len);
}

uint32_t cliAvailable(void)
{
    uint32_t ret;

    ret = cli_node.p_driver->available();

    return ret;
}

bool cliUpdate(char rx_data)
{
    bool ret = false;

    cliLine_t* line = &cli_node.cmdLine;

    if (cli_node.state == CLI_STATE_NORMAL)
    {
        switch (rx_data)
        {
        case CLI_KEY_ENTER:
            if (line->count > 0)
            {
                cliParseArgs();
                cliAddLine();
                ret = true;
            }

            cliPrintf("\n");


            line->cmd_buffer[line->count] = '\0';
            line->count = 0;
            line->cursor = 0;
            
            

            break;
        case CLI_KEY_DEL:
            if (line->count>0 && line->cursor>0)
            {
                if (line->count == line->cursor)
                {
                    line->count--;
                }
                else if (line->count > line->cursor)
                {
                    int mov_amount = 0;
                    mov_amount = line->count - line->cursor;


                    for (int i=0; i<mov_amount; i++)
                    {
                        line->cmd_buffer[line->cursor+i-1] = line->cmd_buffer[line->cursor+i];
                    }

                    line->count--;
                }

                if (line->cursor > 0)
                {
                    line->cursor--;
                    cliPrintf("\b\x1B[1P");
                }
            }
            
            break;
        case CLI_KEY_ESC:
            cli_node.state = CLI_STATE_SP1;
            break;
        default:
            if (line->count < CLI_LINE_BUF_MAX)
            {
                if (line->count == line->cursor)
                {
                    cliWrite((char* )&rx_data, 1);

                    line->cmd_buffer[line->count] = rx_data;
                    line->count++;
                    line->cursor++;
                }
                else if (line->count > line->cursor)
                {
                    int mov_amount = 0;

                    mov_amount = line->count - line->cursor;

                    for (int i=0; i<mov_amount; i++)
                    {
                        line->cmd_buffer[line->count - i] = line->cmd_buffer[line->count - i - 1];
                    }

                    line->cmd_buffer[line->cursor] = rx_data;
                    line->count++;
                    line->cursor++;
                    cliPrintf("\x1B[4h%c\x1B[4l", rx_data);
                }   
            }
            break;
        }

    }

    switch (cli_node.state)
    {
    case CLI_STATE_SP1:
        cli_node.state = CLI_STATE_SP2;
        break;
    case CLI_STATE_SP2:
        if (rx_data == CLI_KEY_ESC2)
        {
            cli_node.state = CLI_STATE_SP3;    
        }
        else
        {
            cli_node.state = CLI_STATE_NORMAL;
        }
        break;
    case CLI_STATE_SP3:
        if (rx_data == CLI_KEY_UP)
        {
            cliChangeLine(true);
        }
        else if (rx_data == CLI_KEY_DOWN)
        {
            cliChangeLine(false);
        }
        else if (rx_data == CLI_KEY_LEFT)
        {
            if (line->cursor > 0)
            {
                line->cursor--;
                cliPrintf("\x1B\x5B\x44");

            }
        }
        else if (rx_data == CLI_KEY_RIGHT)
        {
            if (line->cursor < line->count)
            {
                line->cursor++;
                cliPrintf("\x1B\x5B\x43");
            }
            
        }
        else if (rx_data == CLI_KEY_HOME)
        {
            if (line->cursor > 0)
            {
                cliPrintf("\x1B[%dD", line->cursor);
                line->cursor = 0;
            }
        }
        else if (rx_data == CLI_KEY_END)
        {
            if (line->count - line->cursor > 0)
            {
                cliPrintf("\x1B[%dC", line->count - line->cursor);
                line->cursor = line->count;
            }
        }
        else
        {
            cli_node.state = CLI_STATE_SP4;
            break;
        }
        cli_node.state = CLI_STATE_NORMAL;
        break;
    case CLI_STATE_SP4:
        cli_node.state = CLI_STATE_NORMAL;
        break;
    default:
        break;
    }

    return ret;
}

bool cliSpinOnce(void)
{
    bool ret = false;

    char rx_data;
    
    if (cliAvailable() > 0)
    {
        rx_data = cliRead();
        
        ret = cliUpdate(rx_data);
    }
    
    return ret;
}


bool cliParseArgs(void)
{
    bool ret = true;
    const char* delim = " \t";
    int argc = 0;
    char** argv = cli_node.argv;
    char* ret_ptr = NULL;
    char* next_ptr = NULL;

    argv[argc] = NULL;

    for (ret_ptr = __strtok_r(cli_node.cmdLine.cmd_buffer, delim, &next_ptr); ret_ptr; ret_ptr = __strtok_r(NULL, delim, &next_ptr))
    {
        argv[argc++] = ret_ptr;
    }

    cli_node.argc = argc;

    if (argc < 1)
    {
        ret = false;
    }

    return ret;
}

void cliAddLine(void)
{
    cli_node.cmdHistList[cli_node.cmdHistLatest] = cli_node.cmdLine;

    if (cli_node.cmdHistLatest < CLI_HIST_LIST_MAX)
    {
        cli_node.cmdHistLatest = (cli_node.cmdHistLatest + 1) % CLI_HIST_LIST_MAX;
    }

    cli_node.cmdHistIndex = cli_node.cmdHistLatest;
}

void cliChangeLine(bool keyUp)
{
    int index = 0;
    

    if (cli_node.cmdHistLatest == 0)
    {
        return;
    }

    if(keyUp == false)
    {
        if (cli_node.cmdHistIndex >= cli_node.cmdHistLatest)
        {
            cli_node.cmdHistIndex = cli_node.cmdHistLatest;
            return;
        }

        index = ++cli_node.cmdHistIndex;
    }
    else
    {
        if (cli_node.cmdHistIndex <= 0)
        {
            cli_node.cmdHistIndex = 0;
            return;
        }
        
        index = --cli_node.cmdHistIndex;
    }


    if (cli_node.cmdLine.cursor > 0)
    {
        cliPrintf("\x1B[%dD", cli_node.cmdLine.cursor);
    }

    if (cli_node.cmdLine.count > 0)
    {
        cliPrintf("\x1B[%dP", cli_node.cmdLine.count);
    }

    cliWrite(cli_node.cmdHistList[index].cmd_buffer, cli_node.cmdHistList[index].count);
    cli_node.cmdLine = cli_node.cmdHistList[index];
}


bool cliAddcommand(const char *cmdName, void (*p_func)(cliArgs_t *args))
{
    bool ret = true;

    if (cli_node.cmdCount >= CLI_CMD_LIST_MAX)
    {
        ret = false;
        return ret;
    }

    strcpy(cli_node.cmdList[cli_node.cmdCount].cmdName, cmdName);
    cli_node.cmdList[cli_node.cmdCount].cmdFunc = p_func;

    cli_node.cmdCount++;
    return ret;
}

bool cliRunCommand(void)
{
    bool ret = true;



    for (int i=0; i<cli_node.cmdCount; i++)
    {
        if (strcmp(cli_node.argv[0], cli_node.cmdList[i].cmdName) == 0)
        {
            cli_node.cmdArgs.argc = cli_node.argc;
            cli_node.cmdArgs.argv = cli_node.argv;

            cli_node.cmdList[i].cmdFunc(&cli_node.cmdArgs);
        }
    }


    return true;
}

#endif

