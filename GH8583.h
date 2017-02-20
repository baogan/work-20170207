#ifndef _GH8583_H
#define _GH8583_H

//#include <stdio.h>

#ifndef uchar
#define uchar   unsigned char
#endif

#ifdef MY_DEBUG
#define MY_TRACE(x,args) fprintf(stderr, x, args)
#else
#define MY_TRACE(x,args)
#endif

#define MY_BITGET(p,n) ((p)[(n-1)/8]&(0x80>>((n-1)%8)))       //取bit位
#define MY_BITSET(p,n) ((p)[(n-1)/8]|=(0x80>>((n-1)%8)))      //设bit位
#define MY_BITCLR(p,n) ((p)[(n-1)/8]&=(~(0x80>>((n-1)%8))))   //清bit位


/*
* ISO8583版本信息，不同版本，ISO_DEF变量g_isodef定义就不同,为0时自定义
*/
#define ISO8583_VERSION 0
//#define MAX_PACKET_LEN	2048
#define MAX_PACKET_LEN	1500

/*
* 数据域个数，正常标准为128，如有需要可以为64
*/
#define ISO_FLDS 64

#define TYP_BIT 0
#define TYP_BCD 1
#define TYP_ASC 2
#define TYP_BIN 3

#define FMT_FIXED 0
#define FMT_LLVAR 2
#define FMT_LLLVAR 3
/*
* ISO定义域结构定义
*/
typedef struct  {
int typ;
int fmt;
int   len;

} ISO_DEF;
#define Printf printf
#define MagPacketHeaderLen 13
#define MagPacketHeaderLen128 46
/*
* ISO数据域结构定义
*/
typedef struct tagISO_MSG {
struct tagISO_FLD {
int len;
unsigned char *buf;
} fld[ISO_FLDS+1];
} ISO_MSG;
void Print8583Packet(uchar *msg, uchar *hexBuff, int BuffLen);
int  SetPacket(uchar *Buff,int Len);
int   GetPacket(uchar *Buff,int BuffLen);
int   GetField(int FieldNum,uchar *Buff,int *DataLen,int *typ);
int   SetField(int FieldNum,uchar *Buff,int DataLen);
void AddFld56(uchar *hexBuff, int BuffLen, int *SendLen);
#endif