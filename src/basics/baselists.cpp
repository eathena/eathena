#include "basetypes.h"
#include "baselists.h"

class linkdata : public CDLinkNode
{
public:
	int a;
	linkdata(int i):a(i)	{}
	~linkdata()	{}

	linkdata* next()	{ return dynamic_cast<linkdata*>( CDLinkNode::next() ); }
	linkdata* prev()	{ return dynamic_cast<linkdata*>( CDLinkNode::prev() ); }

};


void test_lists()
{
	//double linked list test with scope creation/destruction
	CDLinkNode head, tail, root;

	{
		CDLinkNode a(head,tail);
		CDLinkNode b(head,tail);
		CDLinkNode c(head,tail);
	}

	{
		CDLinkNode d(root);
		CDLinkNode e(root);
		{
			CDLinkNode f(root);
		}
	}

	{
		TDLinkRoot<linkdata> root;

		linkdata a(1),b(2),c(3),d(4);
		linkdata* x;

		x=root.top(); while(x) {printf("%i ", x->a); x=x->next(); } printf("\n");
		root.insert(a);
		x=root.top(); while(x) {printf("%i ", x->a); x=x->next(); } printf("\n");
		root.insert(b);
		x=root.top(); while(x) {printf("%i ", x->a); x=x->next(); } printf("\n");
		root.push(c);
		x=root.top(); while(x) {printf("%i ", x->a); x=x->next(); } printf("\n");
		root.insert(d);
		x=root.top(); while(x) {printf("%i ", x->a); x=x->next(); } printf("\n");

		b.unlink();
		x=root.top(); while(x) {printf("%i ", x->a); x=x->next(); } printf("\n");
		d.unlink();
		x=root.top(); while(x) {printf("%i ", x->a); x=x->next(); } printf("\n");

		x = root.pop();
		x=root.top(); while(x) {printf("%i ", x->a); x=x->next(); } printf("\n");

		root.top(x);
		x=root.top(); while(x) {printf("%i ", x->a); x=x->next(); } printf("\n");
		root.pop();
		x=root.top(); while(x) {printf("%i ", x->a); x=x->next(); } printf("\n");


	}
}
