/*******************************************
 *
 *
 *
 * Descriptions:
 ******************************************/

static char *Id = "$ID";
#include "C010Comm.h"
#include <string.h>

#define MAX_TCPCHD 500

#define COMI_MSG_WRITETO 60
#define COMI_MSG_READTO 68

#define RETRYNUM 5

void       Wait();
void       Wrmsg_to();
void       Rdmsg_to();
void     HandleExit(int n);

int     ChdNum;
int     socket_id_old,socket_id_new;
int     return_flag=0;

msg_def MsgIn;
msg_def MsgOut;

struct  sockaddr_in local_addr;
int     Local_len= sizeof( local_addr);

char        gsSrvId[SRV_ID_LEN+1];
char        gsToSrvId[SRV_ID_LEN+1];
char        gsSrvSeq[SRV_SEQ_ID_LEN+1];
char        gsLogFile[LOG_NAME_LEN_MAX];
char        gsMsgFile[LOG_NAME_LEN_MAX];
T_SrvMsq    gatSrvMsq[SRV_MSQ_NUM_MAX];

Tbl_srv_inf_Def    tTblSrvInf;

void SaveMsg (long lPid);
int nCConnectSocket(unsigned short Port, char *Ip_addr);
int main(int argc, char *argv[])
{
      int         nRet;
      int         i,nPid,port,ppid;
      long                lUsageKey;

      setbuf(stdout, NULL);
      setbuf(stderr, NULL);

      if(argc < 5)
      {
        printf("Usage:%s srvid seq tosrvid port\n", argv[0]);
        exit(-1);
      }
      printf("argvi111[%s][%s][%s][%s][%s]",argv[0],argv[1],argv[2],argv[3],argv[4]);
      strcpy(gsSrvId, argv[1]);
      strcpy(gsSrvSeq, argv[2]);
      strcpy(gsToSrvId, argv[3]);
      port = atol(argv[4]);

      nRet = DbsConnect ();
      if (nRet)
      {
        printf("OLDCommP nRet[%d] LINE[%d]\n", nRet, __LINE__);
        exit(-2);
      }

      /*if (getenv(SRV_USAGE_KEY))
                lUsageKey=atoi (getenv(SRV_USAGE_KEY));
            else {
                DbsDisconnect ();
                exit(-3);
            }*/
    memset ((char *)&tTblSrvInf, 0x00, sizeof (tTblSrvInf));
    memcpy (tTblSrvInf.srv_id, gsSrvId, SRV_ID_LEN);
    nRet = DbsSRVINF (DBS_SELECT, &tTblSrvInf);
    if (nRet)
    {
        printf("OLDCommP nRet[%d] LINE[%d] srvif[%s] key[%ld]\n", nRet, __LINE__,gsSrvId,tTblSrvInf.usage_key);
        DbsDisconnect ();
        exit(-2);
    }
    memset(gsLogFile, 0, sizeof(gsLogFile));
    memset(gsMsgFile, 0, sizeof(gsMsgFile));
    sprintf (gsLogFile, "%s.%s.log", gsSrvId, gsSrvSeq);
    sprintf (gsMsgFile, "%s.%s.msg", gsSrvId, gsSrvSeq);

    memset ((char *)gatSrvMsq, 0, sizeof (gatSrvMsq));
    nRet = MsqInit (gsSrvId, gatSrvMsq);
      for( i=0; i<=20; i++)
      {
        HtLog (gsLogFile, HT_LOG_MODE_NORMAL, __FILE__,__LINE__, "srvid=%s, gatSrvMsq[%d] sSrvId=%s nMsqId=%d lMsqType=%08d ",                         gsSrvId,i,gatSrvMsq[i].sSrvId,gatSrvMsq[i].nMsqId,gatSrvMsq[i].lMsqType);
      }
    if (nRet)
    {
        HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "MsqInit error,%d", nRet);
        DbsDisconnect ();
        exit(-3);
    }

    /* set SIGTERM handling */
    if (sigset(SIGTERM, HandleExit) == SIG_ERR)
        HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "sigset SIGTERM error, %d.", errno);

    HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "FromP Begin to create socket");

      socket_id_old = Create_socket( port );

    HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__,
          "create socket OK!, socket_id_od = %d",
        socket_id_old);

      ppid=getpid();

       sigset(SIGCHLD, Wait);
       while(1)
       {
        HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__,
                "OLDCommP child process num:%d",ChdNum);

        while (ChdNum >= MAX_TCPCHD) sleep(1);

        HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__,
            "begin to accept");

        socket_id_new = accept(socket_id_old,
            (struct sockaddr *)&local_addr, &Local_len);
        if (socket_id_new <=0)
        {
            if (errno==EINTR)
            {
                HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__,"EINTR");
                continue;
            }
            HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__,
                    "accept error");
            sleep(2);
            continue;
        }

        HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__,
                "accepted the connect request; vc = %d", socket_id_new);

        nPid= fork();
        if (nPid < 0)
        {
            close(socket_id_new);
            HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "fork error");
                   continue;
        }
        if (!nPid)
        {
            nRet = DbsConnect ();
            if (nRet)
            {
                HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__,
                    "OLDCommP Connect DB nRet[%d]", nRet);
                exit(-2);
            }

            DoIt( socket_id_new );

            close(socket_id_new);

            DbsDisconnect ();

            HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "End DOIt");

            exit(0);
        }
        ChdNum++;
        close(socket_id_new);
       }

    DbsDisconnect ();

}

