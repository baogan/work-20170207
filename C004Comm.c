/*********************************************************************************
 *              TOPLINK+ -- Shanghai Huateng Software System Inc.
 *********************************************************************************
 * Date: 2012-03-05
 * Modified by: wgj
 * Descriptions: short link to the bank of china
 ********************************************************************************/
static char *Id = "$ID";

#include "C004Comm.h"


#define C_TCP_MSG_LEN_IN_BIN(x)         (x == 'B')
#define C_TCP_MSG_LEN_IN_BCD(x)         (x == 'C')

void    Wait();
void    Rdmsg_to();
void    SaveMsg (long lPid);
void    HandleExit(int n);

int     ChdNum;
int     port;
int     gnBackupHostPort;
int     return_flag=0;

char    gsSrvId[SRV_ID_LEN+1];
char    gsToSrvId[SRV_ID_LEN+1];
char    gsSrvSeq[SRV_SEQ_ID_LEN+1];
char    gsLogFile[LOG_NAME_LEN_MAX];
char    gsMsgFile[LOG_NAME_LEN_MAX];
char    IPdress[20+1];
char    gsBackupHostIp[20+1];
char    gsTcpMsgLenType;
int     gnTcpMsgLenL;


T_SrvMsq           gatSrvMsq[SRV_MSQ_NUM_MAX];
Tbl_srv_inf_Def    tTblSrvInf;
struct  sockaddr_in local_addr;
int     Local_len= sizeof( local_addr);

int nCConnectSocket(unsigned short Port, char *Ip_addr);

int main(int argc, char *argv[])
{
    int     nRet;
    int     nPid,ppid;
    int     nMsgLen;
    int     nReturnCode;
    long    lUsageKey;
    char    sMsgBuf[4096];
    int     i,ret;
    char    *lspTmp;

    if (argc < 5)
    {
        printf("Usage:%s srvid seq tosrvid port\n", argv[0]);
        exit(-1);
    }
    strcpy(gsSrvId, argv[1]);
    strcpy(gsSrvSeq, argv[2]);
    strcpy(gsToSrvId, argv[3]);
    strcpy(IPdress, argv[4]);

    if (getenv(SRV_USAGE_KEY))
        lUsageKey=atoi (getenv(SRV_USAGE_KEY));
    else
        return -1;

    /* »ñÈ¡±¸·ÝÖ÷»úIP */
    memset(gsBackupHostIp, 0, sizeof(gsBackupHostIp));
    if (getenv("HOST_SRV_IP"))
        strcpy(gsBackupHostIp,getenv("HOST_SRV_IP"));

    gnBackupHostPort = 0;
    if (getenv("HOST_SRV_PORT"))
        gnBackupHostPort = atoi(getenv("HOST_SRV_PORT"));

    ppid = getpid();
    port = atol(argv[5]);

    nRet = DbsConnect ();
    if (nRet)
    {
        printf("CommP nRet[%d] LINE[%d]\n", nRet, __LINE__);
        exit(-2);
    }

    memset ((char *)&tTblSrvInf, 0x00, sizeof (tTblSrvInf));
    memcpy (tTblSrvInf.srv_id, gsSrvId, SRV_ID_LEN);
    tTblSrvInf.usage_key = lUsageKey;
//    tTblSrvInf.usage_key = 0;

    nRet = DbsSRVINF (DBS_SELECT, &tTblSrvInf);
    if (nRet)
    {
        printf("CommP nRet[%d] LINE[%d]\n", nRet, __LINE__);
        DbsDisconnect ();
        exit(-2);
    }

    memset(gsLogFile, 0, sizeof(gsLogFile));
    memset(gsMsgFile, 0, sizeof(gsMsgFile));
    sprintf (gsLogFile, "%s.%s.log", gsSrvId, gsSrvSeq);
    sprintf (gsMsgFile, "%s.%s.msg", gsSrvId, gsSrvSeq);
    HtLog (gsLogFile, HT_LOG_MODE_NORMAL, __FILE__,__LINE__, "port: %ld.", port);
    memset ((char *)gatSrvMsq, 0, sizeof (gatSrvMsq));
    nRet = MsqInit (gsSrvId, gatSrvMsq);
    if (nRet)
    {
        HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "MsqInit error,%d", nRet);
        DbsDisconnect ();
        exit(-3);
    }

    if ((lspTmp = getenv(SAMEnvCommMsgLenFmt)) == NULL) {
        HtLog(gsLogFile, HT_LOG_MODE_ERROR,__FILE__,__LINE__, "Call getenv(SAMEnvCommMsgLenFmt) error %d\n", errno);
        exit(-10);
    } /* end of if */

    gsTcpMsgLenType = *lspTmp++;
    if(((gnTcpMsgLenL = atoi(lspTmp)) < 0) || (gnTcpMsgLenL > NCMaxTcpMsgLenL)) {
        HtLog (gsLogFile, HT_LOG_MODE_ERROR,__FILE__,__LINE__, "Call atoi(gnTcpMsgLenL) error %d\n", errno);
        exit(-11);
    } /* end of if */

    DbsDisconnect ();

    if (sigset(SIGCHLD, Wait) == SIG_ERR)
            HtLog(gsLogFile,  HT_LOG_MODE_ERROR, __FILE__,__LINE__, "sigset SIGCHLD error, %d.", errno);

    if (sigset(SIGTERM, HandleExit) == SIG_ERR)
            HtLog(gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "sigset SIGTERM error, %d.", errno);

    while(1)
    {
        /* ´ÓÏûÏ¢¶ÓÁÐÖÐ»ñÈ¡ÏûÏ¢ */
        memset (sMsgBuf, 0, sizeof(sMsgBuf) );

        sigrelse (SIGTERM);
        nMsgLen = MSQ_MSG_SIZE_MAX;
        nReturnCode = MsqRcv (gsSrvId, gatSrvMsq, 0, MSQ_RCV_MODE_BLOCK, &nMsgLen, sMsgBuf);
        sighold (SIGTERM);
        if (nReturnCode)
        {
            if (nReturnCode != ERR_CODE_MSQ_BASE + EINTR)
            {
                 HtLog( gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "MsqRcv error, %d.", nReturnCode);
                 return;
            }
            else
            continue;
        }

        HtLog( gsLogFile, HT_LOG_MODE_NORMAL, __FILE__,__LINE__, "MsqRcv msglen=%d.", nMsgLen);
        HtWriteLog (gsMsgFile, HT_LOG_MODE_NORMAL, __FILE__,__LINE__, sMsgBuf, nMsgLen);
        HtDebugString(gsLogFile, HT_LOG_MODE_NORMAL, __FILE__, __LINE__, sMsgBuf, nMsgLen);

        if ( nPid = fork() )
        {
            ChdNum++;
            continue;
        }

        if ( !nPid )
        {
            nReturnCode = HandleRequestMessage((char*)sMsgBuf, nMsgLen);
            if ( nReturnCode )
            {
                HtLog( gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "HandleRequest error, %d.", nReturnCode);
                exit(0);
            }
            exit(0);
        }
    }
}


