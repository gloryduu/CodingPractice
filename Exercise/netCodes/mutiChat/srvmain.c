#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>     //to use basename()
#include <poll.h>
#include <unistd.h>     //to use syscall, e.g.:close
#include <errno.h>
#include <assert.h>

#include <sys/types.h>    //to use listen(Linux no require, but portable applications are probably wise to include it)
#include <sys/socket.h>
#include <arpa/inet.h>

#define BOOL_FALSE      0
#define BOOL_TRUE       1

#define RET_SUCCESS                  0
#define RET_ERR_GENERAL	            -1
#define RET_ERR_PARAM               -2
#define RET_ERR_MEM_NOTENOUGH       -3

#define FD_INVALID          -1
#define LISTEN_QUE_SIZE     5
#define POLL_EVENT_SIZE     5

#define USR_FDSIZE          94024   //cat /proc/sys/fs/
#define USR_BUF_SIZE        64
#define MSG_HEAD_SIZE       32
#define SND_MSGBLOCK_SIZE   2

typedef struct tagUSR_CONRCD
{
	struct sockaddr_in stAddrUsr;
	char szBuf[USR_BUF_SIZE];
	char *pcWrtBuf;
} USR_CONRCD_S;

int main(int argc, char *argv[])
{
    int iRet = 0;
    int iSrvSock = FD_INVALID;
    int iUsrSock = FD_INVALID;
    int iSockOpt = 0;
    int iPlRet = 0;
    struct sockaddr_in stAddrSrv;
    struct pollfd astPlFd[POLL_EVENT_SIZE + 1];
    USR_CONRCD_S *pstUsrConRcd;
    unsigned int uiUsrCnt = 0;

    /* cheak parameters */
    if (argc < 3)
    {
        printf("Usage: %s serverIp listenPort\n", basename(argv[0]));
        return RET_ERR_PARAM;
    }

    /* creat a socket */
    iSrvSock = socket(AF_INET, SOCK_STREAM, 0);
    if (FD_INVALID == iSrvSock)
    {
        perror("socket() failed");
        return RET_ERR_GENERAL;
    }
    printf("socket() success, new socket fd: %d\n", iSrvSock);

    iSockOpt = 1;
    if (setsockopt(iSrvSock, SOL_SOCKET, SO_REUSEADDR, &iSockOpt, sizeof(iSockOpt)) < 0)
    {
        perror("setsockopt() failed");
    }

    /* bind server addr to socket */
    memset(&stAddrSrv, 0, sizeof(stAddrSrv));
    stAddrSrv.sin_family = AF_INET;
    if (inet_pton(AF_INET, argv[1], &stAddrSrv.sin_addr) != 1)
    {
        perror("inet_pton() failed");
        close(iSrvSock);
        return RET_ERR_GENERAL;
    }
    stAddrSrv.sin_port = htons(atoi(argv[2]));
    if (bind(iSrvSock, (struct sockaddr*)&stAddrSrv, sizeof(stAddrSrv)) < 0)
    {
        perror("bind() failed");
        close(iSrvSock);
        return RET_ERR_GENERAL;
    }
    printf("bind() success\n");

    /* listen... */
    if (listen(iSrvSock, LISTEN_QUE_SIZE) < 0)
    {
        perror("listen() failed");
        close(iSrvSock);
        return RET_ERR_GENERAL;
    }
    printf("listen() success\n");

    pstUsrConRcd = malloc(sizeof(USR_CONRCD_S) * USR_FDSIZE);
    if (NULL == pstUsrConRcd)
    {
        perror("malloc() failed");
        close(iSrvSock);
        return RET_ERR_MEM_NOTENOUGH;
    }
    memset(pstUsrConRcd, 0, sizeof(USR_CONRCD_S) * USR_FDSIZE);

    /* init poll event */
    astPlFd[0].fd = iSrvSock;
    astPlFd[0].events = POLLIN;
    for (int idx = 1; idx <= POLL_EVENT_SIZE; idx++)
    {
        astPlFd[idx].fd = -1;
        astPlFd[idx].events = 0;
    }

    for (;;)
    {
        printf("poll() blocking...\n");
        iPlRet = poll(astPlFd, uiUsrCnt + 1, -1);
        printf("poll() return, iPlRet = %d, cur poll event que size %u\n", iPlRet, uiUsrCnt + 1);
        for (int idx = 0; idx <= POLL_EVENT_SIZE; idx++)
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

        /* listenFd */
        if (astPlFd[0].revents & POLLIN)
        {
            /* new connect */
            struct sockaddr_in stAddrUsr;
            socklen_t sockAddrLen = sizeof(stAddrUsr);
            memset(&stAddrUsr, 0, sockAddrLen);
            int iConSock = accept(iSrvSock, (struct sockaddr*)&stAddrUsr, &sockAddrLen);
            if (FD_INVALID == iConSock)
            {
                perror("accept() failed");
                continue;
            }

            if (uiUsrCnt < POLL_EVENT_SIZE)
            {
                uiUsrCnt++;
                astPlFd[uiUsrCnt].fd = iConSock;
                astPlFd[uiUsrCnt].events = POLLIN;
                astPlFd[uiUsrCnt].revents = 0;      //clean old usr ready event
                pstUsrConRcd[iConSock].stAddrUsr = stAddrUsr;
                printf("add new usr con %d to poll, cur usr num %u\n", iConSock, uiUsrCnt);
            }
            else
            {
                char *pInfoTmp = "cur usr num up to limit, closing...";
                send(iConSock, pInfoTmp, strlen(pInfoTmp), 0);
                printf("cur usr num up to limit %u, new con %d closing...\n", uiUsrCnt, iConSock);
                close(iConSock);
            }
            iPlRet--;
        }
        else if (astPlFd[0].revents)
        {
            printf("listenFd failed, revents %d", astPlFd[0].revents);
            iPlRet--;
        }

        /* no ready usr fd */
        if (iPlRet <= 0)
        {
            printf("no ready usr fd\n");
            continue;
        }

        for (int idx = 1; idx <= uiUsrCnt; idx++)
        {
            int iIsToClose = BOOL_FALSE;
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
                memset(pstUsrConRcd[iCurUsrFd].szBuf, 0, USR_BUF_SIZE);
                int iRecvMsgLen = recv(iCurUsrFd, pstUsrConRcd[iCurUsrFd].szBuf, USR_BUF_SIZE, 0);
                printf("after recv, errno = %d\n", errno);

                /* no block error, just triggered when set fd or read/write noblock */
                if ((0 == iRecvMsgLen) && (errno & (EAGAIN | EWOULDBLOCK)))
                {
                    printf("0 == iRecvMsgLen && EAGAIN | EWOULDBLOCK\n");
                }
                /* read error */
                else if (iRecvMsgLen <= 0)
                {
                    /* peer close */
                    if (0 == iRecvMsgLen)
                    {
                        printf("peer close, usr %d closing...\n", iCurUsrFd);
                    }
                    /* other error */
                    else
                    {
                        perror("recv error");
                    }
                    iIsToClose = BOOL_TRUE;
                }
                /* read success */
                else
                {
                    unsigned int uiMsgLen = strlen(pstUsrConRcd[iCurUsrFd].szBuf);
                    printf("read from usr %d msg len %d, msg: %s\n",
                            iCurUsrFd, uiMsgLen, pstUsrConRcd[iCurUsrFd].szBuf);
                    for (int idx1 = 1; idx1 <= uiUsrCnt; idx1++)
                    {
                        if (astPlFd[idx1].fd != iCurUsrFd)
                        {
                            /* change to write */
                            astPlFd[idx1].events &= ~POLLIN;
                            astPlFd[idx1].events |= POLLOUT;
                            pstUsrConRcd[astPlFd[idx1].fd].pcWrtBuf = pstUsrConRcd[iCurUsrFd].szBuf;
                        }
                    }
                    iUsrSock = iCurUsrFd;
                }
                iPlRet--;
            }
            /* write ready */
            else if (astPlFd[idx].revents & POLLOUT)
            {
                if (NULL != pstUsrConRcd[iCurUsrFd].pcWrtBuf)
                {
                    char szMsgHead[MSG_HEAD_SIZE];
                    struct iovec astIovec[SND_MSGBLOCK_SIZE];
                    unsigned int uiMsgHeadLen = 0;
                    unsigned int uiMsgDataLen = 0;

                    szMsgHead[0] = '\0';
                    memset(astIovec, 0, sizeof(astIovec));
                    (void)snprintf(szMsgHead, MSG_HEAD_SIZE, "Msg from usr %d:", iUsrSock);
                    uiMsgHeadLen = strlen(szMsgHead);
                    uiMsgDataLen = strlen(pstUsrConRcd[iCurUsrFd].pcWrtBuf);
                    astIovec[0].iov_base = szMsgHead;
                    astIovec[0].iov_len = uiMsgHeadLen;
                    astIovec[1].iov_base = pstUsrConRcd[iCurUsrFd].pcWrtBuf;
                    astIovec[1].iov_len = uiMsgDataLen;
                    if (writev(iCurUsrFd, astIovec, SND_MSGBLOCK_SIZE) != uiMsgHeadLen + uiMsgDataLen)
                    {
                        perror("writev() != uiMsgHeadLen + uiMsgDataLen");
                    }
                    printf("write to usr %d msg len %d: %s%s\n",
                            iCurUsrFd, uiMsgHeadLen + uiMsgDataLen, szMsgHead, pstUsrConRcd[iCurUsrFd].pcWrtBuf);
                    pstUsrConRcd[iCurUsrFd].pcWrtBuf = NULL;
                }

                /* change to read */
                astPlFd[idx].events &= ~POLLOUT;
                astPlFd[idx].events |= POLLIN;
                iPlRet--;
            }
            /* disconnect */
            else if (astPlFd[idx].revents & (POLLHUP|POLLERR))
            {
                iIsToClose = BOOL_TRUE;
                printf("usr %d disconnect, event POLLHUP|POLLERR\n", iCurUsrFd);
                iPlRet--;
            }
            else
            {
                iPlRet--;
                assert(0);
            }

            if (BOOL_TRUE == iIsToClose)
            {
                /* update cur pos to last usr con */
                if (idx < uiUsrCnt)
                {
                    astPlFd[idx]= astPlFd[uiUsrCnt];
                }

                /* close cur usr con and reset the last usr con */
                close(iCurUsrFd);
                astPlFd[uiUsrCnt].fd = -1;
                astPlFd[uiUsrCnt].events = 0;
                astPlFd[uiUsrCnt].revents = 0;

                /* maybe cur pos need do onece again */
                idx--;
                uiUsrCnt--;
            }

            /* no more ready usr fd */
            if (iPlRet <= 0)
            {
                printf("no more ready usr fd\n");
                break;
            }
        }
    }

    close(iSrvSock);
    free(pstUsrConRcd);
    printf("exit\n");
    return 0;
}