void Wait()
{
    int nPid, Status;
    while(TRUE)
    {
        nPid = waitpid((pid_t)(-1), &Status, WNOHANG|WUNTRACED);
        if (nPid > 0)
        {
            if (--ChdNum == 0);
        }
        else
            break;
    }
}

void DoIt(int socket_id)
{
      long         nPid;
      int          nMsgSource;
      int          nRet;
      int          nMsgLen, nRetVal, nMpRetLen ;
      char         sNetMsgBuf[BUF_SIZE];
      char         sTmpMsgBuf[BUF_SIZE];
      int          nLen;
      unsigned char     Buf_head[5];
      char    sMsgType[17];
      int          socket_id_other;
      unsigned short          port;
      int            nMsgRecvLen;
      nPid = getpid();

      HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__,
             "begin read msg from p, pid = %ld", nPid);

      nMsgLen=ReadSocket(socket_id,sNetMsgBuf,BUF_SIZE);
      if (nMsgLen <=0)
      {
        if (nMsgLen == E_SBREAK)
        {
          HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__,
                 "read socket call");
          close(socket_id);
          exit(1);
        }
        else
          exit(-1);
      }

      memset(sTmpMsgBuf, 0, BUF_SIZE);
      memcpy(sTmpMsgBuf, sNetMsgBuf, nMsgLen);


      HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "recv from %s len = %d", gsSrvId, nMsgLen);
      /*HtDebugString (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__,  sNetMsgBuf,nMsgLen);*/
      HtWriteLog (gsMsgFile, HT_LOG_MODE_NORMAL, __FILE__,__LINE__, sNetMsgBuf, nMsgLen);
      /*HtDebugString (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, sNetMsgBuf, nMsgLen);*/


      memset(&MsgOut,' ',sizeof(msg_def));
      memcpy(MsgOut.msg_buf, gsSrvId, SRV_ID_LEN);
      memcpy(MsgOut.msg_buf+SRV_ID_LEN, gsToSrvId, SRV_ID_LEN);
      memset(sMsgType,0,sizeof(sMsgType));
      sprintf(sMsgType,"%016ld",nPid);
      memcpy(MsgOut.msg_buf+SRV_ID_LEN*2, sMsgType, FLD_MSQ_TYPE_LEN);
      memcpy(MsgOut.msg_buf+SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN, sNetMsgBuf, nMsgLen);

      HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__,
             "send msg to %s,gatSrvMsq %s", gsToSrvId,gatSrvMsq);
      HtDebugString (gsLogFile, HT_LOG_MODE_NORMAL, __FILE__,__LINE__,  MsgOut.msg_buf, nMsgLen+SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN);
      // modify by hxb at Tue 14 Feb 2017 covert  message format 128 bitmap  to 64 bitmap
      Covert128To64(sNetMsgBuf,nMsgLen);
      nRet = MsqSnd (gsToSrvId, gatSrvMsq, 0, nMsgLen+SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN, MsgOut.msg_buf);
      if (nRet)
      {
        HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__,
               "MsqSnd ToSrvId[%s] error [%d][%d]",
               gsToSrvId, nRet, errno);
        exit(1);
      }
      HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__,
             "write msg to %s ok!", gsToSrvId);
      HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__,
             "begin receive msg from %s,gatSrvMsq[%s]", gsSrvId,gatSrvMsq);

      HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__,
             "write msg to p ok!");
      sleep(1);
      close(socket_id);
}

void   Wrmsg_to()
{
    HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__,
        "Write to message queue over time !");
    return_flag=1;
}

void   Rdmsg_to()
{
    HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__,
        "Read from message queue over time!");
    return_flag=1;
}

