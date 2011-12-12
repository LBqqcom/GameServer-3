
#include <windows.h>
#include <io.h>
#include "inifile.h"
#define INI_FILE_MAX_ROW 1024
#ifdef WIN32
#define ACCESS _access
#else        ACCESS access
#endif        

int isbreakchar(char ch)
{
    if(ch==9)        //Tab Key
	return 0;
    if((ch<32)&&(ch>0))
	return 1;
    else
	return 0;
}
/*���ļ��ж�ȡһ��*/
int readline(FILE* fpt, char* linestr)
{
    int i,rv;
    char cc;
    char buf[INI_FILE_MAX_ROW];

    if(feof(fpt)){
	return 1;
    }
    for(i=0;i<INI_FILE_MAX_ROW;i++){
	rv=fread(&cc,1,1,fpt);
	if(rv<1)
	    break;
	if(isbreakchar(cc)==1)
	    break;
	buf[i]=cc;
    }
    buf[i]='\0';
    if(lstrlen(buf) == 0)
	return 1;
    lstrcpy(linestr,buf);
    return 0;        
}
/*ȥ���ײ��Ŀո����Tab��*/
void TrimLeftSpace(char *str)
{
    char buf[INI_FILE_MAX_ROW];
    char *ptr;

    if(str==NULL)
	return;
    ptr=str;
    while(*ptr != '\0')
    {
	if((*ptr !=' ')&&(*ptr !='\t'))
	    break;
	ptr ++;
    }
    lstrcpy(buf,ptr);
    lstrcpy(str,buf);
}
/*ȥ��β���Ŀո����Tab��*/
int TrimRightSpace(char *str)
{
    int pos_end;
    char *ptr;
    if(str==NULL)
	return 1;
    ptr = str;
    pos_end = lstrlen(ptr);
    if(pos_end==0)
	return 1;
    while((*(ptr+pos_end-1) ==' ')||(*(ptr+pos_end-1) == '\t'))
    {
	if(pos_end == 0)
	    break;
	pos_end--;
    }
    *(str+pos_end)='\0';
    return 0;
}

