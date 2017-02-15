#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "BF8583.h"
#include "HtLog.h"

#define  MAX_BUF_SIZE 1024
#define  PORT_NUMBER 28080
#define DEBUG_SW 1
uchar 	GTCP_OPENED;
int 	GTCPHandle;

typedef unsigned char			u8;

static int s_iCompress = 1;

ISO_DEF s_isodef[65] = {


    /*000*/ {TYP_BCD, 0, 4}, // "Message Type Indicator"
    /*001*/ {TYP_BIT, 0, 128}, // "Bitmap"
    /*002*/ {TYP_BCD, 2, 19}, // "Primary Account number"
    /*003*/ {TYP_BCD, 0, 6}, // "Processing Code"
    /*004*/ {TYP_BCD, 0, 12}, // "Amount, Transaction"
    /*005*/ {TYP_BCD, 0, 12}, // "Amount, Reconciliation"
    /*006*/ {TYP_BCD, 0, 12}, // "Amount, Cardholder billing"
    /*007*/ {TYP_BCD, 0, 10}, // "Date and time, transmission"
    /*008*/ {TYP_BCD, 0, 8}, // "Amount, Cardholder billing fee"
    /*009*/ {TYP_BCD, 0, 8}, // "Conversion rate, Reconciliation"
    /*010*/ {TYP_BCD, 0, 8}, // "Conversion rate, Cardholder billing"
    /*011*/ {TYP_BCD, 0, 6}, // "Systems trace audit number"
    /*012*/ {TYP_BCD, 0, 6}, // "Date and time, Local transaction"
    /*013*/ {TYP_BCD, 0, 4}, // "Date, Effective"
    /*014*/ {TYP_BCD, 0, 4}, // "Date, Expiration"
    /*015*/ {TYP_BCD, 0, 4}, // "Date, Settlement"
    /*016*/ {TYP_BCD, 0, 4}, // "Date, Conversion"
    /*017*/ {TYP_BCD, 0, 4}, // "Date, Capture"
    /*018*/ {TYP_BCD, 0, 4}, // "Merchant type"
    /*019*/ {TYP_BCD, 0, 3}, // "Country code, Acquiring institution"
    /*020*/ {TYP_BCD, 0, 3}, // "Country code, Primary account number"
    /*021*/ {TYP_BCD, 0, 3}, // "Country code, Forwarding institution"
    /*022*/ {TYP_BCD, 0, 3}, // "Point of service data code"
    /*023*/ {TYP_BCD, 0, 3}, // "Card sequence number"
    /*024*/ {TYP_BCD, 0, 3}, // "Function code"
    /*025*/ {TYP_BCD, 0, 2}, // "Message reason code"
    /*026*/ {TYP_BCD, 0, 2}, // "Card acceptor business code"
    /*027*/ {TYP_BCD, 0, 16}, // "Approval code length"
    /*028*/ {TYP_BCD, 0, 6}, // "Date, Reconciliation"
    /*029*/ {TYP_BCD, 0, 3}, // "Reconciliation indicator"
    /*030*/ {TYP_BCD, 0, 24}, // "Amounts, original"
    /*031*/ {TYP_ASC, 2, 99}, // "Acquirer reference data"
    /*032*/ {TYP_BCD, 2, 11}, // "Acquirer institution identification code"
    /*033*/ {TYP_BCD, 2, 11}, // "Forwarding institution identification code"
    /*034*/ {TYP_ASC, 2, 28}, // "Primary account number, extended"
    /*035*/ {TYP_BCD, 2, 37}, // "Track 2 data"
    /*036*/ {TYP_BCD, 3, 104}, // "Track 3 data"
    /*037*/ {TYP_ASC, 0, 12}, // "Retrieval reference number"
    /*038*/ {TYP_ASC, 0, 6}, // "Approval code"
    /*039*/ {TYP_ASC, 0, 2}, // "Action code"
    /*040*/ {TYP_BCD, 0, 3}, // "Service code"
    /*041*/ {TYP_ASC, 0, 8}, // "Card acceptor terminal identification"
    /*042*/ {TYP_ASC, 0, 15}, // "Card acceptor identification code"
    /*043*/ {TYP_ASC, 2, 99}, // "Card acceptor name/location"
    /*044*/ {TYP_ASC, 2, 25}, // "Additional response data"
    /*045*/ {TYP_ASC, 2, 76}, // "Track 1 data"
    /*046*/ {TYP_BCD, 0, 14}, // "Amounts, Fees"
    /*047*/ {TYP_BCD, 0, 18}, // "Additional data - national"
    /*048*/ {TYP_BCD, 3, 322}, // "Additional data - private"TYP_ASC
    /*049*/ {TYP_ASC, 0, 3}, // "Currency code, Transaction"
    /*050*/ {TYP_ASC, 0, 3}, // "Currency code, Reconciliation"
    /*051*/ {TYP_ASC, 0, 3}, // "Currency code, Cardholder billing"
    /*052*/ {TYP_ASC, 0, 8}, // "Personal identification number, PIN) data"
    /*053*/ {TYP_BCD, 0, 16}, // "Security related control information"
    /*054*/ {TYP_ASC, 3, 20}, // "Amounts, additional"
    /*055*/ {TYP_ASC, 3, 255}, // "IC card system related data"
    /*056*/ {TYP_BCD, 0, 6}, // "Original data elements"
    /*057*/ {TYP_ASC, 3, 100}, // "Authorization life cycle code"
    /*058*/ {TYP_ASC, 3, 100}, // "Authorizing agent institution Id Code"
    /*059*/ {TYP_ASC, 2, 999}, // "Transport data"
    /*060*/ {TYP_BCD, 3, 19}, // "Reserved for national use"
    /*061*/ {TYP_BCD, 3, 29}, // "Reserved for national use"
    /*062*/ {TYP_ASC, 3, 512}, // "Reserved for private use"
    /*063*/ {TYP_ASC, 3, 163}, // "Reserved for private use"
    /*064*/ {TYP_ASC, 0, 8} // "Message authentication code field"

 
};
ISO_DEF s_new_isodef[65] = {


    /*000*/ {TYP_ASC, 0, 4}, // "Message Type Indicator"
    /*001*/ {TYP_BIT, 0, 128}, // "Bitmap"
    /*002*/ {TYP_ASC, 2, 19}, // "Primary Account number"
    /*003*/ {TYP_ASC, 0, 6}, // "Processing Code"
    /*004*/ {TYP_ASC, 0, 12}, // "Amount, Transaction"
    /*005*/ {TYP_ASC, 0, 12}, // "Amount, Reconciliation"
    /*006*/ {TYP_ASC, 0, 12}, // "Amount, Cardholder billing"
    /*007*/ {TYP_ASC, 0, 10}, // "Date and time, transmission"
    /*008*/ {TYP_ASC, 0, 8}, // "Amount, Cardholder billing fee"
    /*009*/ {TYP_ASC, 0, 8}, // "Conversion rate, Reconciliation"
    /*010*/ {TYP_ASC, 0, 8}, // "Conversion rate, Cardholder billing"
    /*011*/ {TYP_ASC, 0, 6}, // "Systems trace audit number"
    /*012*/ {TYP_ASC, 0, 6}, // "Date and time, Local transaction"
    /*013*/ {TYP_ASC, 0, 4}, // "Date, Effective"
    /*014*/ {TYP_ASC, 0, 4}, // "Date, Expiration"
    /*015*/ {TYP_ASC, 0, 4}, // "Date, Settlement"
    /*016*/ {TYP_ASC, 0, 4}, // "Date, Conversion"
    /*017*/ {TYP_ASC, 0, 4}, // "Date, Capture"
    /*018*/ {TYP_ASC, 0, 4}, // "Merchant type"
    /*019*/ {TYP_ASC, 0, 3}, // "Country code, Acquiring institution"
    /*020*/ {TYP_ASC, 0, 3}, // "Country code, Primary account number"
    /*021*/ {TYP_ASC, 0, 3}, // "Country code, Forwarding institution"
    /*022*/ {TYP_ASC, 0, 3}, // "Point of service data code"
    /*023*/ {TYP_ASC, 0, 3}, // "Card sequence number"
    /*024*/ {TYP_ASC, 0, 3}, // "Function code"
    /*025*/ {TYP_ASC, 0, 2}, // "Message reason code"
    /*026*/ {TYP_ASC, 0, 2}, // "Card acceptor business code"
    /*027*/ {TYP_ASC, 0, 16}, // "Approval code length"
    /*028*/ {TYP_ASC, 0, 8}, // "Date, Reconciliation"
    /*029*/ {TYP_ASC, 0, 3}, // "Reconciliation indicator"
    /*030*/ {TYP_ASC, 0, 24}, // "Amounts, original"
    /*031*/ {TYP_ASC, 2, 99}, // "Acquirer reference data"
    /*032*/ {TYP_ASC, 2, 11}, // "Acquirer institution identification code"
    /*033*/ {TYP_ASC, 2, 11}, // "Forwarding institution identification code"
    /*034*/ {TYP_ASC, 2, 28}, // "Primary account number, extended"
    /*035*/ {TYP_ASC, 2, 37}, // "Track 2 data"
    /*036*/ {TYP_ASC, 3, 104}, // "Track 3 data"
    /*037*/ {TYP_ASC, 0, 12}, // "Retrieval reference number"
    /*038*/ {TYP_ASC, 0, 6}, // "Approval code"
    /*039*/ {TYP_ASC, 0, 2}, // "Action code"
    /*040*/ {TYP_ASC, 0, 3}, // "Service code"
    /*041*/ {TYP_ASC, 0, 8}, // "Card acceptor terminal identification"
    /*042*/ {TYP_ASC, 0, 15}, // "Card acceptor identification code"
    /*043*/ {TYP_ASC, 0, 40}, // "Card acceptor name/location"
    /*044*/ {TYP_ASC, 2, 25}, // "Additional response data"
    /*045*/ {TYP_ASC, 2, 79}, // "Track 1 data"
    /*046*/ {TYP_ASC, 0, 14}, // "Amounts, Fees"
    /*047*/ {TYP_ASC, 0, 18}, // "Additional data - national"
    /*048*/ {TYP_ASC, 3, 512}, // "Additional data - private"TYP_ASC
    /*049*/ {TYP_ASC, 0, 3}, // "Currency code, Transaction"
    /*050*/ {TYP_ASC, 0, 3}, // "Currency code, Reconciliation"
    /*051*/ {TYP_ASC, 0, 3}, // "Currency code, Cardholder billing"
    /*052*/ {TYP_ASC, 0, 8}, // "Personal identification number, PIN) data"
    /*053*/ {TYP_ASC, 0, 16}, // "Security related control information"
    /*054*/ {TYP_ASC, 3, 40}, // "Amounts, additional"
    /*055*/ {TYP_ASC, 3, 255}, // "IC card system related data"
    /*056*/ {TYP_ASC, 0, 6}, // "Original data elements"
    /*057*/ {TYP_ASC, 3, 900}, // "Authorization life cycle code"
    /*058*/ {TYP_ASC, 3, 100}, // "Authorizing agent institution Id Code"
    /*059*/ {TYP_ASC, 3, 600}, // "Transport data"
    /*060*/ {TYP_ASC, 3, 100}, // "Reserved for national use"
    /*061*/ {TYP_ASC, 3, 200}, // "Reserved for national use"
    /*062*/ {TYP_ASC, 3, 200}, // "Reserved for private use"
    /*063*/ {TYP_ASC, 3, 200}, // "Reserved for private use"
    /*064*/ {TYP_ASC, 0, 8} // "Message authentication code field"

 
};
#define MY_BITGET(p,n) ((p)[(n-1)/8]&(0x80>>((n-1)%8)))       //å–bitä½?
#define MY_BITSET(p,n) ((p)[(n-1)/8]|=(0x80>>((n-1)%8)))      //è®¾bitä½?
#define MY_BITCLR(p,n) ((p)[(n-1)/8]&=(~(0x80>>((n-1)%8))))   //æ¸…bitä½?
unsigned char GBuff[MAX_PACKET_LEN];
uchar MsgType[2];
uchar GBitMap[8];
int GPacketLen;
int  asc2hex(uchar *asc, uchar *hex, int pair_len)
{
    uchar src1,src2;
	int len;

	for (len=0; len < pair_len; len++)
	{
		src1 = *(asc+len*2);
		src2 = *(asc+len*2+1);
		if ((src1>='0') && (src1<='9'))
		    src1=src1-'0';
	       	else if ((src1>='A') && (src1<='F'))
		   src1=src1-'A'+10;
	        else if ((src1>='a') && (src1<='f'))
			src1=src1-'a'+10;
		else
			return -1;

		if ((src2>='0') && (src2<='9'))
		    src2=src2-'0';
	       	else if ((src2>='A') && (src2<='F'))
		   src2=src2-'A'+10;
	     	else if ((src2>='a') && (src2<='f'))
			src2=src2-'a'+10;
		else
			return -1;

		*hex++ = (src1 << 4) | src2;
	}
	return 0;
}