int Create_socket( unsigned Port)
{
    int     socket_id;
     int       RetryTimeSap = 2,nRetryFlag = 0;

       memset(&local_addr,0, sizeof(local_addr));
       local_addr.sin_port = htons(Port);
       local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = inet_addr("0.0.0.0");

     while ((socket_id = socket(AF_INET, SOCK_STREAM, 0)) < 0)
       {
        HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__,
                 "socket error time");
          if (nRetryFlag++ == RETRYNUM)
           {
               RetryTimeSap *= 2;
               nRetryFlag = 0;
            exit(1);
           }
           sleep(RetryTimeSap);
       }

    Setsokopt(socket_id);

       RetryTimeSap = 2;
       nRetryFlag = 0;
       while(bind(socket_id,(struct sockaddr *)&local_addr,
        sizeof(local_addr)) <0)
       {
        HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__,
                "Error time");
           if (nRetryFlag++ == RETRYNUM)
           {
               RetryTimeSap *= 2;
               nRetryFlag = 0;
           }
           sleep(RetryTimeSap);
       }

    listen(socket_id, 1024);

       return(socket_id);
}


int ReadSocket(int socket_id_new,char *buf ,int len)
{
    int num, nLen, I=0,iTimeout=60;
    unsigned char tmp_buf[5];
    char tmp_Str[2048];
    unsigned char logbuf[2048];
	char const_srv_id[30];

    memset(tmp_Str, 0, sizeof(tmp_Str));
    memset(tmp_buf, 0, sizeof(tmp_buf));
    memset(logbuf, 0, sizeof(logbuf));
	memset(const_srv_id, 0, sizeof(const_srv_id));

        if (getenv("COMM_P_TIME_OUT"))
        iTimeout=atoi (getenv("COMM_P_TIME_OUT"));
    alarm(iTimeout);
        memset(tmp_buf,0,sizeof(tmp_buf));
    num = read(socket_id_new, tmp_buf, 2);
    memcpy(logbuf, tmp_buf, 2);
    HtDebugString (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, logbuf, 2);

    HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "len = %d", num);
    HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "socket = %d, pid = %d", socket_id_new, getpid());
    if ( num < 0 )
    {
        if (errno == EINTR) return;
        return (E_SBREAK);
    }

    if ( num == 0 ) return (E_SBREAK);

    HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "tmp_buf0[%d], tmp_buf1[%d]", tmp_buf[0], tmp_buf[1]);
    nLen = tmp_buf[0]*256 + tmp_buf[1];
    HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "nLen = %d", nLen);
    if ( nLen == 0 ) return (E_SBREAK);

       while((num=read(socket_id_new, tmp_Str, nLen))<=0)
    {
          if(errno==EINTR) continue;
        HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "read socket error");
        return(E_SBREAK);
        }

    if (nLen >= 2048 )
    {
        HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__,"error nLen=%d >= 2048", nLen);
        return -1;
    }
    memcpy(buf, tmp_Str, nLen);
    memcpy(logbuf+2, tmp_Str, nLen);
    HtDebugString (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, logbuf, 2+nLen);
	memcpy(const_srv_id,gsSrvId,4);
	strcat(const_srv_id,"Recvpacket");
	Print8583Packet(const_srv_id,logbuf, 2+nLen);

    return num ;
}

int WriteSocket(int socket_id_new,char *buf, int len)
{
    int  num, iWritelen;
    char Buf_head[5];
    char saSendBuf[BUF_SIZE];
    char sTmp[4+1];
    char const_srv_id[30];

      if (len == 0) return(0);

       memset(saSendBuf,0,sizeof(saSendBuf));
    memset(sTmp,0,sizeof(sTmp));
	memset(const_srv_id, 0, sizeof(const_srv_id));

    sprintf(sTmp,"%04x", len);
    Str2Hex(sTmp,saSendBuf,4);
    memcpy(saSendBuf+2,buf,len);

    len = len + 2;
    HtDebugString (gsLogFile, HT_LOG_MODE_DEBUG, __FILE__,    __LINE__, saSendBuf, len);
    memcpy(const_srv_id,gsSrvId,4);
	strcat(const_srv_id,"Sendpacket");
    Print8583Packet(const_srv_id,saSendBuf, len);

HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__,"socket = %d, pid = %d", socket_id_new, getpid());
    iWritelen=0;
    for(;;)
    {
          while((num=write(socket_id_new,&saSendBuf[iWritelen],
                len-iWritelen))<=0)
           {
            if (errno == EINTR) continue;
            HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__,
                "write socket");
            return(E_SBREAK);
           }

           iWritelen+=num;
        if(iWritelen>=len) break;
    }
    return(iWritelen);
}

