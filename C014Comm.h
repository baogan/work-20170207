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
#include <sys/ioctl.h>

#include "SrvDef.h"
#include "DbsDef.h"
#include "DbsTbl.h"
#include "ErrCode.h"
#include "MsqOpr.h"
#include "HtLog.h"
#include "IpcInt.h"

#define CHD_PROCESS_NUM 1
#define MAX_TABLE_ITEM 20
#define TRUE 1
#define FALSE 0
#define TEST 
#define BUF_SIZE 1800 
#define CONNECT_TIME_OUT        10
#define S_INI 5150
#define E_SBREAK -1

typedef  struct tag_msg_def 
{
  long msg_type;
  short msg_src;
  char msg_buf[BUF_SIZE];
} msg_def;