unsigned int bcd2int(uchar* s,int slen)
{
	unsigned int o;
	int i;
	if (slen>5)
		return 0;
	o=0;
	for(i=0;i<slen;i++)
		o=o*100+(s[i]/0x10)*10+(s[i]%0x10);
	return o;
}
void int2bcd(int inInt,uchar*BCDData,int NLen)
{
	uchar Tmp[20];
	uchar Fmt[20];

	sprintf((char*)Fmt,"%%0%dd",NLen);
	sprintf((char*)Tmp,(char*)Fmt,inInt);
	
	asc2hex(Tmp,BCDData,NLen/2);
	//Printf("PsamSequ tmp=%s inInt=%d\n",Tmp,inInt);
	//DebugBuff("PsamSequ BCDData",BCDData,NLen/2);
}

int   SetPacket(uchar *Buff,int Len)
{
	if(Len>MAX_PACKET_LEN)
		return -1;
	if(Len<10)
		return -1;

	memcpy(GBuff,Buff,Len);	
	memcpy(MsgType,GBuff,2);
	memcpy(GBitMap,GBuff+2,8);
	GPacketLen=Len;
	return 0;
}
int  GetPacket(uchar *Buff,int BuffLen)
{
	if(GPacketLen >BuffLen)
		return -1;
	memcpy(Buff,GBuff,GPacketLen);
	return GPacketLen;
}


