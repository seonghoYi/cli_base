#include "cli.h"
#include "term.h"

#ifdef _USE_MODULE_CLI

#define CLI_LINE_BUF_MAX    256


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
* F11       
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


typedef struct _consoleDriver
{
    int (* open)();
    int (* close)();
    int (* getch)();
    int (* putch)(int);
    int (* available)();

} consoleDriver_t;


typedef struct _cmdLine
{
    char cmd_buffer[CLI_LINE_BUF_MAX];
    unsigned int buf_len;
    unsigned int cursor;

} cmdLine_t;


typedef struct _cli
{
    cmdLine_t cmdLine;
    consoleDriver_t* p_driver;

    int state;

} cli_t;


static cli_t cli_node;
static consoleDriver_t driver;

int cliInstallDriver(consoleDriver_t* p_driver);

int cliInit(void)
{
    int ret = 0;

    cli_node.p_driver = &driver;
    cli_node.state = CLI_STATE_NORMAL;

    ret = cliInstallDriver(cli_node.p_driver);

    return ret;
}

int cliDeinit(void)
{
    int ret = 0;

    return ret;
}

int cliInstallDriver(consoleDriver_t* p_driver)
{
    int ret = 0;

    p_driver->open = termInit;
    p_driver->close = termDeinit;
    p_driver->getch = termGetch;
    p_driver->putch = termPutch;
    p_driver->available = termAvailable;

    return ret;
}

int cliOpen(void)
{
    int ret = 0;

    if ((ret = cli_node.p_driver->open()) != 0)
    {
        printf("term init failed\n");
    }

    return ret;
}

int cliClose(void)
{
    int ret = 0;
    ret = cli_node.p_driver->close();

    return ret;
}

int cliWrite(unsigned char* p_data, unsigned int len)
{
    int i;

    for (i = 0; i < len; i++)
    {
        cli_node.p_driver->putch((int)p_data[i]);
    }

    return i;
}

int cliRead(void)
{
    int ch = EOF;

    ch = cli_node.p_driver->getch();

    return ch;
}

int cliAvailable(void)
{
    int ret;

    ret = cli_node.p_driver->available();

    return ret;
}

int cliUpdate(char rx_data)
{
    int ret = 0;

    if (cli_node.state == CLI_STATE_NORMAL)
    {
        printf("%02X\n", rx_data);
        switch (rx_data)
        {
        case CLI_KEY_ENTER:
            printf("enter\n");
            break;
        case CLI_KEY_DEL:
            printf("backspace\n");
            break;
        case CLI_KEY_ESC:
            cli_node.state = CLI_STATE_SP1;
            break;
        default:
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
            printf("up\n");
        }
        else if (rx_data == CLI_KEY_DOWN)
        {
            printf("down\n");
        }
        else if (rx_data == CLI_KEY_LEFT)
        {
            printf("left\n");
        }
        else if (rx_data == CLI_KEY_RIGHT)
        {
            printf("right\n");
        }
        else if (rx_data == CLI_KEY_HOME)
        {
            printf("HOME\n");
        }
        else if (rx_data == CLI_KEY_END)
        {
            printf("END\n");
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

int cliSpinOnce(void)
{
    int ret = 1;

    char rx_data;
    
    if (cliAvailable() > 0)
    {
        rx_data = cliRead();
        
        ret = cliUpdate(rx_data);
    }
    
    return ret;
}


#endif