int HandleRequestMessage(char *psMsgBuf, int nMsgLen)
{
    int  socket_id;
    int  nMsgRecvLen;
    int  nReturnCode;
    int  ret;
    char sMsgRevBuf[4096];
    unsigned char sMsgSndBuf[4096];
    unsigned char sMsgConvBuf[4096];
    char tmpBuf[4096];
    char sTxnNum[5];
    char tmp[7];
    char sHandFor1702[12];
    char sHandFor1702Old[12];
    char sHandFor1718[47];
    char sHandFor1718old[47];
    char str[30];
    memset(str,0x00,sizeof(str));
    memset(sMsgRevBuf, 0x00, sizeof(sMsgRevBuf));
    memset(sMsgSndBuf, 0x00, sizeof(sMsgSndBuf));
    memset(tmp, 0x00, sizeof(tmp));
    int ConvertLen;


    memcpy(sMsgRevBuf, psMsgBuf, SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN);
	if(!(memcmp(psMsgBuf+SRV_ID_LEN, "1701", SRV_ID_LEN) == 0
		||memcmp(psMsgBuf+SRV_ID_LEN, "1705", SRV_ID_LEN) == 0))

	{

	    socket_id = nCConnectSocket(port, IPdress);
	    if (socket_id < 0 && gnBackupHostPort != 0 && gsBackupHostIp[0] != 0x00 ) {
	        socket_id = nCConnectSocket(gnBackupHostPort, gsBackupHostIp);
	    }

	    if( socket_id < 0 )  {
	        HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "nCConnectSocket error [%d]", socket_id);
	        exit(1);
	    }
	}

    HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "create socket OK!, socket_id = %d", socket_id);

    if(memcmp(psMsgBuf+SRV_ID_LEN, "1703", SRV_ID_LEN) == 0)
    {
        sprintf(sMsgSndBuf, "%6.6d",nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN)+2);
            sMsgSndBuf[6] = (nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN))/256;
        sMsgSndBuf[7] = (nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN))%256;
        memcpy(sMsgSndBuf+8,psMsgBuf+SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN, nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN));
        HtDebugString(gsLogFile, HT_LOG_MODE_DEBUG, __FILE__, __LINE__, sMsgSndBuf,nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN)+8);
        nReturnCode = WriteSocket(socket_id, sMsgSndBuf, nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN)+8);
        HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "WriteSocket [%d]", nReturnCode);
        if (nReturnCode <= 0)
        {
        HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__,
               "Write socket error");
        close(socket_id);
        exit(1);
        }

        sigset(SIGALRM, Rdmsg_to);
       // alarm(COMI_MSG_READTO);
        int  iTimeout=60;
        if (getenv("TL_COMM_TIMEOUT"))
        iTimeout=atoi (getenv("TL_COMM_TIMEOUT"));
        alarm(iTimeout);
        HtLog(gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "ReadSocket Begin [%d]second......", iTimeout);

        nMsgRecvLen=read(socket_id, tmp, 6);
        if (nMsgRecvLen <= 0)
        {
        if (nMsgLen == E_SBREAK)
            HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "E_SBREAK error");
        HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "read socket call error[%d]", nMsgRecvLen);
        close(socket_id);
        exit(1);
        }
        alarm(0);
        nMsgRecvLen = ReadSocket(socket_id, sMsgRevBuf+SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN, atoi(tmp),"1703Recvpacket");
        if (nMsgRecvLen <= 0)
        {
        if (nMsgLen == E_SBREAK)
            HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "E_SBREAK error");
        HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "read socket call error[%d]", nMsgRecvLen);
        close(socket_id);
        exit(1);
        }


    }
	else if(memcmp(psMsgBuf+SRV_ID_LEN, "1701", SRV_ID_LEN) == 0)
	{
		memcpy(sHandFor1702,"\x60\x00\x06\x00\x00\x60\x31\x00\x31\x10\x03\x00",12);

		sMsgSndBuf[0] = (nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN))/256;
        sMsgSndBuf[1] = (nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN))%256;
        memcpy(sMsgSndBuf+2,psMsgBuf+SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN, nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN));
		memcpy(sMsgSndBuf+2,sHandFor1702,11);

		HtDebugString(gsLogFile, HT_LOG_MODE_DEBUG, __FILE__, __LINE__, sMsgSndBuf,nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN)+2);

		Print8583Packet("1701sendpacket",sMsgSndBuf, nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN)+2);
		nReturnCode = WriteSocket(socket_id, sMsgSndBuf, nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN)+2);
        HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "WriteSocket [%d]", nReturnCode);
        if (nReturnCode <= 0)
        {
              HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "Write socket error");
              close(socket_id);
              exit(1);
        }

        sigset(SIGALRM, Rdmsg_to);
       // alarm(COMI_MSG_READTO);

        nMsgRecvLen = ReadSocket(socket_id, sMsgRevBuf+SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN, MES_BUF_SIZE,"1701Recvpacket");
        if (nMsgRecvLen <= 0)
        {
             if (nMsgLen == E_SBREAK)
                  HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "E_SBREAK error");
             HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "read socket call error[%d],%s", nMsgRecvLen,strerror(nMsgRecvLen));
             close(socket_id);
             exit(1);
        }

	}
	else if(memcmp(psMsgBuf+SRV_ID_LEN, "1705", SRV_ID_LEN) == 0)
	{
		memcpy(sHandFor1702,"\x60\x00\x06\x00\x00\x60\x31\x00\x31\x10\x03\x00",12);

		sMsgSndBuf[0] = (nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN))/256;
        sMsgSndBuf[1] = (nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN))%256;
        memcpy(sMsgSndBuf+2,psMsgBuf+SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN, nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN));
		memcpy(sMsgSndBuf+2,sHandFor1702,11);

		HtDebugString(gsLogFile, HT_LOG_MODE_DEBUG, __FILE__, __LINE__, sMsgSndBuf,nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN)+2);


		Print8583Packet("1705sendpacket",sMsgSndBuf, nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN)+2);
		nReturnCode = WriteSocket(socket_id, sMsgSndBuf, nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN)+2);
        HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "WriteSocket [%d]", nReturnCode);
        if (nReturnCode <= 0)
        {
              HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "Write socket error");
              close(socket_id);
              exit(1);
        }

        sigset(SIGALRM, Rdmsg_to);
       // alarm(COMI_MSG_READTO);

        nMsgRecvLen = ReadSocket(socket_id, sMsgRevBuf+SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN, MES_BUF_SIZE,"1705Recvpacket");
        if (nMsgRecvLen <= 0)
        {
             if (nMsgLen == E_SBREAK)
                  HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "E_SBREAK error");
             HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "read socket call error[%d],%s", nMsgRecvLen,strerror(nMsgRecvLen));
             close(socket_id);
             exit(1);
        }

	}
	else if(memcmp(psMsgBuf+SRV_ID_LEN, "1706", SRV_ID_LEN) == 0)
	{
		int SendLen;
		memcpy(sHandFor1702,"\x60\x00\x00\x00\x00\x01\x01\x00\x00\x00\x00\x00",12);

		sMsgSndBuf[0] = (nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN))/256;
        sMsgSndBuf[1] = (nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN))%256;
        memcpy(sMsgSndBuf+2,psMsgBuf+SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN, nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN));
		memcpy(sMsgSndBuf+2,sHandFor1702,11);

		HtDebugString(gsLogFile, HT_LOG_MODE_DEBUG, __FILE__, __LINE__, sMsgSndBuf,nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN)+2);
		Print8583Packet("1706sendpacket",sMsgSndBuf, nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN)+2);
		nReturnCode = WriteSocket(socket_id, sMsgSndBuf, nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN)+2);
        HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "WriteSocket [%d]", nReturnCode);
        if (nReturnCode <= 0)
        {
              HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "Write socket error");
              close(socket_id);
              exit(1);
        }

        sigset(SIGALRM, Rdmsg_to);
       // alarm(COMI_MSG_READTO);

        nMsgRecvLen = ReadSocket(socket_id, sMsgRevBuf+SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN, MES_BUF_SIZE,"1706Recvpacket");
        if (nMsgRecvLen <= 0)
        {
             if (nMsgLen == E_SBREAK)
                  HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "E_SBREAK error");
             HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "read socket call error[%d],%s", nMsgRecvLen,strerror(nMsgRecvLen));
             close(socket_id);
             exit(1);
        }
		DelFld56(sMsgRevBuf+SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN-2,nMsgRecvLen+2,&nMsgRecvLen);
		nMsgRecvLen-=2;
		Print8583Packet("1706Recvpacket",sMsgRevBuf+SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN-2, nMsgRecvLen);
	}
	else if(memcmp(psMsgBuf+SRV_ID_LEN, "1707", SRV_ID_LEN) == 0)
	{
		//memcpy(sHandFor1702,"\x60\x04\x08\x00\x00\x60\x31\x00\x31\x10\x04\x00",12);
        memcpy(sHandFor1702,"\x60\x03\x63\x00\x00\x60\x31\x00\x31\x10\x04\x00",12);
		sMsgSndBuf[0] = (nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN))/256;
        sMsgSndBuf[1] = (nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN))%256;
        memcpy(sMsgSndBuf+2,psMsgBuf+SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN, nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN));
		memcpy(sMsgSndBuf+2,sHandFor1702,11);

		HtDebugString(gsLogFile, HT_LOG_MODE_DEBUG, __FILE__, __LINE__, sMsgSndBuf,nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN)+2);
		Print8583Packet("1707sendpacket",sMsgSndBuf, nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN)+2);
		nReturnCode = WriteSocket(socket_id, sMsgSndBuf, nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN)+2);
        HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "WriteSocket [%d]", nReturnCode);
        if (nReturnCode <= 0)
        {
              HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "Write socket error");
              close(socket_id);
              exit(1);
        }

        sigset(SIGALRM, Rdmsg_to);
       // alarm(COMI_MSG_READTO);

        nMsgRecvLen = ReadSocket(socket_id, sMsgRevBuf+SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN, MES_BUF_SIZE,"1707Recvpacket");
        if (nMsgRecvLen <= 0)
        {
             if (nMsgLen == E_SBREAK)
                  HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "E_SBREAK error");
             HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "read socket call error[%d],%s", nMsgRecvLen,strerror(nMsgRecvLen));
             close(socket_id);
             exit(1);
        }
	}
    else if(memcmp(psMsgBuf+SRV_ID_LEN, "1716", SRV_ID_LEN) == 0)
    {
        //memcpy(sHandFor1702,"\x60\x04\x08\x00\x00\x60\x31\x00\x31\x10\x04\x00",12); 60 00 00 00 00 60 31 00 31 01 01
        memcpy(sHandFor1702,"\x60\x00\x00\x00\x00\x60\x31\x00\x31\x01\x01\x00",12);
        sMsgSndBuf[0] = (nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN))/256;
        sMsgSndBuf[1] = (nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN))%256;
        memcpy(sMsgSndBuf+2,psMsgBuf+SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN, nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN));
        memcpy(sMsgSndBuf+2,sHandFor1702,11);

        HtDebugString(gsLogFile, HT_LOG_MODE_DEBUG, __FILE__, __LINE__, sMsgSndBuf,nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN)+2);
        Print8583Packet("1716sendpacket",sMsgSndBuf, nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN)+2);
        nReturnCode = WriteSocket(socket_id, sMsgSndBuf, nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN)+2);
        HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "WriteSocket [%d]", nReturnCode);
        if (nReturnCode <= 0)
        {
              HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "Write socket error");
              close(socket_id);
              exit(1);
        }

        sigset(SIGALRM, Rdmsg_to);
       // alarm(COMI_MSG_READTO);

        nMsgRecvLen = ReadSocket(socket_id, sMsgRevBuf+SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN, MES_BUF_SIZE,"1716Recvpacket");
        if (nMsgRecvLen <= 0)
        {
             if (nMsgLen == E_SBREAK)
                  HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "E_SBREAK error");
             HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "read socket call error[%d],%s", nMsgRecvLen,strerror(nMsgRecvLen));
             close(socket_id);
             exit(1);
        }
    }
	else if(memcmp(psMsgBuf+SRV_ID_LEN, "1717", SRV_ID_LEN) == 0)
	{
		memcpy(sHandFor1702,"\x60\x04\x00\x00\x00\x60\x31\x00\x31\x10\x04\x00",12);

		sMsgSndBuf[0] = (nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN))/256;
        sMsgSndBuf[1] = (nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN))%256;
        memcpy(sMsgSndBuf+2,psMsgBuf+SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN, nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN));
		memcpy(sMsgSndBuf+2,sHandFor1702,11);

		HtDebugString(gsLogFile, HT_LOG_MODE_DEBUG, __FILE__, __LINE__, sMsgSndBuf,nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN)+2);
		Print8583Packet("1717sendpacket",sMsgSndBuf, nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN)+2);
		nReturnCode = WriteSocket(socket_id, sMsgSndBuf, nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN)+2);
        HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "WriteSocket [%d]", nReturnCode);
        if (nReturnCode <= 0)
        {
              HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "Write socket error");
              close(socket_id);
              exit(1);
        }

        sigset(SIGALRM, Rdmsg_to);
       // alarm(COMI_MSG_READTO);

        nMsgRecvLen = ReadSocket(socket_id, sMsgRevBuf+SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN, MES_BUF_SIZE,"1717Recvpacket");
        if (nMsgRecvLen <= 0)
        {
             if (nMsgLen == E_SBREAK)
                  HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "E_SBREAK error");
             HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "read socket call error[%d],%s", nMsgRecvLen,strerror(nMsgRecvLen));
             close(socket_id);
             exit(1);
        }
	}
	else if(memcmp(psMsgBuf+SRV_ID_LEN, "1727", SRV_ID_LEN) == 0)
	{

		//memcpy(sHandFor1702,"\x60\x00\x18\x00\x00\x60\x31\x00\x31\x10\x04\x00",12);

		sMsgSndBuf[0] = (nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN))/256;
		sMsgSndBuf[1] = (nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN))%256;
		memcpy(sMsgSndBuf+2,psMsgBuf+SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN, nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN));
		//memcpy(sMsgSndBuf+2,sHandFor1702,11);

		//HtDebugString(gsLogFile, HT_LOG_MODE_DEBUG, __FILE__, __LINE__, sMsgSndBuf,nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN)+2);
		memcpy(str,psMsgBuf+SRV_ID_LEN,4);
		memcpy(str+4,"sendpacket",10);
		Print8583Packet(str,sMsgSndBuf, nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN)+2);
		nReturnCode = WriteSocket(socket_id, sMsgSndBuf, nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN)+2);
        HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "WriteSocket [%d]", nReturnCode);
        if (nReturnCode <= 0)
        {
              HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "Write socket error");
              close(socket_id);
              exit(1);
        }

        sigset(SIGALRM, Rdmsg_to);
       // alarm(COMI_MSG_READTO);

	    memcpy(str+4,"Recvpacket",10);

        nMsgRecvLen = ReadSocket(socket_id, sMsgRevBuf+SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN, MES_BUF_SIZE,str);
        if (nMsgRecvLen <= 0)
        {
             if (nMsgLen == E_SBREAK)
                  HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "E_SBREAK error");
             HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "read socket call error[%d],%s", nMsgRecvLen,strerror(nMsgRecvLen));
             close(socket_id);
             exit(1);
        }
	}
	else if(memcmp(psMsgBuf+SRV_ID_LEN, "1708", SRV_ID_LEN) == 0)
	{
		memcpy(sHandFor1702,"\x60\x00\x03\x00\x00\x60\x31\x00\x11\x43\x01\x00",12);
/*60 00 03 00 00 60 31 00 11 43 01 */
		sMsgSndBuf[0] = (nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN))/256;
        sMsgSndBuf[1] = (nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN))%256;
        memcpy(sMsgSndBuf+2,psMsgBuf+SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN, nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN));
		memcpy(sMsgSndBuf+2,sHandFor1702,11);

		HtDebugString(gsLogFile, HT_LOG_MODE_DEBUG, __FILE__, __LINE__, sMsgSndBuf,nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN)+2);
		Print8583Packet("1708sendpacket",sMsgSndBuf, nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN)+2);
		nReturnCode = WriteSocket(socket_id, sMsgSndBuf, nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN)+2);
        HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "WriteSocket [%d]", nReturnCode);
        if (nReturnCode <= 0)
        {
              HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "Write socket error");
              close(socket_id);
              exit(1);
        }

        sigset(SIGALRM, Rdmsg_to);
       // alarm(COMI_MSG_READTO);

        nMsgRecvLen = ReadSocket(socket_id, sMsgRevBuf+SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN, MES_BUF_SIZE,"1708Recvpacket");
        if (nMsgRecvLen <= 0)
        {
             if (nMsgLen == E_SBREAK)
                  HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "E_SBREAK error");
             HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "read socket call error[%d],%s", nMsgRecvLen,strerror(nMsgRecvLen));
             close(socket_id);
             exit(1);
        }
	}
    else if(memcmp(psMsgBuf+SRV_ID_LEN, "1709", SRV_ID_LEN) == 0)
	{
		memcpy(sHandFor1702,"\x60\x00\x18\x00\x01\x60\x31\x00\x31\x30\x91\x00",12);
        /*60 00 18 00 01 60 31 00 31 30 91*/
		sMsgSndBuf[0] = (nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN))/256;
        sMsgSndBuf[1] = (nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN))%256;
        memcpy(sMsgSndBuf+2,psMsgBuf+SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN, nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN));
		memcpy(sMsgSndBuf+2,sHandFor1702,11);

		HtDebugString(gsLogFile, HT_LOG_MODE_DEBUG, __FILE__, __LINE__, sMsgSndBuf,nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN)+2);
		Print8583Packet("1709sendpacket",sMsgSndBuf, nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN)+2);
		nReturnCode = WriteSocket(socket_id, sMsgSndBuf, nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN)+2);
        HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "WriteSocket [%d]", nReturnCode);
        if (nReturnCode <= 0)
        {
              HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "Write socket error");
              close(socket_id);
              exit(1);
        }

        sigset(SIGALRM, Rdmsg_to);
       // alarm(COMI_MSG_READTO);

        nMsgRecvLen = ReadSocket(socket_id, sMsgRevBuf+SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN, MES_BUF_SIZE,"1709Recvpacket");
        if (nMsgRecvLen <= 0)
        {
             if (nMsgLen == E_SBREAK)
                  HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "E_SBREAK error");
             HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "read socket call error[%d],%s", nMsgRecvLen,strerror(nMsgRecvLen));
             close(socket_id);
             exit(1);
        }
        DelFld21(sMsgRevBuf+SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN-2,nMsgRecvLen+2,&nMsgRecvLen);
		nMsgRecvLen-=2;
	}

	else if(memcmp(psMsgBuf+SRV_ID_LEN, "1702", SRV_ID_LEN) == 0)
	{
        //60 00 10 00 00 01 00
        //60 00 10 00 00 03 03
		memcpy(sHandFor1702,"\x60\x00\x10\x00\x00\01\x00\x00\x31\x10\x03\x00",12);
			sMsgSndBuf[0] = (nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN)+4)/256;
			sMsgSndBuf[1] = (nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN)+4)%256;
			memcpy(sMsgSndBuf+2,sHandFor1702,7);
			memcpy(sHandFor1702Old,psMsgBuf+SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN,7);
			memcpy(sMsgSndBuf+2+7,psMsgBuf+SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN+7, nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN+7));
			HtDebugString(gsLogFile, HT_LOG_MODE_DEBUG, __FILE__, __LINE__, sMsgSndBuf,nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN)+2);
            Print8583Packet("1702sendpacket",sMsgSndBuf, nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN)+2);
			nReturnCode = WriteSocket(socket_id, sMsgSndBuf, nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN)+4+2);
			HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "WriteSocket [%d]", nReturnCode);
			if (nReturnCode <= 0)
			{
				  HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "Write socket error");
				  close(socket_id);
				  exit(1);
			}

			sigset(SIGALRM, Rdmsg_to);
		   // alarm(COMI_MSG_READTO);

			nMsgRecvLen = ReadSocket(socket_id, sMsgRevBuf+SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN, MES_BUF_SIZE,"1702Recvpacket");
			if (nMsgRecvLen <= 0)
			{
				 if (nMsgLen == E_SBREAK)
					  HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "E_SBREAK error");
				 HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "read socket call error[%d],%s", nMsgRecvLen,strerror(nMsgRecvLen));
				 close(socket_id);
				 exit(1);
			}
			//nMsgRecvLen-=4;

			memcpy(sMsgRevBuf+SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN,sHandFor1702Old,7);
			memcpy(tmpBuf,sMsgRevBuf+SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN+7,nMsgRecvLen-7);
			//HtDebugString(gsLogFile, HT_LOG_MODE_DEBUG, __FILE__, __LINE__, tmpBuf, nMsgRecvLen-7);

			memcpy(sMsgRevBuf+SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN+7,tmpBuf,nMsgRecvLen-7);
			// modify by hxb 2015111
	}
    // add at time 20170206 hxb  fu lin men
    else if(memcmp(psMsgBuf+SRV_ID_LEN, "1718", SRV_ID_LEN) == 0)
    {
        //60 00 10 00 00 01 00
        //60 00 10 00 00 03 03
       // memcpy(sHandFor1718,"\x60\x00\x10\x00\x00\01\x00\x00\x31\x10\x03\x00",12);
            memcpy(sHandFor1718, "\x2e\x01", 2);
            memcpy(sHandFor1718+2, "\x30\x30\x30\x30", 4);
            // 36 30 31 39 31 32 34 31 20 20 20
            memcpy(sHandFor1718+6, "\x36\x30\x31\x39\x31\x32\x34\x31\x20\x20\x20", 11);
            memcpy(sHandFor1718+17, "\x36\x30\x31\x39\x31\x32\x34\x31\x20\x20\x20", 11);
            memcpy(sHandFor1718+28, "\x30\x30\x30",3);
            memcpy(sHandFor1718+31, "\x30", 1);
            memcpy(sHandFor1718+32, "\x30\x30\x30\x30\x30\x30\x30\x30", 8);
            memcpy(sHandFor1718+40, "\x30", 1 );
            memcpy(sHandFor1718+41, "\x30\x30\x30\x30\x30", 5);
            sMsgSndBuf[0] = (nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN))/256;
            sMsgSndBuf[1] = (nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN))%256;
            memcpy(sMsgSndBuf+2,sHandFor1718,46);
            //memcpy(sHandFor1718old,psMsgBuf+SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN,46);
            memcpy(sMsgSndBuf+2+46,psMsgBuf+SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN+11, nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN+11));

            HtDebugString(gsLogFile, HT_LOG_MODE_DEBUG, __FILE__, __LINE__, sMsgSndBuf,nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN)+2);
            Print8583Packet("1718sendpacket",sMsgSndBuf, nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN)+2);

            Convert64To128(sMsgSndBuf+2,sMsgConvBuf, nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN)+2,&ConvertLen);

            nReturnCode = WriteSocket(socket_id, sMsgSndBuf, nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN)+4+2);
            HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "WriteSocket [%d]", nReturnCode);
            if (nReturnCode <= 0)
            {
                  HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "Write socket error");
                  close(socket_id);
                  exit(1);
            }

            sigset(SIGALRM, Rdmsg_to);
           // alarm(COMI_MSG_READTO);

           /****
            nMsgRecvLen = ReadSocket(socket_id, sMsgRevBuf+SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN, MES_BUF_SIZE,"1702Recvpacket");
            if (nMsgRecvLen <= 0)
            {
                 if (nMsgLen == E_SBREAK)
                      HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "E_SBREAK error");
                 HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "read socket call error[%d],%s", nMsgRecvLen,strerror(nMsgRecvLen));
                 close(socket_id);
                 exit(1);
            }
            //nMsgRecvLen-=4;

            memcpy(sMsgRevBuf+SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN,sHandFor1702Old,7);
            memcpy(tmpBuf,sMsgRevBuf+SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN+7,nMsgRecvLen-7);
            //HtDebugString(gsLogFile, HT_LOG_MODE_DEBUG, __FILE__, __LINE__, tmpBuf, nMsgRecvLen-7);

            memcpy(sMsgRevBuf+SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN+7,tmpBuf,nMsgRecvLen-7);
            ****/
            // modify by hxb 2015111
    }
	else if(memcmp(psMsgBuf+SRV_ID_LEN, "1801", SRV_ID_LEN) == 0)
	{
			memcpy(sHandFor1702,"\x60\x00\x06\x00\x00\x60\x31\x00\x31\x10\x03\x00",12);

			sMsgSndBuf[0] = (nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN))/256;
			sMsgSndBuf[1] = (nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN))%256;
			memcpy(sMsgSndBuf+2,psMsgBuf+SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN, nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN));
			memcpy(sMsgSndBuf+2,sHandFor1702,11);

			HtDebugString(gsLogFile, HT_LOG_MODE_DEBUG, __FILE__, __LINE__, sMsgSndBuf,nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN)+2);




			Print8583Packet("1801sendpacket",sMsgSndBuf, nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN)+2);
			nReturnCode = WriteSocket(socket_id, sMsgSndBuf, nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN)+2);
			HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "WriteSocket [%d]", nReturnCode);
			if (nReturnCode <= 0)
			{
				HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "Write socket error");
				close(socket_id);
				exit(1);
			}

			sigset(SIGALRM, Rdmsg_to);
			// alarm(COMI_MSG_READTO);

			nMsgRecvLen = ReadSocket(socket_id, sMsgRevBuf+SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN, MES_BUF_SIZE,"1801Recvpacket");
			if (nMsgRecvLen <= 0)
			{
				if (nMsgLen == E_SBREAK)
					HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "E_SBREAK error");
				HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "read socket call error[%d],%s", nMsgRecvLen,strerror(nMsgRecvLen));
				close(socket_id);
				exit(1);
			}

	}
	else if(memcmp(psMsgBuf+SRV_ID_LEN, "1710", SRV_ID_LEN) == 0)
	{
			memcpy(sHandFor1702,"\x60\x00\x06\x00\x00\x60\x01\x00\x01\x01\x02\x00",12);

			sMsgSndBuf[0] = (nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN))/256;
			sMsgSndBuf[1] = (nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN))%256;
			memcpy(sMsgSndBuf+2,psMsgBuf+SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN, nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN));
			memcpy(sMsgSndBuf+2,sHandFor1702,11);

			HtDebugString(gsLogFile, HT_LOG_MODE_DEBUG, __FILE__, __LINE__, sMsgSndBuf,nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN)+2);




			Print8583Packet("1710sendpacket",sMsgSndBuf, nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN)+2);
			nReturnCode = WriteSocket(socket_id, sMsgSndBuf, nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN)+2);
			HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "WriteSocket [%d]", nReturnCode);
			if (nReturnCode <= 0)
			{
				HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "Write socket error");
				close(socket_id);
				exit(1);
			}

			sigset(SIGALRM, Rdmsg_to);
			// alarm(COMI_MSG_READTO);

			nMsgRecvLen = ReadSocket(socket_id, sMsgRevBuf+SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN, MES_BUF_SIZE,"1710Recvpacket");
			if (nMsgRecvLen <= 0)
			{
				if (nMsgLen == E_SBREAK)
					HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "E_SBREAK error");
				HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "read socket call error[%d],%s", nMsgRecvLen,strerror(nMsgRecvLen));
				close(socket_id);
				exit(1);
			}

	}
	else if(memcmp(psMsgBuf+SRV_ID_LEN, "1711", SRV_ID_LEN) == 0)
	{
		//memcpy(sHandFor1702,"\x60\x00\x18\x00\x01\x60\x31\x00\x31\x30\x91\x00",12);
		memcpy(sHandFor1702,"\x60\x00\x04\x00\x00\x60\x22\x00\x00\x00\x00\x00",12);
        /*60 00 18 00 01 60 31 00 31 30 91*/
		sMsgSndBuf[0] = (nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN))/256;
        sMsgSndBuf[1] = (nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN))%256;
        memcpy(sMsgSndBuf+2,psMsgBuf+SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN, nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN));
		memcpy(sMsgSndBuf+2,sHandFor1702,11);

		HtDebugString(gsLogFile, HT_LOG_MODE_DEBUG, __FILE__, __LINE__, sMsgSndBuf,nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN)+2);
		Print8583Packet("1711sendpacket",sMsgSndBuf, nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN)+2);
		nReturnCode = WriteSocket(socket_id, sMsgSndBuf, nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN)+2);
        HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "WriteSocket [%d]", nReturnCode);
        if (nReturnCode <= 0)
        {
              HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "Write socket error");
              close(socket_id);
              exit(1);
        }

        sigset(SIGALRM, Rdmsg_to);
       // alarm(COMI_MSG_READTO);

        nMsgRecvLen = ReadSocket(socket_id, sMsgRevBuf+SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN, MES_BUF_SIZE,"1711Recvpacket");
        if (nMsgRecvLen <= 0)
        {
             if (nMsgLen == E_SBREAK)
                  HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "E_SBREAK error");
             HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "read socket call error[%d],%s", nMsgRecvLen,strerror(nMsgRecvLen));
             close(socket_id);
             exit(1);
        }
		DelFld31(sMsgRevBuf+SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN-2,nMsgRecvLen+2,&nMsgRecvLen);
	}
	else if(memcmp(psMsgBuf+SRV_ID_LEN, "1712", SRV_ID_LEN) == 0)
	{
		//memcpy(sHandFor1702,"\x60\x00\x18\x00\x01\x60\x31\x00\x31\x30\x91\x00",12);
		memcpy(sHandFor1702,"\x60\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00",12);
        /*60 00 18 00 01 60 31 00 31 30 91*/
		sMsgSndBuf[0] = (nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN))/256;
        sMsgSndBuf[1] = (nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN))%256;
        memcpy(sMsgSndBuf+2,psMsgBuf+SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN, nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN));
		memcpy(sMsgSndBuf+2,sHandFor1702,11);

		HtDebugString(gsLogFile, HT_LOG_MODE_DEBUG, __FILE__, __LINE__, sMsgSndBuf,nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN)+2);
		Print8583Packet("1712sendpacket",sMsgSndBuf, nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN)+2);
		nReturnCode = WriteSocket(socket_id, sMsgSndBuf, nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN)+2);
        HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "WriteSocket [%d]", nReturnCode);
        if (nReturnCode <= 0)
        {
              HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "Write socket error");
              close(socket_id);
              exit(1);
        }

        sigset(SIGALRM, Rdmsg_to);
       // alarm(COMI_MSG_READTO);

        nMsgRecvLen = ReadSocket(socket_id, sMsgRevBuf+SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN, MES_BUF_SIZE,"1712Recvpacket");
        if (nMsgRecvLen <= 0)
        {
             if (nMsgLen == E_SBREAK)
                  HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "E_SBREAK error");
             HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "read socket call error[%d],%s", nMsgRecvLen,strerror(nMsgRecvLen));
             close(socket_id);
             exit(1);
        }
	}
	else if(memcmp(psMsgBuf+SRV_ID_LEN, "1713", SRV_ID_LEN) == 0)
	{
		memcpy(sHandFor1702,"\x60\x00\x32\x00\x00\x60\x31\x00\x31\x12\x03\x00",12);
        /*\x60\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00*/
		sMsgSndBuf[0] = (nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN))/256;
        sMsgSndBuf[1] = (nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN))%256;
        memcpy(sMsgSndBuf+2,psMsgBuf+SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN, nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN));
		memcpy(sMsgSndBuf+2,sHandFor1702,11);

		HtDebugString(gsLogFile, HT_LOG_MODE_DEBUG, __FILE__, __LINE__, sMsgSndBuf,nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN)+2);
		Print8583Packet("1713sendpacket",sMsgSndBuf, nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN)+2);
		nReturnCode = WriteSocket(socket_id, sMsgSndBuf, nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN)+2);
        HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "WriteSocket [%d]", nReturnCode);
        if (nReturnCode <= 0)
        {
              HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "Write socket error");
              close(socket_id);
              exit(1);
        }

        sigset(SIGALRM, Rdmsg_to);
       // alarm(COMI_MSG_READTO);

        nMsgRecvLen = ReadSocket(socket_id, sMsgRevBuf+SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN, MES_BUF_SIZE,"1713Recvpacket");
        if (nMsgRecvLen <= 0)
        {
             if (nMsgLen == E_SBREAK)
                  HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "E_SBREAK error");
             HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "read socket call error[%d],%s", nMsgRecvLen,strerror(nMsgRecvLen));
             close(socket_id);
             exit(1);
        }
	}
	else if(memcmp(psMsgBuf+SRV_ID_LEN, "1715", SRV_ID_LEN) == 0)
	{
		memcpy(sHandFor1702,"\x60\x00\x04\x00\x00\x60\x00\x00\x14\x10\x20\x00",12);
        /*\x60\x00\x32\x00\x00\x60\x31\x00\x31\x12\x03\x00*/
		sMsgSndBuf[0] = (nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN))/256;
        sMsgSndBuf[1] = (nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN))%256;
        memcpy(sMsgSndBuf+2,psMsgBuf+SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN, nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN));
		memcpy(sMsgSndBuf+2,sHandFor1702,11);

		HtDebugString(gsLogFile, HT_LOG_MODE_DEBUG, __FILE__, __LINE__, sMsgSndBuf,nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN)+2);
		Print8583Packet("1715sendpacket",sMsgSndBuf, nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN)+2);
		nReturnCode = WriteSocket(socket_id, sMsgSndBuf, nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN)+2);
        HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "WriteSocket [%d]", nReturnCode);
        if (nReturnCode <= 0)
        {
              HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "Write socket error");
              close(socket_id);
              exit(1);
        }

        sigset(SIGALRM, Rdmsg_to);
       // alarm(COMI_MSG_READTO);

        nMsgRecvLen = ReadSocket(socket_id, sMsgRevBuf+SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN, MES_BUF_SIZE,"1715Recvpacket");
        if (nMsgRecvLen <= 0)
        {
             if (nMsgLen == E_SBREAK)
                  HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "E_SBREAK error");
             HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "read socket call error[%d],%s", nMsgRecvLen,strerror(nMsgRecvLen));
             close(socket_id);
             exit(1);
        }
		if(memcmp(sMsgRevBuf+SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN+11,"\x08\x10",2)==0)
		{
			DelFld59(sMsgRevBuf+SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN-2,nMsgRecvLen+2,&nMsgRecvLen);
			nMsgRecvLen-=2;
		}
	}
	else
    {
        sMsgSndBuf[0] = (nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN))/256;
        sMsgSndBuf[1] = (nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN))%256;
        memcpy(sMsgSndBuf+2,psMsgBuf+SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN, nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN));
        HtDebugString(gsLogFile, HT_LOG_MODE_DEBUG, __FILE__, __LINE__, sMsgSndBuf,nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN)+2);
        nReturnCode = WriteSocket(socket_id, sMsgSndBuf, nMsgLen-(SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN)+2);
        HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "WriteSocket [%d]", nReturnCode);
        if (nReturnCode <= 0)
        {
              HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "Write socket error");
              close(socket_id);
              exit(1);
        }

        sigset(SIGALRM, Rdmsg_to);
       // alarm(COMI_MSG_READTO);

        nMsgRecvLen = ReadSocket(socket_id, sMsgRevBuf+SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN, MES_BUF_SIZE,"");
        if (nMsgRecvLen <= 0)
        {
             if (nMsgLen == E_SBREAK)
                  HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "E_SBREAK error");
             HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "read socket call error[%d],%s", nMsgRecvLen,strerror(nMsgRecvLen));
             close(socket_id);
             exit(1);
        }

    }
    memcpy(sMsgRevBuf, gsSrvId, SRV_ID_LEN);
    memcpy(sMsgRevBuf+SRV_ID_LEN, gsToSrvId, SRV_ID_LEN);
	HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "srv_id[%s], to_id[%s]",gsSrvId, gsToSrvId);
	HtDebugString(gsLogFile, HT_LOG_MODE_DEBUG, __FILE__, __LINE__, sMsgRevBuf, nMsgRecvLen+SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN+2);

    //HtWriteLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, sMsgRevBuf,nMsgRecvLen+SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN);
    nReturnCode= MsqSnd (gsToSrvId, gatSrvMsq, 0, nMsgRecvLen+SRV_ID_LEN*2+FLD_MSQ_TYPE_LEN, sMsgRevBuf);
    if (nReturnCode)
    {
        HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "MsqSnd ToSrvId[%s] error [%d][%d]:[%s]",
            gsToSrvId, nReturnCode, errno, strerror(errno));
        close(socket_id);
        exit(1);
    }
	if(!(memcmp(psMsgBuf+SRV_ID_LEN, "1701", SRV_ID_LEN) == 0
		||memcmp(psMsgBuf+SRV_ID_LEN, "1705", SRV_ID_LEN) == 0))

		{


    close(socket_id);
	}
    return 0;
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