int GetFieldLen(ISO_DEF isodef,int *HeadLen,uchar *Data)
{
	int DataLen;
	if(isodef.fmt==0)
	{
		*HeadLen=0;
		DataLen=isodef.len;
	}
	if(isodef.fmt==2)
	{
		*HeadLen=1;
		DataLen=bcd2int(Data,1);
	}
	if(isodef.fmt==3)
	{
		*HeadLen=2;
		DataLen=bcd2int(Data,2);
	}
	if(isodef.typ==TYP_BCD)
		DataLen=(DataLen+1)/2;
	else if (!(isodef.typ==TYP_ASC||isodef.typ==TYP_BIN))
		DataLen=0;
	return DataLen;
}
int   GetField(int FieldNum,uchar *Buff,int *DataLen,int *typ)
{
	int i,len,HeadLen;
	int offset;
	if (FieldNum>64)
		return -1;
	offset=0;
	for (i=0;i<FieldNum&&i<64;i++)
	{
		if(i==0)
		{
			offset+=2;
			continue;
		}
		if(i==1)
		{
			offset+=8;
			continue;
		}
		if (!MY_BITGET(GBitMap,i))
			continue;
		len=GetFieldLen(s_isodef[i],&HeadLen,(uchar*)GBuff+offset);
		if(len<0)
		{
			return -1;
		}
		offset+=HeadLen+len;
		if(offset>MAX_PACKET_LEN)
			return -2;
	}
	if(FieldNum==0)//MsgType
	{
		memcpy(Buff,GBuff,2);
		*DataLen=2;
		return 0;
	}
	if(FieldNum==1)//BitMap
	{
		memcpy(Buff,GBuff+2,8);
		*DataLen=8;
		return 0;
	}
	if(!MY_BITGET(GBitMap,FieldNum))
		return -3;//æ— æ­¤åŸ?
	len=GetFieldLen(s_isodef[FieldNum],&HeadLen,(uchar*)GBuff+offset);
	if(len<0)
	{
		return -1;
	}
	if(len>*DataLen)
		return -4;//ç”¨æˆ·åŒºæº¢å‡?
	memcpy(Buff,GBuff+offset+HeadLen,len);
	*DataLen=len;
	*typ=s_isodef[FieldNum].typ;
	return 0;
}
int  SetField(int FieldNum,uchar *Buff,int DataLen)
{
	uchar Head[3];
	int i,HeadLen,BuffLen,len2,FieldLen,offset=0,offset2;
	uchar TmpBuff[MAX_PACKET_LEN];
	uchar TmpField[1024];
	int DataOffset =0;
	
	if(FieldNum==0)
	{
		memcpy(GBuff,Buff,2);
		return 0;
	}
	if(FieldNum==1)//ä¸å…è®¸ç›´æŽ¥ä¿®æ”¹bitmap
		return -1;

	BuffLen = DataLen;
	
	if(s_isodef[FieldNum].typ==TYP_BCD)
		BuffLen=(DataLen+1)/2;

	
		
	
	if(DataLen>s_isodef[FieldNum].len)//è¾“å…¥é•¿åº¦å¤§äºŽè§„å®šé•¿åº¦
	{
		//Printf("Data Len ERR FieldNum=%i DataLen=%i MustLen=%i\n",FieldNum,DataLen,s_isodef[FieldNum].len);
		return -5;//æ•°æ®é•¿åº¦é”™è¯¯
	}
	memset(TmpField,0x00,sizeof(TmpField));
	if(s_isodef[FieldNum].fmt==0)
	{
		HeadLen=0;
		if(s_isodef[FieldNum].typ==TYP_BCD)
			DataOffset = (s_isodef[FieldNum].len+1)/2 - BuffLen;
		else
			DataOffset = s_isodef[FieldNum].len - BuffLen;
		memcpy(TmpField+DataOffset,Buff,BuffLen);
		if(s_isodef[FieldNum].typ==TYP_BCD)
			BuffLen=(s_isodef[FieldNum].len+1) / 2;
		else
			BuffLen=s_isodef[FieldNum].len;
		/*if(len!=s_isodef[FieldNum].len)
			return -5;//æ•°æ®é•¿åº¦é”™è¯¯*/
	}
	else if(s_isodef[FieldNum].fmt==2)
	{
		int2bcd(DataLen,TmpField,2);
		HeadLen=1;
		memcpy(TmpField+HeadLen,Buff,BuffLen);
	}
	else if(s_isodef[FieldNum].fmt==3)
	{
		int2bcd(DataLen,TmpField,4);
		HeadLen=2;
		memcpy(TmpField+HeadLen,Buff,BuffLen);
	}

	
	FieldLen=BuffLen+HeadLen;
	offset=0;
	for(i=0;i<FieldNum;i++)
	{
		if(i==0)
		{
			offset+=2;
			continue;
		}
		if(i==1)
		{
			offset+=8;
			continue;
		}
		if (!MY_BITGET(GBitMap,i))
			continue;
		len2=GetFieldLen(s_isodef[i],&HeadLen,(uchar*)GBuff+offset);
		if(len2<=0)
		{
			//Printf("eee %i\n",len2);
			return -1;
		}
		offset+=HeadLen+len2;
		if(offset>GPacketLen)
		{
			//Printf("fffff %i  %i\n",offset,GPacketLen);
			return -2;
		}
	}
	if(MY_BITGET(GBitMap,FieldNum))//åˆ é™¤åŽŸæ¥çš„é‚£ä¸ªåŸŸ
	{
		len2=GetFieldLen(s_isodef[FieldNum],&HeadLen,(uchar*)GBuff+offset);
		GPacketLen-=(len2+HeadLen);
		memcpy(GBuff+offset,GBuff+offset+len2+HeadLen,GPacketLen-offset);
	}
	memcpy(TmpBuff,GBuff+offset,GPacketLen-offset);
	memcpy(GBuff+offset,TmpField,FieldLen);
	memcpy(GBuff+offset+FieldLen,TmpBuff,GPacketLen-offset);
	MY_BITSET(GBitMap,FieldNum);
	memcpy(GBuff+2,GBitMap,8);
	//Printf("GPacketLen %i  FieldLen %i\n",GPacketLen,FieldLen);
	GPacketLen+=FieldLen;
	
	return 0;
}
int  DelField(int FieldNum)
{
	uchar Head[3];
	int i,HeadLen,BuffLen,len2,FieldLen,offset=0,offset2;
	uchar TmpBuff[MAX_PACKET_LEN];
	uchar TmpField[1024];
	int DataOffset =0;
	
	if(FieldNum==0)
	{
		return -1;
	}
	if(FieldNum==1)
		return -1;

	if (!MY_BITGET(GBitMap,FieldNum))
		return 0;


	offset=0;
	for(i=0;i<FieldNum;i++)
	{
		if(i==0)
		{
			offset+=2;
			continue;
		}
		if(i==1)
		{
			offset+=8;
			continue;
		}
		if (!MY_BITGET(GBitMap,i))
			continue;
		len2=GetFieldLen(s_isodef[i],&HeadLen,(uchar*)GBuff+offset);
		if(len2<=0)
		{
			//Printf("eee %i\n",len2);
			return -1;
		}
		offset+=HeadLen+len2;
		if(offset>GPacketLen)
		{
			//Printf("fffff %i  %i\n",offset,GPacketLen);
			return -2;
		}
	}
	if(MY_BITGET(GBitMap,FieldNum))//åˆ é™¤åŽŸæ¥çš„é‚£ä¸ªåŸŸ
	{
		len2=GetFieldLen(s_isodef[FieldNum],&HeadLen,(uchar*)GBuff+offset);
		GPacketLen-=(len2+HeadLen);
		memcpy(GBuff+offset,GBuff+offset+len2+HeadLen,GPacketLen-offset);
	}

	MY_BITCLR(GBitMap,FieldNum);
	memcpy(GBuff+2,GBitMap,8);
	
	return 0;
}