int trimspace(char* ori,char * trim)
{
    char *ptrsrc,*ptrdes;

    ptrsrc = ori;
    ptrdes = trim;
    while(*ptrsrc != '\0')
    {
	if((*ptrsrc != ' ')&&(*ptrsrc!='\r'))
	{
	    *ptrdes = *ptrsrc;
	    ptrdes++;
	}
	ptrsrc ++;
    }
    *ptrdes = '\0';
    return 0;
}
/***********************
��һ���н�����item��value
������
line������
item������
value�����
����ֵ��
0���ɹ�
1��ʧ��
*/ 
int readitemfromline(char *line,char *item,char *value)
{

    char linebuf[INI_FILE_MAX_ROW],leftbuf[INI_FILE_MAX_ROW],rightbuf[INI_FILE_MAX_ROW];
    char *ptr;

    if((line==NULL)||(item==NULL))
	return 1;
    lstrcpy(linebuf,line);
    TrimLeftSpace(linebuf);
    if(linebuf[0] == '#')/*��ʾ����Ϊע��*/
	return 1;
    ptr = strchr(linebuf,'=');
    if(ptr==NULL)
	return 1;
    *ptr = '\0';
    /*�ѡ�=��2�ߵ��ַ����ֿ�*/
    lstrcpy(leftbuf,linebuf);
    lstrcpy(rightbuf,ptr+1);
    /*ȥ��2�ߵĿո�*/
    TrimLeftSpace(leftbuf);
    TrimRightSpace(leftbuf);
    TrimLeftSpace(rightbuf);
    TrimRightSpace(rightbuf);
    if(lstrcmp(leftbuf,item)==0)/*�ҵ�item*/
    {
	lstrcpy(value,rightbuf);
	return 0;
    }
    else
	return 1;
}
/*����һ���ַ����Ƿ�����section���ҵ�����1�����򷵻�0*/
int isfindsection(char *line,char *section)
{
    char *ptr;
    char linebuf[INI_FILE_MAX_ROW];

    if((line==NULL)||(section==NULL))
	return 0;
    if(*line != '[')
	return 0;
    lstrcpy(linebuf,line+1);
    ptr = strchr(linebuf,']');
    if(ptr==NULL)
	return 0;
    *ptr = '\0';
    /*ȥ����[����]���м��ַ����еĿո�*/
    TrimLeftSpace(linebuf);
    TrimRightSpace(linebuf);
    if(lstrcmp(linebuf,section)==0)
	return 1;
    else
	return 0;
}
/***************************************
������
��ini�ļ��ж�ȡsection��item��value
������
inifile:���������ini�ļ���
section�����������
item���������
value���������
����ֵ��
0���ɹ�
1��ʧ��
*****************************************/
int getiniitem(char * inifile,char* section,char *item,char *value)
{
    FILE *fp;
    int bOK;
    char line[INI_FILE_MAX_ROW];
    fp = fopen(inifile,"r");
    if(fp == NULL)
    {
	WriteToLog("open file error");
	return 1;
    }
    /*���ñ�־�����ļ��в���section*/
    bOK = 0;
    while(!readline(fp,line))
    {
	if(isfindsection(line,section))
	{
	    bOK = 1;/*�ҵ�section*/
	    break;
	}
    }
    if(!bOK)/*û���ҵ�section������1*/
	return 1;

    /*���ñ�־�����ļ��ж�ȡitem*/
    bOK = 0;
    while(!readline(fp,line))
    {
	//trimspace(line,linebuf);
	if(line[0] == '[')/* �ҵ�'['��˵����������һsection��û����item */
	    break;
	if(readitemfromline(line,item,value) == 0)/*��һ���ַ����в���item������ȡvalue*/
	{
	    bOK = 1;/*�ҵ�item*/
	    break;
	}
    }
    fclose(fp);
    return !bOK;
}
/**********************************
��Ϊ�������
���أ�
0�������ɹ�
1��ʧ��
**********************************/
int setiniitem(char * inifile,char* section,char *item,char *value)
{
    FILE *fp,*fptmp;
    int bfindsection,bok;
    char valueold[INI_FILE_MAX_ROW],line[INI_FILE_MAX_ROW];
    char tmpfile[128];
    char *ptr;
    /*�ж�ini�ļ��Ƿ����*/
    if(ACCESS(inifile,0))/*������*/
    {
	fp = fopen(inifile,"w");
	if(fp == NULL)
	    return 1;
	fprintf(fp,"[%s]\n",section);
	fprintf(fp,"%s=%s\n",item,value);
	fclose(fp);
	return 0;
    }
    /*ini�ļ��Ѿ�����*/
    fp = fopen(inifile,"r");
    if(fp == NULL)
    {
	WriteToLog("open file for read error!");
	return 1;
    }
    /*�½���һ����ʱ�ļ�*/
    sprintf(tmpfile,"%s.tmp",inifile);
    fptmp = fopen(tmpfile,"w");
    if(fptmp == NULL)
    {
	WriteToLog("open file for write error!");
	return 1;
    }

    bfindsection = 0;/*�Ƿ���section��־*/
    bok = 0;/*�Ƿ���д���־*/
    while(!readline(fp,line))
    {        
	if(bok)/*�Ѿ���section�е�itemд�룬��ʣ�µ��ļ�ֱ��д����ʱ�ļ���*/
	{
	    fprintf(fptmp,"%s\n",line);//
	    continue;
	}
	if(!bfindsection)/*δ�ҵ�section*/
	{
	    if(isfindsection(line,section))
	    {
		bfindsection = 1;
		fprintf(fptmp,"%s\n",line);
		continue;
	    }
	    else
	    {
		fprintf(fptmp,"%s\n",line);
		continue;
	    }
	}
	else/*�ҵ���section*/
	{
	    if(line[0] != '[')/*δ������һ��section*/
	    {
		if(!readitemfromline(line,item,valueold))/*��ȡ���ɵ�item*/
		{
		    ptr = strchr(line,'=');
		    *ptr='\0';
		    fprintf(fptmp,"%s=%s\n",line,value);/*������ֵ*/
		    bok = 1;
		    continue;
		}
		else
		{
		    fprintf(fptmp,"%s\n",line);
		    continue;
		}
	    }
	    else/*������һ��section*/
	    {
		fprintf(fptmp,"%s=%s\n",item,value);/*���һ����item*/
		fprintf(fptmp,"%s\n",line);
		bok = 1;
	    }
	}
    }

    if(!bok&&!bfindsection)
    {
	fprintf(fptmp,"[%s]\n",section);
	fprintf(fptmp,"%s=%s\n",item,value);
    }
    if(!bok&&bfindsection)
    {
	fprintf(fptmp,"%s=%s\n",item,value);
    }
    fclose(fp);
    fclose(fptmp);
    remove(inifile);
    rename(tmpfile,inifile);
    return 0;
}
