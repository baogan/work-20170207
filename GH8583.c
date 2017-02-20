#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "GH8583.h"
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
    /*057*/ {TYP_BCD, 0, 3}, // "Authorization life cycle code"
    /*058*/ {TYP_ASC, 3, 100}, // "Authorizing agent institution Id Code"
    /*059*/ {TYP_ASC, 2, 999}, // "Transport data"
    /*060*/ {TYP_BCD, 3, 19}, // "Reserved for national use"
    /*061*/ {TYP_BCD, 3, 29}, // "Reserved for national use"
    /*062*/ {TYP_ASC, 3, 512}, // "Reserved for private use"
    /*063*/ {TYP_ASC, 3, 163}, // "Reserved for private use"
    /*064*/ {TYP_ASC, 0, 8} // "Message authentication code field"

 
};

ISO_DEF s_new_isodef[129] = {


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
    /*064*/ {TYP_ASC, 0, 8}, // "Message authentication code field"
    /*065*/ {TYP_ASC, 0, 2},
    /*66*/  {TYP_ASC, 0, 2},
    /*67*/  {TYP_ASC, 0, 2},
    /*68*/  {TYP_ASC, 0, 2},
    /*69*/  {TYP_ASC, 0, 2},
    /*70*/  {TYP_ASC, 0, 3},   // n3 Network Management Information Code
    /*71*/  {TYP_ASC, 0, 2},
    /*72*/  {TYP_ASC, 0, 2},
    /*73*/  {TYP_ASC, 0, 2},
    /*74*/  {TYP_ASC, 0, 2},
    /*75*/  {TYP_ASC, 0, 2},
    /*76*/  {TYP_ASC, 0, 2},
    /*77*/  {TYP_ASC, 0, 2},
    /*78*/  {TYP_ASC, 0, 2},
    /*79*/  {TYP_ASC, 0, 2},
    /*80*/  {TYP_ASC, 0, 2},
    /*81*/  {TYP_ASC, 0, 2},
    /*82*/  {TYP_ASC, 0, 2},
    /*83*/  {TYP_ASC, 0, 2},
    /*84*/  {TYP_ASC, 0, 2},
    /*85*/  {TYP_ASC, 0, 2},
    /*86*/  {TYP_ASC, 0, 2},
    /*87*/  {TYP_ASC, 0, 2},
    /*88*/  {TYP_ASC, 0, 2},
    /*89*/  {TYP_ASC, 0, 2},
    /*90*/  {TYP_ASC, 0, 42},  // n42 Original Data Elements
    /*91*/  {TYP_ASC, 0, 2},
    /*92*/  {TYP_ASC, 0, 2},
    /*93*/  {TYP_ASC, 0, 2},
    /*94*/  {TYP_ASC, 0, 2},
    /*95*/  {TYP_ASC, 0, 2},
    /*96*/  {TYP_ASC, 0, 8},  // 64bit Message Security Code
    /*97*/  {TYP_ASC, 0, 2},
    /*98*/  {TYP_ASC, 0, 2},
    /*99*/  {TYP_ASC, 0, 2},
    /*100*/ {TYP_ASC, 2, 11}, //  institution identification code
    /*101*/ {TYP_ASC, 0, 2},
    /*102*/ {TYP_ASC, 2, 28}, // Account Identification 1
    /*103*/ {TYP_ASC, 2, 28},  // Account Identification 2
    /*104*/ {TYP_ASC, 0, 2},
    /*105*/ {TYP_ASC, 0, 2},
    /*106*/ {TYP_ASC, 0, 2},
    /*107*/ {TYP_ASC, 0, 2},
    /*108*/ {TYP_ASC, 0, 2},
    /*109*/ {TYP_ASC, 0, 2},
    /*110*/ {TYP_ASC, 0, 2},
    /*111*/ {TYP_ASC, 0, 2},
    /*112*/ {TYP_ASC, 0, 2},
    /*113*/ {TYP_ASC, 0, 2},
    /*114*/ {TYP_ASC, 0, 2},
    /*115*/ {TYP_ASC, 0, 2},
    /*116*/ {TYP_ASC, 0, 2},
    /*117*/ {TYP_ASC, 0, 2},
    /*118*/ {TYP_ASC, 0, 2},
    /*119*/ {TYP_ASC, 0, 2},
    /*120*/ {TYP_ASC, 0, 2},
    /*121*/ {TYP_ASC, 3, 100},  //CUPS Reserved
    /*122*/ {TYP_ASC, 3, 100},  //Acquiring Institution Reserved  
    /*123*/ {TYP_ASC, 3, 100},  //Issuer Institution Reserved
    /*124*/ {TYP_ASC, 0, 2},
    /*125*/ {TYP_ASC, 0, 2},
    /*126*/ {TYP_ASC, 0, 2},
    /*127*/ {TYP_ASC, 0, 2},
    /*128*/ {TYP_ASC, 0, 8}    // mac 

};
#define MY_BITGET(p,n) ((p)[(n-1)/8]&(0x80>>((n-1)%8)))       //Ã¥Ââ€“bitÃ¤Â½?
#define MY_BITSET(p,n) ((p)[(n-1)/8]|=(0x80>>((n-1)%8)))      //Ã¨Â®Â¾bitÃ¤Â½?
#define MY_BITCLR(p,n) ((p)[(n-1)/8]&=(~(0x80>>((n-1)%8))))   //Ã¦Â¸â€¦bitÃ¤Â½?
unsigned char GBuff[MAX_PACKET_LEN];
uchar MsgType[2];
uchar GBitMap[8];
int GPacketLen;
unsigned char GBuff128[MAX_PACKET_LEN];
uchar MsgType128[4];
uchar GBitMap128[16];
int GPacketLen128;
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
int   SetPacket128(uchar *Buff,int Len)
{
	if(Len>MAX_PACKET_LEN)
		return -1;
	if(Len<10)
		return -1;

	memcpy(GBuff128,Buff,Len);	
	memcpy(MsgType128,GBuff128,2);
	memcpy(GBitMap128,GBuff+2,8);
	GPacketLen128=Len;
	return 0;
}
int  GetPacket(uchar *Buff,int BuffLen)
{
	if(GPacketLen >BuffLen)
		return -1;
	memcpy(Buff,GBuff,GPacketLen);
	return GPacketLen;
}
int  GetPacket128(uchar *Buff,int BuffLen)
{
	if(GPacketLen >BuffLen)
		return -1;
	memcpy(Buff,GBuff128,GPacketLen128);
	return GPacketLen128;
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
int GetFieldLen128(ISO_DEF isodef,int *HeadLen,uchar *Data)
{
	int DataLen;
	if(isodef.fmt==0)
	{
		*HeadLen=0;
		DataLen=isodef.len;
	}
	if(isodef.fmt==2)
	{
		*HeadLen=2;
		DataLen=(Data[0]-0x30)*10+(Data[1]-0x30);
		//DataLen=bcd2int(Data,1);
	}
	if(isodef.fmt==3)
	{
		*HeadLen=3;
		DataLen=(Data[0]-0x30)*100+(Data[1]-0x30)*10+(Data[2]-0x30);
		//DataLen=bcd2int(Data,2);
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
int   GetField128(int FieldNum,uchar *Buff,int *DataLen,int *typ)
{
	int i,len,HeadLen;
	int offset;
	if (FieldNum>128)
		return -1;
	offset=0;
	for (i=0;i<FieldNum&&i<128;i++)
	{
		if(i==0)
		{
			offset+=4;
			continue;
		}
		if(i==1)
		{
			offset+=16;
			continue;
		}
		if (!MY_BITGET(GBitMap128,i))
			continue;
		len=GetFieldLen128(s_new_isodef[i],&HeadLen,(uchar*)GBuff128+offset);
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
		memcpy(Buff,GBuff128,4);
		*DataLen=4;
		return 0;
	}
	if(FieldNum==1)//BitMap
	{
		memcpy(Buff,GBuff128+4,8);
		*DataLen=16;
		return 0;
	}
	if(!MY_BITGET(GBitMap128,FieldNum))
		return -3;//Ã¦â€”Â Ã¦Â­Â¤Ã¥Å?
	len=GetFieldLen128(s_new_isodef[FieldNum],&HeadLen,(uchar*)GBuff128+offset);
	if(len<0)
	{
		return -1;
	}
	if(len>*DataLen)
		return -4;//Ã§â€Â¨Ã¦Ë†Â·Ã¥Å’ÂºÃ¦ÂºÂ¢Ã¥â€?
	memcpy(Buff,GBuff128+offset+HeadLen,len);
	*DataLen=len;
	*typ=s_new_isodef[FieldNum].typ;
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
int  DelField128(int FieldNum)
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

	if (!MY_BITGET(GBitMap128,FieldNum))
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
		if (!MY_BITGET(GBitMap128,i))
			continue;
		len2=GetFieldLen(s_new_isodef[i],&HeadLen,(uchar*)GBuff128+offset);
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
	if(MY_BITGET(GBitMap128,FieldNum))//Ã¥Ë†Â Ã©â„¢Â¤Ã¥Å½Å¸Ã¦ÂÂ¥Ã§Å¡â€žÃ©â€šÂ£Ã¤Â¸ÂªÃ¥Å¸Å?
	{
		len2=GetFieldLen(s_new_isodef[FieldNum],&HeadLen,(uchar*)GBuff128+offset);
		GPacketLen128-=(len2+HeadLen);
		memcpy(GBuff128+offset,GBuff128+offset+len2+HeadLen,GPacketLen128-offset);
	}

	MY_BITCLR(GBitMap128,FieldNum);
	memcpy(GBuff+2,GBitMap128,16);
	
	return 0;
}

void DelFld128_90(uchar *hexBuff, int BuffLen, int *SendLen)
{
	
	int Ret,i,j;
	int FieldLen,Fieldtype;
	uchar tmpBuff[2048];
	char str[2048];
	char sstr[10];
	HtLog("packet.log", HT_LOG_MODE_ERROR, __FILE__,__LINE__, "RRF Del FLD 31\n"); 

	memset(str,0,2048);

	Ret=SetPacket(&hexBuff[MagPacketHeaderLen128],BuffLen-MagPacketHeaderLen128);
	if(Ret!=0)
	{
		HtLog("packet.log", HT_LOG_MODE_ERROR, __FILE__,__LINE__, "packet err\n");
		return; 	
	}


	DelField128(90);
	*SendLen=GetPacket128(&hexBuff[MagPacketHeaderLen128],4096);
	*SendLen=*SendLen+MagPacketHeaderLen128;

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
	else if (strcmp(msg, "1702sendpacket") ==0 || strcmp(msg, "1702Recvpacket") == 0)
	{
		s_isodef[61].typ = TYP_ASC;
		s_isodef[35].typ = TYP_ASC;
		s_isodef[36].typ = TYP_ASC;
		//s_isodef[61].fmt = 2;
	    s_isodef[54].typ= TYP_ASC;
		//s_isodef[54].fmt= 2;
	    s_isodef[49].typ = TYP_ASC;
	    s_isodef[49].len = 2;
		
		s_isodef[54].len = 40;
		//s_isodef[62].fmt=2;
		
	}
    else if(strcmp(msg,"1709sendpacket")==0 ||strcmp(msg,"1709Recvpacket")==0)
	{
		s_isodef[21].typ=TYP_ASC;
	    s_isodef[21].fmt=2;
        s_isodef[21].len=256;
	}
	else if(strcmp(msg,"1710sendpacket")==0 ||strcmp(msg,"1710Recvpacket")==0)
	{
		s_isodef[35].typ=TYP_ASC;
		s_isodef[36].typ=TYP_ASC;
	}
	else if(strcmp(msg,"1711sendpacket")==0 ||strcmp(msg,"1711Recvpacket")==0)
	{
		s_isodef[31].typ=TYP_ASC;
		s_isodef[31].fmt=0;
		s_isodef[31].len=6;
	}
	else if(strcmp(msg,"1715sendpacket")==0 ||strcmp(msg,"1715Recvpacket")==0)
	{
		s_isodef[59].fmt=3;
	}
	else if (strcmp(msg,"1718sendpacket")==0 ||strcmp(msg, "1718Recvpacket")==0)
	{
		MagPacketHeaderLen=46+2;
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
	if(strcmp(msg, "1702sendpacket") == 0 || strcmp(msg, "1702Recvpacket") == 0)
		Ret=SetPacket(&hexBuff[MagPacketHeaderLen-4],BuffLen-MagPacketHeaderLen-4);
	else
	    Ret=SetPacket(&hexBuff[MagPacketHeaderLen],BuffLen-MagPacketHeaderLen);
	if(Ret!=0)
	{
		HtLog("packet.log", HT_LOG_MODE_ERROR, __FILE__,__LINE__, "%s packet err\n",msg);
		return; 	
	}
	HtDebugString("packet.log", HT_LOG_MODE_DEBUG, __FILE__, __LINE__, hexBuff, BuffLen);

	memset(str,0,2048);
	if(strcmp(msg, "1702sendpacket") == 0 || strcmp(msg, "1702Recvpacket") == 0)
	{
		for(i=0; i<MagPacketHeaderLen-4; i++)
		{
			sprintf(sstr,"%0.2X ",hexBuff[i]);
			strcpy(str+strlen(str),sstr);
		}
	}
	else
	{
	for(i=0; i<MagPacketHeaderLen; i++)
	{
		sprintf(sstr,"%0.2X ",hexBuff[i]);
		strcpy(str+strlen(str),sstr);
	}
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

void AddFld21(uchar *hexBuff, int BuffLen, int *SendLen)
{
	int Ret,i,j;
	int FieldLen,Fieldtype;
	//uchar tmpBuff[2048];
	char str[100];
	//char sstr[10];
	HtLog("packet.log", HT_LOG_MODE_ERROR, __FILE__,__LINE__, "RRF Add FLD 21\n"); 
	s_isodef[21].typ=TYP_ASC;
	s_isodef[21].fmt=2;
    s_isodef[21].len=256;

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
	memcpy(str,"1700518001505150134320000508d0000d679000001CC0000000020898602B0131450145978",75);
	SetField(21,str,75);


	

	*SendLen=GetPacket(&hexBuff[MagPacketHeaderLen],4096);
	
	*SendLen=*SendLen+MagPacketHeaderLen;


	
	
	
}

void DelFld21(uchar *hexBuff, int BuffLen, int *SendLen)
{
	
	int Ret,i,j;
	int FieldLen,Fieldtype;
	uchar tmpBuff[2048];
	char str[2048];
	char sstr[10];
	HtLog("packet.log", HT_LOG_MODE_ERROR, __FILE__,__LINE__, "RRF Del FLD 21\n"); 
	
	{
		s_isodef[21].typ=TYP_ASC;
	    s_isodef[21].fmt=2;
        s_isodef[21].len=256;
	}

	memset(str,0,2048);

	Ret=SetPacket(&hexBuff[MagPacketHeaderLen],BuffLen-MagPacketHeaderLen);
	if(Ret!=0)
	{
		HtLog("packet.log", HT_LOG_MODE_ERROR, __FILE__,__LINE__, "packet err\n");
		return; 	
	}


	DelField(21);
	*SendLen=GetPacket(&hexBuff[MagPacketHeaderLen],4096);
	*SendLen=*SendLen+MagPacketHeaderLen;


}

void AddFld31(uchar *hexBuff, int BuffLen, int *SendLen)
{
	int Ret,i,j;
	int FieldLen,Fieldtype;
	//uchar tmpBuff[2048];
	char str[100];
	//char sstr[10];
	HtLog("packet.log", HT_LOG_MODE_ERROR, __FILE__,__LINE__, "RRF Add FLD 31\n"); 
	{
		s_isodef[31].typ=TYP_ASC;
		s_isodef[31].fmt=0;
		s_isodef[31].len=6;
	}

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
	memcpy(str,"151123",6);
	SetField(31,str,6);
	*SendLen=GetPacket(&hexBuff[MagPacketHeaderLen],4096);
	*SendLen=*SendLen+MagPacketHeaderLen;
}

void DelFld31(uchar *hexBuff, int BuffLen, int *SendLen)
{
	
	int Ret,i,j;
	int FieldLen,Fieldtype;
	uchar tmpBuff[2048];
	char str[2048];
	char sstr[10];
	HtLog("packet.log", HT_LOG_MODE_ERROR, __FILE__,__LINE__, "RRF Del FLD 31\n"); 

	memset(str,0,2048);

	Ret=SetPacket(&hexBuff[MagPacketHeaderLen],BuffLen-MagPacketHeaderLen);
	if(Ret!=0)
	{
		HtLog("packet.log", HT_LOG_MODE_ERROR, __FILE__,__LINE__, "packet err\n");
		return; 	
	}


	DelField(31);
	*SendLen=GetPacket(&hexBuff[MagPacketHeaderLen],4096);
	*SendLen=*SendLen+MagPacketHeaderLen;
}

void AddFld59(uchar *hexBuff, int BuffLen, int *SendLen)
{
	int Ret,i,j;
	int FieldLen,Fieldtype;
	//uchar tmpBuff[2048];
	char str[100];
	//char sstr[10];
	HtLog("packet.log", HT_LOG_MODE_ERROR, __FILE__,__LINE__, "RRF Add FLD 59\n"); 
	
	{
		s_isodef[59].fmt=3;
	}
	memset(str,0,100);
	Ret=SetPacket(&hexBuff[MagPacketHeaderLen],BuffLen-MagPacketHeaderLen);
	if(Ret!=0)
	{
		HtLog("packet.log", HT_LOG_MODE_ERROR, __FILE__,__LINE__, "packet err\n");
		return; 	
	}
	//HtDebugString("packet.log", HT_LOG_MODE_DEBUG, __FILE__, __LINE__, hexBuff, BuffLen);

    memset(str, 0, sizeof(str));
	memcpy(str,"\x56\x53\x58\x58\x31\x34\x31\x30\x32\x30\x30\x30\x20\x20\x20\x20\x20\x20\x20\x20\x32\x30\x31\x35\x31\x32\x31\x36\x31\x37\x34\x38\x30\x32\x20\x20\x20\x20\x20\x20",40);
	SetField(59,str,40);
	*SendLen=GetPacket(&hexBuff[MagPacketHeaderLen],4096);
	*SendLen=*SendLen+MagPacketHeaderLen;
}

void DelFld59(uchar *hexBuff, int BuffLen, int *SendLen)
{
	
	int Ret,i,j;
	int FieldLen,Fieldtype;
	uchar tmpBuff[2048];
	char str[2048];
	char sstr[10];
	HtLog("packet.log", HT_LOG_MODE_ERROR, __FILE__,__LINE__, "RRF Del FLD 59\n"); 

	memset(str,0,2048);

	Ret=SetPacket(&hexBuff[MagPacketHeaderLen],BuffLen-MagPacketHeaderLen);
	if(Ret!=0)
	{
		HtLog("packet.log", HT_LOG_MODE_ERROR, __FILE__,__LINE__, "packet err\n");
		return; 	
	}


	DelField(59);
	*SendLen=GetPacket(&hexBuff[MagPacketHeaderLen],4096);
	*SendLen=*SendLen+MagPacketHeaderLen;


}

int Convert64To128(uchar *hexBuff, uchar *sendmsg, int BuffLen, int* ConvertLen )
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
	char* pch;
	int HeadLen,DataLen;
	HtLog("packet.log", HT_LOG_MODE_ERROR, __FILE__,__LINE__, "%s\n",hexBuff); 
	
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
		HtLog("packet.log", HT_LOG_MODE_ERROR, __FILE__,__LINE__, "%s packet err\n",hexBuff);
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
			HtLog("packet.log", HT_LOG_MODE_ERROR, __FILE__, __LINE__, "%s", ascii_buf);

		}
	}
	HtLog("packet.log", HT_LOG_MODE_ERROR, __FILE__,__LINE__, "sField63   %s",sField63);
	// default 128 message length=0x30 0x30 0x30 0x30
	memcpy(sendmsg,"0000", 4);
	memcpy(sendmsg+4, hexBuff, MsgPacketHeaderLen);
	memcpy(sendmsg+4+MsgPacketHeaderLen, ascii_buf,offset);
	*ConvertLen = 4+MsgPacketHeaderLen+offset;
	*ConvertLen = *ConvertLen-8-nField63Len-3;  // del 64 field data and 63 field
	MY_BITCLR(sendmsg+4+MsgPacketHeaderLen+4,63);  // del bitmap 63 bit 1---> 0
	MY_BITCLR(sendmsg+4+MsgPacketHeaderLen+4,64); // del bitmap  64 bit 1---> 0
	HtDebugString("packet.log", HT_LOG_MODE_DEBUG, __FILE__, __LINE__, sendmsg, 4+MsgPacketHeaderLen+offset);
    // next trouble is add data between 64 to 128  field to message 
    // data stores in field 63. tag 7F 90 ---- 90 field   tag 7F 22 ----- 122 field  
    pch = strstr(sField63, "\x7F\x90");
    if( pch == NULL )
    {
        ;
    }
    else
    {
	memcpy(sendmsg+(*ConvertLen), pch+2, 42);
	*ConvertLen=(*ConvertLen)+42;
	MY_BITSET(sendmsg+4+MsgPacketHeaderLen+4, 90); // add 90 field like add 26 field 
    }

    pch = strstr(sField63, "\x7F\x22");
    if ( pch == NULL )
    {
    	;
    }
    else
    {
    memcpy(sstr, pch+2, 3);
    sstr[3]=0x00;
    FieldLen=atoi(sstr);
    memcpy(sendmsg+(*ConvertLen), pch+2, FieldLen+3);
    *ConvertLen=*ConvertLen+FieldLen+3;
    MY_BITSET(sendmsg+4+MsgPacketHeaderLen+4, 122);  // add 122 field like add 58 field   	
    }
    // reset bitmap  del 63 64   add 90 122 field
    memcpy(sendmsg+(*ConvertLen), "\x00\x00\x00\x00\x00\x00\x00\x00", 8);
    *ConvertLen = (*ConvertLen) + 8;  // add 128 field 
    MY_BITSET(sendmsg+4+MsgPacketHeaderLen+4,128);
    sprintf(sstr, "%04d", (*ConvertLen));
    memcpy(sendmsg, sstr,4);
    memcpy(sendmsg+4+2, sstr, 4);
    return 0;
}
int Convert128To64(uchar hexBuff[], uchar sendmsg[], int* nMsgLen)
{
	int Ret,i,j;
	int FieldLen,Fieldtype;
	int MsgPacketHeaderLen=46;
	uchar tmpBuff[2048];
	uchar bcd_buf[2048];
	char str[2048];
	char sstr[10];
	uchar sField63[1024];
	int  nField63Len;
	int DataLen;
	int HeadLen;
	int offset;
	unsigned char bcd_value;
	HtLog("packet.log", HT_LOG_MODE_ERROR, __FILE__,__LINE__, "%s\n",hexBuff); 
	
	memset(str,0,2048);
	for(i=0;i<(*nMsgLen);i++)
	{
		sprintf(sstr,"%0.2X",hexBuff[i]);
		strcpy(str+strlen(str),sstr);
	}
	HtLog("packet.log", HT_LOG_MODE_ERROR, __FILE__,__LINE__, "%s",str);
	Ret=SetPacket(&hexBuff[MsgPacketHeaderLen],(*nMsgLen)-MsgPacketHeaderLen);
	if(Ret!=0)
	{
		HtLog("packet.log", HT_LOG_MODE_ERROR, __FILE__,__LINE__, "%s packet err\n",hexBuff);
		return; 	
	}
	HtDebugString("packet.log", HT_LOG_MODE_DEBUG, __FILE__, __LINE__, hexBuff, (*nMsgLen));

	memset(str,0,2048);
	for(i=0; i<MsgPacketHeaderLen; i++)
	{
		sprintf(sstr,"%0.2X ",hexBuff[i]);
		strcpy(str+strlen(str),sstr);
	}
	HtLog("packet.log", HT_LOG_MODE_ERROR, __FILE__,__LINE__, "Head   %s",str);
	memset(str,0,2048);
    offset=0;
	for(i=0; i<=128; i++)
	{
		FieldLen =2048;
		if(GetField128(i,tmpBuff,&FieldLen,&Fieldtype)>=0)
		{
			if(FieldLen==0)
				sprintf(str,"F%0.2d     ",i);
			else
			{
                if(s_new_isodef[i].fmt == 0)
                {
                	HeadLen=0;                    
                }
                else if (s_new_isodef[i].fmt == 2)
                {
                    HeadLen=1;
                }
                else if (s_new_isodef[i].fmt == 3)
                {
                    HeadLen=2;
                }
                DataLen=FieldLen;
                if(HeadLen == 0 && s_new_isodef[i].typ == TYP_ASC && s_isodef[i].typ == TYP_BCD)
                {
                	asc_to_bcd(bcd_buf+offset,tmpBuff,DataLen);
                	DataLen=(DataLen+1)/2;
                }
                else if(HeadLen == 0 && s_new_isodef[i].typ == TYP_ASC && s_isodef[i].typ == TYP_ASC)
                {
                    memcpy(bcd_buf+offset, tmpBuff,DataLen);                    
                }
                else if(HeadLen == 1 && s_new_isodef[i].typ == TYP_ASC && s_isodef[i].typ == TYP_BCD)
                {  
                    bcd_value=(DataLen/10)<<4;
                    bcd_value=bcd_value | (DataLen%10);
                    memcpy(bcd_buf+offset, &bcd_value, 1);
                	asc_to_bcd(bcd_buf+offset+1, tmpBuff, DataLen,0);
                	DataLen=(DataLen+1)/2;
                }
                else if(HeadLen == 1 && s_new_isodef[i].typ == TYP_ASC && s_isodef[i].typ == TYP_ASC)
                {
                    bcd_value=(DataLen/10)<<4;
                    bcd_value=bcd_value | (DataLen%10);
                    memcpy(bcd_buf+offset, &bcd_value, 1);                	
                	memcpy(bcd_buf+offset+1,tmpBuff,DataLen);
                	
                }
                else if(HeadLen == 2 && s_new_isodef[i].typ == TYP_ASC && s_isodef[i].typ == TYP_BCD)
                {
                	sprintf(sstr, "%03d", DataLen);
                	asc_to_bcd(bcd_buf+offset, sstr,3,1); // length 0x31 0x34 0x35 convert to  0x01 0x45
                	asc_to_bcd(bcd_buf+offset+2,tmpBuff, DataLen, 0);
                	DataLen=(DataLen+1)/2;
                }
                else if(HeadLen == 2 && s_new_isodef[i].typ == TYP_ASC && s_isodef[i].typ == TYP_ASC)
                {
                	sprintf(sstr, "%03d", DataLen);
                	asc_to_bcd(bcd_buf+offset,sstr, 3, 1);
                	memcpy(bcd_buf+offset+2, tmpBuff, DataLen);
                }
                else
                {
                	memcpy(bcd_buf+offset, tmpBuff, DataLen+HeadLen);
                }
                if(i == 90 )
                {
                    memcpy(sField63+2,"\x7F\x90",2);
                    memcpy(sField63+2+2, tmpBuff,42);
                    nField63Len=44;
                    memcpy(sField63, "\x00\x44",2);
                }
                if(i == 122 )
                {
                	if(nField63Len == 44)
                	{
                		sprintf(sstr, "%03d", DataLen);
                		memcpy(sField63+46, "\x7F\x22", 2);
                		memcpy(sField63+46+2, sstr, 3);
                		memcpy(sField63+49+2, tmpBuff,DataLen);
                		nField63Len=nField63Len+2+3+DataLen;
                		sprintf(sstr, "%03d", nField63Len);
                		asc_to_bcd(sField63, sstr, 3, 1);
                	}
                	else
                	{
                		sprintf(sstr, "%03d", DataLen);
                		asc_to_bcd(sField63, sstr, 3, 1);
                		memcpy(sField63+2, tmpBuff, DataLen);
                	}

                }
                offset=HeadLen+DataLen;

			}
			HtLog("packet.log", HT_LOG_MODE_ERROR, __FILE__,__LINE__, "%s",bcd_buf);


		}
	}
	memcpy(sendmsg, hexBuff, 46); // head of message
    memcpy(sendmsg+46,bcd_buf, offset);
    *nMsgLen=46+offset;
    HtLog("packet.log", HT_LOG_MODE_ERROR, __FILE__, __LINE__, "%s", sendmsg);
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