void Print8583Packet(uchar *msg, uchar *hexBuff, int BuffLen)
{
	int Ret,i,j;
	int FieldLen,Fieldtype;
	uchar tmpBuff[2048];
	char str[2048];
	char sstr[10];
	HtLog("packet.log", HT_LOG_MODE_ERROR, __FILE__,__LINE__, "%s\n",msg); 
	if(strcmp(msg,"1706sendpacket")==0 ||strcmp(msg,"1706Recvpacket")==0)
	{
		s_isodef[35].typ=TYP_ASC;
		s_isodef[36].typ=TYP_ASC;
		s_isodef[60].typ=TYP_ASC;
		s_isodef[61].typ=TYP_ASC;
		s_isodef[54].typ=TYP_BCD;
		s_isodef[54].fmt=2;
	}
	else if(strcmp(msg,"1708sendpacket")==0 ||strcmp(msg,"1708Recvpacket")==0)
	{
		s_isodef[57].typ=TYP_ASC;
		s_isodef[57].fmt=3;
		s_isodef[57].len=256;
	}
	else if(strcmp(msg,"1727sendpacket")==0 ||strcmp(msg,"1727Recvpacket")==0)
	{
		s_isodef[62].typ=TYP_BCD;
		s_isodef[61].typ=TYP_ASC;
	}
	else
	{
		s_isodef[35].typ=TYP_BCD;
		s_isodef[36].typ=TYP_BCD;
		s_isodef[60].typ=TYP_BCD;
		s_isodef[61].typ=TYP_BCD;
		s_isodef[54].typ=TYP_ASC;
		s_isodef[54].fmt=3;
	}	
	memset(str,0,2048);
	for(i=0;i<BuffLen;i++)
	{
		sprintf(sstr,"%0.2X",hexBuff[i]);
		strcpy(str+strlen(str),sstr);
	}
	HtLog("packet.log", HT_LOG_MODE_ERROR, __FILE__,__LINE__, "%s",str);
	Ret=SetPacket(&hexBuff[MagPacketHeaderLen],BuffLen-MagPacketHeaderLen);
	if(Ret!=0)
	{
		HtLog("packet.log", HT_LOG_MODE_ERROR, __FILE__,__LINE__, "%s packet err\n",msg);
		return; 	
	}
	HtDebugString("packet.log", HT_LOG_MODE_DEBUG, __FILE__, __LINE__, hexBuff, BuffLen);

	memset(str,0,2048);
	for(i=0; i<MagPacketHeaderLen; i++)
	{
		sprintf(sstr,"%0.2X ",hexBuff[i]);
		strcpy(str+strlen(str),sstr);
	}
	HtLog("packet.log", HT_LOG_MODE_ERROR, __FILE__,__LINE__, "Head   %s",str);
	memset(str,0,2048);

	for(i=0; i<=64; i++)
	{
		FieldLen =2048;
		if(GetField(i,tmpBuff,&FieldLen,&Fieldtype)>=0)
		{
			if(FieldLen==0)
				sprintf(str,"F%0.2d     ",i);
			else
			{
				sprintf(str,"F%0.2d     ",i);
				for(j=0; j<FieldLen; j++)
					sprintf(str+strlen(str),"%0.2X ",tmpBuff[j]);
			}
			HtLog("packet.log", HT_LOG_MODE_ERROR, __FILE__,__LINE__, "%s",str);

		}
	}
}

