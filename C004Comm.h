#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/times.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/file.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <fcntl.h>
#include <unistd.h>
/*#include <stropts.h>*/
#include <poll.h>
#include <time.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <math.h>
#include <sys/time.h>
#include <sys/ioctl.h>

#include "SrvDef.h"
#include "SrvParam.h"
#include "DbsDef.h"
#include "DbsTbl.h"
#include "ErrCode.h"
#include "MsqOpr.h"
#include "HtLog.h"
#include "IpcInt.h"
#include "IPPComm.h"

#define NCMaxTcpMsgLenL         4
#define MAX_TCPCHD              50
#define MAX_CONNECTION_NUM      500
#define COMI_MSG_WRITETO        60
#define COMI_MSG_READTO         60
#define RETRYNUM                5
#define MES_BUF_SIZE            4096
#define CHD_COUNT               1000
#define E_SBREAK                -1
#define TRUE                    1
#define FALSE                   0
#define BUF_SIZE                4096
#define CONNECT_TIME_OUT        10

#define SAMEnvCommMsgLenFmt             "TL_COMM_MSG_LEN_FORMAT"


