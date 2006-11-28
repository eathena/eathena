#include "basetree.h"

NAMESPACE_BEGIN(basics)


CDBBase::CDBNode::CDBMemory CDBBase::CDBNode::cMem;



void test_db(void)
{
#if defined(DEBUG)
	CStrDB<int*>	sdb(false,false);
	CNumDB<int*>	ndb(false);


#endif
}



NAMESPACE_END(basics)