int MakeRetrunPacket(uchar *msg, uchar *RecvBuff, int RecvBuffLen,uchar *SendBuff,int *SendBuffLen)
{
	int Ret,i,j;
	int FieldLen,Fieldtype;
	char str[2048];
	char sstr[10];
	uchar field[65][1000];
	int fieldlen[65];
	
	char    sCurrentTime[15];
	int seq;
	
	memset(field,0,65*1000);
	memset(fieldlen,0,65);
	Ret=SetPacket(&RecvBuff[MagPacketHeaderLen],RecvBuffLen-MagPacketHeaderLen);
	if(Ret!=0)
	{
		HtLog("packet.log", HT_LOG_MODE_ERROR, __FILE__,__LINE__, "%s packet err\n",msg);
		return; 	
	}
	HtLog("packet.log", HT_LOG_MODE_ERROR, __FILE__,__LINE__, "\n%s\nHead    ",msg);
	memset(str,0,2048);
	for(i=0; i<MagPacketHeaderLen; i++)
	{
		sprintf(sstr,"%0.2X ",RecvBuff[i]);
		strcpy(str+strlen(str),sstr);
	}
	HtLog("packet.log", HT_LOG_MODE_ERROR, __FILE__,__LINE__, "%s",str);
	memset(str,0,2048);

	for(i=0; i<=64; i++)
	{
		FieldLen =2048;
		if(GetField(i,field[i],&FieldLen,&Fieldtype)>=0)
		{
			fieldlen[i]=FieldLen;
			if(fieldlen[i]==0)
				sprintf(str,"F%0.2d  ",i);
			else
			{
				sprintf(str,"F%0.2d  %0.4d   ",i,fieldlen[i]);
				for(j=0; j<fieldlen[i]; j++)
					sprintf(str+strlen(str),"%0.2X ",field[i][j]);
			}
			HtLog("packet.log", HT_LOG_MODE_ERROR, __FILE__,__LINE__, "%s",str);

		}
	}
	memcpy(SendBuff,"\x60\x00\x00\x00\x14\x60\x31\x00\x31\x30\x01",11);
	CommonGetCurrentTime (sCurrentTime);
	HtLog("packet.log", HT_LOG_MODE_ERROR, __FILE__,__LINE__, "%s",sCurrentTime);
	asc2hex(sCurrentTime+8,field[12],3);
	asc2hex(sCurrentTime+4,field[13],2);
	//memcpy(field[12],"\x15\x58\x32",3);
	//memcpy(field[13],"\x01\x24",2);
	memcpy(field[32],"\x12\x34\x56\x54\x32\x10",6);
	seq = bcd2int(field[11],3);
	sprintf(field[37],"%0.12d",seq);
	memcpy(field[39],"00",2);
	
	if(memcmp(field[0],"\x08\x00",2)==0 && memcmp(field[60],"\x00",1)==0 && memcmp(field[60]+4,"\x00\x30",2)==0)
	{
		//Ç©µ½
		SetPacket((uchar *)"\x08\x10\x00\x00\x00\x00\x00\x00\x00\x00",10);//1¡¢2Óò
		SetField(11,field[11],6);
		SetField(12,field[12],6);
		SetField(13,field[13],4);
		SetField(32,field[32],11);
		SetField(37,field[37],12);
		SetField(39,field[39],2);
		SetField(41,field[41],8);
		SetField(42,field[42],15);
		SetField(60,"\x00\x00\x00\x01\x00\x30",11);
		SetField(62,"\x0B\xA9\x2D\x4B\x18\xBA\x96\xC2\xD2\x5F\x01\x95\xE1\xD4\xDB\x93\xD7\x14\xC8\x4D\xB9\xF1\x7E\x99\x28\x1D\x73\x50\x00\x00\x00\x00\x00\x00\x00\x00\x8C\x65\x0B\xFD",40);
	}
	else if(memcmp(field[0],"\x02\x00",2)==0 && memcmp(field[3],"\x31",1)==0)
	{
		//Óà¶î
		SetPacket((uchar *)"\x02\x10\x00\x00\x00\x00\x00\x00\x00\x00",10);//1¡¢2Óò
		SetField(3,field[3],6);

		SetField(11,field[11],6);
		SetField(12,field[12],6);
		SetField(13,field[13],4);
		SetField(32,field[32],11);
		SetField(37,field[37],12);
		SetField(39,field[39],2);
		SetField(41,field[41],8);
		SetField(42,field[42],15);
		SetField(44,"\x31\x32\x33\x34\x35\x36\x35\x34\x33\x32\x31",11);
		SetField(54,"\x31\x30\x30\x32\x31\x35\x36\x43\x31\x30\x30\x30\x30\x30\x31\x30\x30\x30\x30\x31",20);
		SetField(60,"\x00\x00\x00\x00\x00\x00",11);
		SetField(64,"\x00\x00\x00\x00\x00\x00\x00\x00",8);
	}
	else if(memcmp(field[0],"\x02\x00",2)==0 && memcmp(field[3],"\x00",1)==0 && memcmp(field[25],"\x00",1)==0)
	{
		//Ïû·Ñ
		SetPacket((uchar *)"\x02\x10\x00\x00\x00\x00\x00\x00\x00\x00",10);//1¡¢2Óò
		if(fieldlen[2]==8)
			SetField(2,field[2],16);
		else if(fieldlen[2]==9)
			SetField(2,field[2],18);
		else if(fieldlen[2]==10)
			SetField(2,field[2],19);
		else 
			SetField(2,field[35],16);
		SetField(3,field[3],6);
		SetField(4,field[4],12);
		SetField(11,field[11],6);
		SetField(12,field[12],6);
		SetField(13,field[13],4);
		SetField(15,field[13],4);
		SetField(25,field[25],2);
		SetField(32,field[32],11);
		SetField(37,field[37],12);
		SetField(39,field[39],2);
		SetField(41,field[41],8);
		SetField(42,field[42],15);
		SetField(44,"\x30\x31\x30\x35\x30\x30\x30\x31\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20",22);
		SetField(49,"\x31\x35\x36",3);
		SetField(53,"\x26\x00\x00\x00\x00\x00\x00\x00",16);
		SetField(60,"\x22\x00\x00\x01\x00\x00",11);
		SetField(63,"\x43\x55\x50",3);
		SetField(64,"\x00\x00\x00\x00\x00\x00\x00\x00",8);
	}
	else if(memcmp(field[0],"\x02\x00",2)==0 && memcmp(field[3],"\x20",1)==0 && memcmp(field[25],"\x00",1)==0)
	{
		//Ïû·Ñ³·Ïû
		SetPacket((uchar *)"\x02\x10\x00\x00\x00\x00\x00\x00\x00\x00",10);//1¡¢2Óò
		if(fieldlen[2]==8)
			SetField(2,field[2],16);
		else if(fieldlen[2]==9)
			SetField(2,field[2],18);
		else if(fieldlen[2]==10)
			SetField(2,field[2],19);
		else 
			SetField(2,field[35],16);
		SetField(3,field[3],6);
		SetField(4,field[4],12);
		SetField(11,field[11],6);
		SetField(12,field[12],6);
		SetField(13,field[13],4);
		SetField(15,field[13],4);
		SetField(25,field[25],2);
		SetField(32,field[13],4);
		SetField(37,field[37],12);
		SetField(38,"\x31\x32\x33\x34\x35\x36",6);
		SetField(39,field[39],2);
		SetField(41,field[41],8);
		SetField(42,field[42],15);
		SetField(44,"\x30\x31\x30\x35\x30\x30\x30\x31\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20",22);
		SetField(49,"\x31\x35\x36",3);
		SetField(53,"\x26\x00\x00\x00\x00\x00\x00\x00",16);
		SetField(60,"\x23\x00\x00\x01\x00\x00",11);
		SetField(63,"\x43\x55\x50",3);
		SetField(64,"\x00\x00\x00\x00\x00\x00\x00\x00",8);
	}
	else if(memcmp(field[0],"\x02\x20",2)==0 && memcmp(field[3],"\x20",1)==0)
	{
		//Ïû·Ñ³·Ïû
		SetPacket((uchar *)"\x02\x30\x00\x00\x00\x00\x00\x00\x00\x00",10);//1¡¢2Óò
		if(fieldlen[2]==8)
			SetField(2,field[2],16);
		else if(fieldlen[2]==9)
			SetField(2,field[2],18);
		else if(fieldlen[2]==10)
			SetField(2,field[2],19);
		else 
			SetField(2,field[35],16);
		SetField(3,field[3],6);
		SetField(4,field[4],12);
		SetField(11,field[11],6);
		SetField(12,field[12],6);
		SetField(13,field[13],4);
		SetField(15,field[13],4);
		SetField(25,field[25],2);
		SetField(32,field[13],4);
		SetField(37,field[37],12);
		SetField(38,"\x31\x32\x33\x34\x35\x36",6);
		SetField(39,field[39],2);
		SetField(41,field[41],8);
		SetField(42,field[42],15);
		SetField(44,"\x30\x31\x30\x35\x30\x30\x30\x31\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20",22);
		SetField(49,"\x31\x35\x36",3);
		SetField(53,"\x26\x00\x00\x00\x00\x00\x00\x00",16);
		SetField(60,"\x25\x00\x00\x01\x00\x00",11);
		SetField(63,"\x43\x55\x50",3);
		SetField(64,"\x00\x00\x00\x00\x00\x00\x00\x00",8);
	}
	else
		return -1;
	*SendBuffLen=GetPacket(&SendBuff[11],1024);
	*SendBuffLen=*SendBuffLen+11;

	//SendBuff[0] = (*SendBuffLen)/256;
	//SendBuff[1] = (*SendBuffLen)%256;
	Print8583Packet("SendPacket",SendBuff-2,*SendBuffLen+2);
	return 0;
}

