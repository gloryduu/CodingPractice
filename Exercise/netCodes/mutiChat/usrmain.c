#define _GNU_SOURCE     //to use <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>     //to use basename()
#include <poll.h>
#include <unistd.h>     //to use syscall, e.g.:close
#include <errno.h>
#include <assert.h>

#include <fcntl.h>      //to use splice()

#include <sys/types.h>    //to use listen(Linux no require, but portable applications are probably wise to include it)
#include <sys/socket.h>
#include <arpa/inet.h>

#define BOOL_FALSE      0
#define BOOL_TRUE       1

#define RET_SUCCESS                  0
#define RET_ERR_GENERAL	            -1
#define RET_ERR_PARAM               -2

#define FD_INVALID          -1
#define FD_STDIN            0
#define FD_STDOUT           1
#define POLL_EVENT_SIZE     2

#define USR_BUF_SIZE        64

int main(int argc, char *argv[])
{
    int iRet = 0;
    int iUsrSock = FD_INVALID;
    int iSockOpt = 0;
    int iPlRet = 0;
    socklen_t sockAddrLen = 0;
    struct sockaddr_in stAddrSrv;
    struct pollfd astPlFd[POLL_EVENT_SIZE];
    int aiPipeFd[2];
    unsigned int uiUsrCnt = 0;
    ssize_t ssRcvMsgLen = 0;
    ssize_t ssSndMsgLen = 0;
    int iToCloseUsr = BOOL_FALSE;

    /* cheak parameters */
    if (argc < 3)
    {
        printf("Usage: %s serverIp listenPort\n", basename(argv[0]));
        return RET_ERR_PARAM;
    }

    /* creat a socket */
    iUsrSock = socket(AF_INET, SOCK_STREAM, 0);
    if (FD_INVALID == iUsrSock)
    {
        perror("socket() failed");
        return RET_ERR_GENERAL;
    }
    printf("socket() success, new socket fd: %d\n", iUsrSock);

    /* connect to server */
    memset(&stAddrSrv, 0, sizeof(stAddrSrv));
    stAddrSrv.sin_family = AF_INET;
    if (inet_pton(AF_INET, argv[1], &stAddrSrv.sin_addr) != 1)
    {
        perror("inet_pton() failed");
        close(iUsrSock);
        return RET_ERR_GENERAL;
    }
    stAddrSrv.sin_port = htons(atoi(argv[2]));
    sockAddrLen = sizeof(stAddrSrv);
    if (connect(iUsrSock, (struct sockaddr*)&stAddrSrv, sockAddrLen) < 0)
    {
        perror("connect() failed");
        close(iUsrSock);
        return RET_ERR_GENERAL;
    }
    printf("connect() success\n");

    if (pipe(aiPipeFd) < 0)
    {
        perror("pipe() failed");
        close(iUsrSock);
        return RET_ERR_GENERAL;
    }

    /* init poll event */
    astPlFd[uiUsrCnt].fd = FD_STDIN;
    astPlFd[uiUsrCnt].events = POLLIN;
    uiUsrCnt++;
    astPlFd[uiUsrCnt].fd = iUsrSock;
    astPlFd[uiUsrCnt].events = POLLIN;
    uiUsrCnt++;

    for (;;)
    {
        printf("poll() blocking...\n");
        iPlRet = poll(astPlFd, uiUsrCnt, -1);
        printf("poll() return, iPlRet = %d, cur poll event que size %u\n", iPlRet, uiUsrCnt);
        for (int idx = 0; idx < POLL_EVENT_SIZE; idx++)
        {
            printf("astPlFd[%d].fd = %d\n"
                   "astPlFd[].events = %d\n"
                   "astPlFd[].revents = %d\n",
                   idx, astPlFd[idx].fd, astPlFd[idx].events, astPlFd[idx].revents);
        }

        if (iPlRet < 0)
        {
            /* signal interrupt, just usefull to daemon */
            if (EINTR == errno)
            {
                perror("signal interrupt");
                continue;
            }
            /* other error */
            else
            {
                /* TODO: exit clean */
                perror("unknow error");
                break;
            }
        }

        /* infinit time-out mode, it is inevitable to happen: iPlRet != 0 */

        for (int idx = 0; idx < uiUsrCnt; idx++)
        {
            int iCurUsrFd = astPlFd[idx].fd;
            printf("cur usr con fd %u, revents %d\n", iCurUsrFd, astPlFd[idx].revents);

            if (!astPlFd[idx].revents)
            {
                printf("nothing happy\n");
                continue;
            }

            /* read ready */
            if (astPlFd[idx].revents & POLLIN)
            {
                printf("usr %d read ready, event POLLIN\n", iCurUsrFd);
                ssRcvMsgLen = splice(iCurUsrFd, NULL, aiPipeFd[1], NULL, USR_BUF_SIZE, 0);
                printf("read %u bytes from usr %d\n", ssRcvMsgLen, iCurUsrFd);
                if (FD_STDIN == iCurUsrFd)
                {
                    if (0 == ssRcvMsgLen)
                    {
                        printf("nothing input\n");
                        continue;
                    }
                    else
                    {
                        ssSndMsgLen = splice(aiPipeFd[0], NULL, iUsrSock, NULL, USR_BUF_SIZE, 0);
                        printf("write %u bytes to usr %d\n", ssSndMsgLen, iUsrSock);
                    }
                }
                else if (iUsrSock == iCurUsrFd)
                {
                    if (0 == ssRcvMsgLen)
                    {
                        printf("peer close, usr %d closing...\n", iUsrSock);
                        iToCloseUsr = BOOL_TRUE;
                        break;
                    }
                    ssSndMsgLen = splice(aiPipeFd[0], NULL, FD_STDOUT, NULL, USR_BUF_SIZE, 0);
                    printf("write %u bytes to usr %d\n", ssSndMsgLen, FD_STDOUT);
                }
                else
                {
                    assert(0);
                }
                iPlRet--;
            }
            /* write ready */
            else if (astPlFd[idx].revents & POLLOUT)
            {
                perror("catch a out event");
                iPlRet--;
            }
            /* disconnect */
            else if (astPlFd[idx].revents & (POLLHUP|POLLERR))
            {
                printf("usr %d disconnect, event POLLHUP|POLLERR\n", iCurUsrFd);
                if (iUsrSock == iCurUsrFd)
                {
                    printf("peer close, usr %d closing...\n", iUsrSock);
                    iToCloseUsr = BOOL_TRUE;
                    break;
                }
                iPlRet--;
            }
            else
            {
                iPlRet--;
                assert(0);
            }

            /* no more ready usr fd */
            if (iPlRet <= 0)
            {
                printf("no more ready usr fd\n");
                break;
            }
        }

        if (BOOL_TRUE == iToCloseUsr)
        {
            close(iUsrSock);
            iUsrSock = FD_INVALID;
            iToCloseUsr = BOOL_FALSE;
            break;
        }
    }

    if (FD_INVALID != iUsrSock)
    {
        close(iUsrSock);
    }

    printf("exit\n");
    return 0;
}