void Rdmsg_to()
{
/*    HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__,
           "Read from message queue over time!");*/
    HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__,
           "Read from socket over time!");
    return_flag=1;
    exit(1);
}

void SaveMsg (long lPid)
{
    int                nReturnCode;
    Tbl_dumpmsg_Def    tTblDumpmsg;

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
    HtLog( gsLogFile, HT_LOG_MODE_NORMAL, __FILE__,__LINE__, "CommP exits.");
    exit( 1 );
}

int ReadSocket(int socket_id_new,char *buf ,int len,char *Srv_id_str)
{
    int   num,num2, i=0, iTimeout=60;
    unsigned short nLen;
    char  Buf_head[9];
    char  tmp_Str[4096];
    char  *ptm_str;
    short tt;
    char sLen[2+1];

    memset(sLen, 0, sizeof(sLen));

    if (getenv("TL_COMM_TIMEOUT"))
    iTimeout=atoi (getenv("TL_COMM_TIMEOUT"));
    alarm(iTimeout);

    memset(Buf_head, 0, sizeof(Buf_head));
    memset((char*)&tmp_Str[0], 0x00, sizeof(tmp_Str));

    HtLog(gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "ReadSocket Begin [%d]second......", iTimeout);

    num = read(socket_id_new, &tt, 2);
    //num = read(socket_id_new, sLen, 2);
    if ( num < 0 )
    {
        HtLog(gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "read error num=%d", num);

        if (errno == EINTR) return;
        return (E_SBREAK);
    }
    HtLog(gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "num=%d", num);
    if ( num == 0 ) return (E_SBREAK);
       nLen = ntohs(tt);
    HtLog(gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "read nLen=%d", nLen);
    //nLen = sLen[0] *256 + sLen[1]; HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "nLen = %d", nLen);
    if ( nLen == 0 ) return (E_SBREAK);

    memcpy(tmp_Str, (char*)&tt, 2);
    ptm_str = tmp_Str+2;

    while((num=read(socket_id_new, ptm_str, nLen))<=0)
    {

        if(errno==EINTR) continue;
        HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__,
               "read socket error");
        return(E_SBREAK);
    }
	if(num<nLen)
	{
		HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__,
	               "real read socket num:%d nlen:%d",num,nLen);
		while((num2=read(socket_id_new, ptm_str+num, nLen-num))<=0)
	    {

	        if(errno==EINTR) continue;
	        HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__,
	               "read socket error");
	        return(E_SBREAK);
	    }
		if(num+num2!=nLen)
			HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__,
	               "read socket len error num:%d nlen:%d",num+num2,nLen);
	}

    HtDebugString(gsLogFile, HT_LOG_MODE_DEBUG, __FILE__, __LINE__, tmp_Str, nLen+2);
	Print8583Packet(Srv_id_str,tmp_Str, nLen+2);



    memcpy(buf, ptm_str, nLen);

    HtLog(gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "ReadSocket End");

    return nLen;
}