void AddFld56(uchar *hexBuff, int BuffLen, int *SendLen)
{
	int Ret,i,j;
	int FieldLen,Fieldtype;
	uchar tmpBuff[2048];
	char str[2048];
	char sstr[10];
	HtLog("packet.log", HT_LOG_MODE_ERROR, __FILE__,__LINE__, "RRF Add FLD 56\n"); 
	
	{
		s_isodef[35].typ=TYP_ASC;
		s_isodef[36].typ=TYP_ASC;
		s_isodef[60].typ=TYP_ASC;
		s_isodef[61].typ=TYP_ASC;
		s_isodef[54].typ=TYP_BCD;
		s_isodef[54].fmt=2;
	}

	memset(str,0,2048);

	Ret=SetPacket(&hexBuff[MagPacketHeaderLen],BuffLen-MagPacketHeaderLen);
	if(Ret!=0)
	{
		HtLog("packet.log", HT_LOG_MODE_ERROR, __FILE__,__LINE__, "packet err\n");
		return; 	
	}

	
	FieldLen =2048;
	GetField(60,tmpBuff,&FieldLen,&Fieldtype);
	asc2hex(tmpBuff+2,sstr,3);
	SetField(56,sstr,6);
	*SendLen=GetPacket(&hexBuff[MagPacketHeaderLen],4096);
	*SendLen=*SendLen+MagPacketHeaderLen;

	
	
}
void DelFld56(uchar *hexBuff, int BuffLen, int *SendLen)
{
	
	int Ret,i,j;
	int FieldLen,Fieldtype;
	uchar tmpBuff[2048];
	char str[2048];
	char sstr[10];
	HtLog("packet.log", HT_LOG_MODE_ERROR, __FILE__,__LINE__, "RRF Del FLD 56\n"); 
	
	{
		s_isodef[35].typ=TYP_ASC;
		s_isodef[36].typ=TYP_ASC;
		s_isodef[60].typ=TYP_ASC;
		s_isodef[61].typ=TYP_ASC;
		s_isodef[54].typ=TYP_BCD;
		s_isodef[54].fmt=2;
	}

	memset(str,0,2048);

	Ret=SetPacket(&hexBuff[MagPacketHeaderLen],BuffLen-MagPacketHeaderLen);
	if(Ret!=0)
	{
		HtLog("packet.log", HT_LOG_MODE_ERROR, __FILE__,__LINE__, "packet err\n");
		return; 	
	}


	DelField(56);
	*SendLen=GetPacket(&hexBuff[MagPacketHeaderLen],4096);
	*SendLen=*SendLen+MagPacketHeaderLen;


}

