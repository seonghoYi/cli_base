#include "term.h"

#include <errno.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <fcntl.h>

static struct termios oldattr, newattr;

int termInit(void)
{
    int ret = 0;
    ret = tcgetattr(STDIN_FILENO, &oldattr);            // 현재 터미널 설정 읽음
    newattr = oldattr;
    newattr.c_lflag &= ~(ICANON | ECHO);                // CANONICAL과 ECHO 끔
    newattr.c_cc[VMIN] = 0;                             // 최소 입력 문자 수를 0로 설정
    newattr.c_cc[VTIME] = 0;                            // 최소 읽기 대기 시간을 0으로 설정
    ret = tcsetattr(STDIN_FILENO, TCSANOW, &newattr);   // 터미널에 설정 입력
    return ret;
}

int termDeinit(void)
{
    int ret = 0;

    oldattr.c_lflag |= (ICANON | ECHO);
    ret = tcsetattr(STDIN_FILENO, TCSANOW, &oldattr);  // 원래의 설정으로 복구
    return ret;
}

int termGetch()
{
    int c = getchar();

    return c;
}

int termPutch(int ch)
{
    int ret = putchar(ch);
    fflush(stdout);
    return ret;
}

int termAvailable(void)
{
    int ret_num = 0;

    int ch;

    ret_num = read(STDIN_FILENO, &ch, 1);

    if (ret_num > 0)
    {
        ungetc(ch, stdin);
    }


    return ret_num;
}