int WriteSocket(int socket_id_new,char *buf, int len)
{
    int  num, iWritelen;
    char Buf_head[5];
    char saSendBuf[BUF_SIZE];

    if (len == 0) return(0);

    memset(saSendBuf,0,sizeof(saSendBuf));

    memcpy(&saSendBuf[0],buf,len);

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

#if 0
int nCConnectSocket(unsigned short Port, char *Ip_addr)
{
    struct sockaddr_in   Sin;
    int                  nConeCount;
    int                  Socket_id, RetryTimeSap = 2, nRetryFlag = 0;

    HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "Ip_addr, %s", Ip_addr);
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
            #if 0  /*20140404 ÐÞ¸Ä±ÜÃâÎÞÏÞÑ­»·*/
            if (nRetryFlag++ == RETRYNUM)
            {
                if ( RetryTimeSap<100) RetryTimeSap += 2;
                nRetryFlag = 0;
            }
            #else
            nRetryFlag++;
            if (nRetryFlag >= RETRYNUM)
            {
                HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "get socket_id error[%d]", errno);
                return -1;
            }
            #endif
            sleep(RetryTimeSap);
        }

        if ( connect(Socket_id, (struct sockaddr *)&Sin, sizeof(Sin)) < 0)
        {
            nConeCount++;
            HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "connect error[%s][%d], %d",Ip_addr , Port, errno);
            close(Socket_id);
            if (errno == ECONNREFUSED)
            {
                if (nConeCount <= 3)
                    continue;
            }
            if (nConeCount <= 2)
                    continue;
            return -1;
        }
        break;
    }
    return(Socket_id);
}
#endif
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