void AddFld57(uchar *hexBuff, int BuffLen, int *SendLen)
{
	int Ret,i,j;
	int FieldLen,Fieldtype;
	//uchar tmpBuff[2048];
	char str[100];
	//char sstr[10];
	HtLog("packet.log", HT_LOG_MODE_ERROR, __FILE__,__LINE__, "RRF Add FLD 57\n"); 
	s_isodef[57].typ=TYP_ASC;
	s_isodef[57].fmt=3;
    s_isodef[57].len=256;

	memset(str,0,100);
	/*for(i=0;i<BuffLen;i++)
	{
		sprintf(sstr,"%0.2X",hexBuff[i]);
		strcpy(str+strlen(str),sstr);
	}
	HtLog("packet.log", HT_LOG_MODE_ERROR, __FILE__,__LINE__, "%s\n",str);*/
	Ret=SetPacket(&hexBuff[MagPacketHeaderLen],BuffLen-MagPacketHeaderLen);
	if(Ret!=0)
	{
		HtLog("packet.log", HT_LOG_MODE_ERROR, __FILE__,__LINE__, "packet err\n");
		return; 	
	}
	//HtDebugString("packet.log", HT_LOG_MODE_DEBUG, __FILE__, __LINE__, hexBuff, BuffLen);

	
//	FieldLen =2048;
//	GetField(60,tmpBuff,&FieldLen,&Fieldtype);
//	asc2hex(tmpBuff+2,sstr,3);
    memset(str, 0, sizeof(str));
	memcpy(str,"\x30\x30\x56\x32\x30\x31\x34\x30\x37\x32\x35\x30\x31\x20\x20\x20\x20\x20\x20\x20\x20\x20\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x35\x44\x45\x43\x39\x38\x30\x42\x43\x42\x43\x32\x42\x36\x44\x31\x31\x39\x46\x38\x42\x34\x46\x37\x34\x37\x38\x45\x39\x32\x37\x44",64);
	SetField(57,str,64);


	

	*SendLen=GetPacket(&hexBuff[MagPacketHeaderLen],4096);
	
	*SendLen=*SendLen+MagPacketHeaderLen;


	
	
	
}