int Setsokopt(int sdNew)
{
    struct linger soLinger;
    int soKeepAlive,soReuseAddr;

       /*************************************************/
    /* Set socket options for new socket              */
    /*************************************************/
    soLinger.l_onoff = 1;
    soLinger.l_linger = 0;
    if( -1 == setsockopt(sdNew,
                     SOL_SOCKET,
                     SO_LINGER,
                     (char*)&soLinger,
                     sizeof(soLinger)))
    {
        HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__,
             "set linger option failed!");
        return -1;
    }

    soKeepAlive = 0;
#if 1
    if( -1 == setsockopt(sdNew,
                     SOL_SOCKET,
                     SO_KEEPALIVE,
                     &soKeepAlive,
                     (int)sizeof(soKeepAlive)))
    {
        HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__,
            "set keepalive option failed!");
        return -1;
    }
#endif
    soReuseAddr = 1;
    if( -1 == setsockopt(sdNew,
                     SOL_SOCKET,
                     SO_REUSEADDR,
                     &soReuseAddr,
                     (int)sizeof(soReuseAddr)))
    {
        HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__,
            "set reuse addr option failed!");
        return -1;
    }
    return 0;
}

void SaveMsg (long lPid)
{
    int                    nReturnCode;
    Tbl_dumpmsg_Def        tTblDumpmsg;

    memset (&tTblDumpmsg, 0, sizeof (tTblDumpmsg));
    memcpy (tTblDumpmsg.srv_id, gsSrvId, SRV_ID_LEN);
    sprintf (tTblDumpmsg.msq_type, "%016d", lPid);
    CommonGetCurrentTime (tTblDumpmsg.creation_time);

    nReturnCode = DbsDumpmsg (DBS_INSERT, &tTblDumpmsg);
    if (nReturnCode)
        HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "DbsDumpmsg insert err, %d", nReturnCode);

}

void HandleExit (int n)
{
    DbsDisconnect ();
    HtLog( gsLogFile, HT_LOG_MODE_NORMAL, __FILE__,__LINE__, "OLDCommP exits.");
    exit( 1 );
}
int nCConnectSocket(unsigned short Port, char *Ip_addr)
{
    struct sockaddr_in   Sin;
    int                  nConeCount;
    int                  Socket_id, /*RetryTimeSap = 2*/ RetryTimeSap = 1, nRetryFlag = 0;
    unsigned long ul = 1;
    struct  timeval tm;
    fd_set set;
    int nFlag=-1,nLen,nError=-1;
    nLen = sizeof(int);

    HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "Ip_addr, %s and port=%d", Ip_addr,Port);
    memset(&Sin, 0, sizeof(Sin));
    Sin.sin_port = htons(Port);
    Sin.sin_family = AF_INET;
    Sin.sin_addr.s_addr = inet_addr(Ip_addr);

    /*20140404 ÐÞ¸ÄÒòÎªÎ´³õÊ¼»¯Ôì³ÉµÄÑ­»·connect*/
    nConeCount=0;

    while(1)
		{
			while ((Socket_id = socket(AF_INET, SOCK_STREAM, 0)) < 0)
				{
					HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "socket, %d", errno);
					nRetryFlag++;
					if (nRetryFlag >= RETRYNUM)
						{
							HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "get socket_id error[%d]", errno);
							return -1;
						}

					sleep(RetryTimeSap);
				}

			ioctl(Socket_id, FIONBIO, &ul); //ÉèÖÃÎª·Ç×èÈûÄ£Ê½

			if ( connect(Socket_id, (struct sockaddr *)&Sin, sizeof(Sin)) < 0 )
				{
					tm.tv_sec = CONNECT_TIME_OUT;
					tm.tv_usec = 0;
					FD_ZERO(&set);
					FD_SET(Socket_id, &set);
					HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "connect:select check time_out[%d].[%d]",tm.tv_sec,errno);
					if( select(Socket_id+1, NULL, &set, NULL, &tm) > 0 )
						{
							getsockopt(Socket_id, SOL_SOCKET, SO_ERROR, &nError, (socklen_t *)&nLen);
							if( nError == 0 )
								nFlag = 0;
							else
								nFlag = -1;
					}else
						{
							HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "connect:select check error[%d]",errno);
							nFlag = -1;
						}
			}else
                nFlag=0;
			ul = 0;
			ioctl(Socket_id, FIONBIO, &ul); //ÉèÖÃÎª×èÈûÄ£Ê½

			if( nFlag )
				{
					nConeCount++;
					HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "connect error[%s][%d], %d",Ip_addr , Port, errno);
					close(Socket_id);
#if 0
					if ( errno == ECONNREFUSED )
						{
							if (nConeCount <= 3)
								continue;
						}
					if (nConeCount <= 2)
						continue;
#endif
					return -1;
				}
			break;
		}

    return(Socket_id);
}
