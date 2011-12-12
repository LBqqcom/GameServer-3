//SessionManage.h
#ifndef _SessionMangage_H_
#define _SessionMangage_H_
#include "..\iocp\IOCP.h"
#include "..\MsgDef\MsgDef.h"
typedef struct _CSession
{
	CIOCPContext* lpSession;
	CRITICAL_SECTION Lock;
	_CSession *pNext;
}CSession;
class CSessionMangage 
{
public:
	CSessionMangage();
	~CSessionMangage();
public:
	void OnMessage(PACKET* lpPacket);
private:
	CSession* m_pFreeSessionList;
	int m_nFreeSessionCount;
	int m_nMaxFreeSessions;
	CRITICAL_SECTION m_FreeSessionListLock;
	// ������ͷ��׽���������ָ��

	CSession *AllocateSession();
	void ReleaseSession(CSession *pSession);
	// �ͷſ��л����������б�Ϳ���������ָ���б�
	void FreeSession();
};
#endif