void DelFld62(uchar *hexBuff, int BuffLen, int *SendLen)
{
	
	int Ret;


	HtLog("packet.log", HT_LOG_MODE_ERROR, __FILE__,__LINE__, "RRF Del FLD 62\n"); 
	


	Ret=SetPacket(&hexBuff[MagPacketHeaderLen],BuffLen-MagPacketHeaderLen);
	if(Ret!=0)
	{
		HtLog("packet.log", HT_LOG_MODE_ERROR, __FILE__,__LINE__, "packet err\n");
		return; 	
	}


	DelField(62);
	*SendLen=GetPacket(&hexBuff[MagPacketHeaderLen],4096);
	
	*SendLen=*SendLen+MagPacketHeaderLen;


}
// add at convert 64 bitmap message to 128 bitmap message
int Convert64To128(uchar *hexBuff, uchar *sendmsg, int BuffLen int* ConvertLen )
{
	int Ret,i,j;
	int FieldLen,Fieldtype;
	uchar tmpBuff[2048];
	char str[2048];
	char sstr[10];
	int  MsgPacketHeaderLen = 46;
	uchar ascii_buf[2048];
	uchar sField63[1024];
	int  nField63Len; 
	int offset;
	HtLog("packet.log", HT_LOG_MODE_ERROR, __FILE__,__LINE__, "%s\n",msg); 
	
	memset(str,0,2048);
	for(i=0;i<BuffLen;i++)
	{
		sprintf(sstr,"%0.2X",hexBuff[i]);
		strcpy(str+strlen(str),sstr);
	}
	HtLog("packet.log", HT_LOG_MODE_ERROR, __FILE__,__LINE__, "%s",str);
	Ret=SetPacket(&hexBuff[MsgPacketHeaderLen],BuffLen-MsgPacketHeaderLen);
	if(Ret!=0)
	{
		HtLog("packet.log", HT_LOG_MODE_ERROR, __FILE__,__LINE__, "%s packet err\n",msg);
		return; 	
	}
	HtDebugString("packet.log", HT_LOG_MODE_DEBUG, __FILE__, __LINE__, hexBuff, BuffLen);

	memset(str,0,2048);
	for(i=0; i<MsgPacketHeaderLen; i++)
	{
		sprintf(sstr,"%0.2X ",hexBuff[i]);
		strcpy(str+strlen(str),sstr);
	}
	HtLog("packet.log", HT_LOG_MODE_ERROR, __FILE__,__LINE__, "Head   %s",str);
	memset(str,0,2048);
    offset=0;
	for(i=0; i<=64; i++)
	{
		FieldLen =2048;
		if(GetField_Length(i,tmpBuff,&FieldLen,&Fieldtype)>=0)
		{
			if(FieldLen==0)
				sprintf(str,"F%0.2d     ",i);
			else
			{
				sprintf(str,"F%0.2d     ",i);
				for(j=0; j<FieldLen; j++)
					sprintf(str+strlen(str),"%0.2X ",tmpBuff[j]);

				if( s_isodef[i].fmt == 0)
				{
					HeadLen=0;
					DataLen=s_isodef[i].len;
				}
				if ( s_isodef[i].fmt == 2)
				{
					HeadLen=1;
					DataLen=bcd2int(tmpBuff,1);

				}
				if (s_isodef[i].fmt == 3)
				{
					HeadLen=2;
					DataLen=bcd2int(tmpBuff, 2);
				}

                if( i == 0 )
                {
                	bcd_to_asc(ascii_buf+offset, tmpBuff, 4, 0);
                	HeadLen = 0;
                }
                else if( i == 1)
                {
                	memcpy(ascii_buf+offset, tmpBuff, 8);                	
                	memcpy(ascii_buf+offset+8, "\x00\x00\x00\x00\x00\x00\x00\x00",8);
                	offset=offset+8;
                	HeadLen=0;
                }
                else if(s_isodef[i].fmt == 0 && Fieldtype == TYP_BCD && s_new_isodef[i].typ == TYP_ASC)
                {
                	// 55 field lenght=145  0x01 0x45  convert 0x31 0x34 0x35  type=1
                	HeadLen=0; 
                	bcd_to_asc(ascii_buf+offset, tmpBuff, DataLen, 0);
                	//bcd_to_asc(ascii_buf, tmpBuff, 3, 1);
                	//memcpy(ascii_buf+3, tmpBuff+2, DataLen);
                }
                else if(s_isodef[i].fmt == 2 && Fieldtype == TYP_BCD && s_new_isodef[i].typ == TYP_ASC)
                {
                   // example card length 0x19  convert  0x30 0x39   HeadLen 1 -> 2 
                   HeadLen=2;
				   bcd_to_asc(ascii_buf+offset, tmpBuff, HeadLen+DataLen, 0);
                }
                else if(s_isodef[i].fmt == 2 && Fieldtype == TYP_BCD && s_new_isodef[i].typ == TYP_ASC)
                {
                	// 55 field lenght=145  0x01 0x45  convert 0x31 0x34 0x35  type=1 drop off first 0x30 
                	// 2 bytes ---> 3 bytes  
                	HeadLen=3;
                	bcd_to_asc(ascii_buf+offset, tmpBuff, HeadLen+DataLen, 1);
                }
                else if(s_isodef[i].fmt == 0 && Fieldtype == TYP_ASC && s_new_isodef[i].typ == TYP_ASC)
                {
                    HeadLen=0;
                    memcpy(ascii_buf+offset, tmpBuff, DataLen);
                }
                else if(s_isodef[i].fmt == 2 && Fieldtype == TYP_ASC && s_new_isodef[i].typ == TYP_ASC)
                {
                    HeadLen=2;
                    bcd_to_asc(ascii_buf+offset, tmpBuff, HeadLen,0);
                    memcpy(ascii_buf+offset+HeadLen, tmpBuff+1, DataLen);

                }
                else if(s_isodef[i].fmt == 3 && Fieldtype == TYP_ASC && s_new_isodef[i].typ == TYP_ASC)
                {
                	HeadLen=3;
                	bcd_to_asc(ascii_buf+offset, tmpBuff, HeadLen, 1);
                	memcpy(ascii_buf+offset+HeadLen, tmpBuff+2, DataLen);
                }	
                else
                {
                	memcpy(ascii_buf+offset, tmpBuff, DataLen+HeadLen);
                }
                //memcpy(ascii_buf+offset, tmpBuff, DataLen+HeadLen*2);
                if( i == 63)
                {
                	memcpy(sField63, ascii_buf+offset, HeadLen+DataLen);
                    nField63Len=DataLen;
                }

                offset=HeadLen+DataLen;  

			}
			HtLog("packet.log", HT_LOG_MODE_ERROR, __FILE__,__LINE__, "%s",str);
			HtLog("packet.log", HT_LOG_MODE_ERROR, __FILE__, __LINE__, "%s", ascii_buf)

		}
	}
	HtLog("packet.log", HT_LOG_MODE_ERROR, __FILE__,__LINE__, "sField63   %s",sField63);
	// default 128 message length=0x30 0x30 0x30 0x30
	memcpy(sendmsg,"0000", 4);
	memcpy(sendmsg+4, hexBuff+2, MsgPacketHeaderLen);
	memcpy(sendmsg+4+MsgPacketHeaderLen, ascii_buf,offset);
	ConvertLen = 4+MsgPacketHeaderLen+offset;
	HtDebugString("packet.log", HT_LOG_MODE_DEBUG, __FILE__, __LINE__, sendmsg, 4+MsgPacketHeaderLen+offset);
    // next trouble is add data between 64 to 128  field to message 
    // data stores in field 63
    

    ConvertLen=MsgPacketHeaderLen+offset;
    return 0;
}
int Convert128To64(char sNetMsgBuf[], int* nMsgLen)
{

	return 0;
}
int   GetField_Length(int FieldNum,uchar *Buff,int *DataLen,int *typ)
{
	int i,len,HeadLen;
	int offset;
	if (FieldNum>64)
		return -1;
	offset=0;
	for (i=0;i<FieldNum&&i<64;i++)
	{
		if(i==0)
		{
			offset+=2;
			continue;
		}
		if(i==1)
		{
			offset+=8;
			continue;
		}
		if (!MY_BITGET(GBitMap,i))
			continue;
		len=GetFieldLen(s_isodef[i],&HeadLen,(uchar*)GBuff+offset);
		if(len<=0)
		{
			return -1;
		}
		offset+=HeadLen+len;
		if(offset>MAX_PACKET_LEN)
			return -2;
	}
	if(FieldNum==0)//MsgType
	{
		memcpy(Buff,GBuff,2);
		*DataLen=2;
		return 0;
	}
	if(FieldNum==1)//BitMap
	{
		memcpy(Buff,GBuff+2,8);
		*DataLen=8;
		return 0;
	}
	if(!MY_BITGET(GBitMap,FieldNum))
		return -3;//无此域
	len=GetFieldLen(s_isodef[FieldNum],&HeadLen,(uchar*)GBuff+offset);
	if(len<=0)
	{
		return -1;
	}
	if(len>*DataLen)
		return -4;//用户区溢出
	memcpy(Buff,GBuff+offset,len);
	*DataLen=len;
	*typ=s_isodef[FieldNum].typ;
	return 0;
}