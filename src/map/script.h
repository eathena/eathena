// $Id: script.h,v 1.2 2004/09/25 05:32:19 MouseJstr Exp $
#ifndef _SCRIPT_H_
#define _SCRIPT_H_

// predeclaration
struct map_session_data;



extern struct Script_Config
{
	unsigned verbose_mode : 1;
	unsigned warn_func_no_comma : 1;
	unsigned warn_cmd_no_comma : 1;
	unsigned warn_func_mismatch_paramnum : 1;
	unsigned warn_cmd_mismatch_paramnum : 1;
	unsigned event_requires_trigger : 1;
	unsigned event_script_type : 1;
	unsigned _unused7 : 1;

	size_t check_cmdcount;
	size_t check_gotocount;
	char die_event_name[24];
	char kill_event_name[24];
	char login_event_name[24];
	char logout_event_name[24];
	char mapload_event_name[24];
} script_config;


///////////////////////////////////////////////////////////////////////////////
// a label marks a position within a script with a name
class CLabel : public MiniString
{
public:
	///////////////////////////////////////////////////////////////////////////
	// class data
						// name of the label
	size_t		cPos;	// position within the programm

	///////////////////////////////////////////////////////////////////////////
	// constructors/destructor
	CLabel()	{}
	CLabel(const MiniString& s) : MiniString(s)	{}
	CLabel(const MiniString& s, size_t p) : MiniString(s), cPos(p)	{}
	~CLabel()	{}
	///////////////////////////////////////////////////////////////////////////
	// compare operators for sorting derived from MiniString
};

///////////////////////////////////////////////////////////////////////////////
// a script data stub
// contains the parsed programm and a label list
class _CScript : public global, public noncopyable
{
	///////////////////////////////////////////////////////////////////////////
	// friends
	friend class CScript;
	friend class TPtrCount<_CScript>;

	///////////////////////////////////////////////////////////////////////////
	// class data
	MiniString					cName;		// name of the script
	TArrayDST<unsigned char>	cProgramm;	// the programm
	TslistDST<CLabel>			cLabels;	// list of labels

	///////////////////////////////////////////////////////////////////////////
	// construction only friends are allowed to create
	_CScript(const char* n, const unsigned char* s, size_t sz, CLabel* l, size_t cnt)
		: cName(n), cProgramm(s, sz), cLabels(l, cnt)
	{ }
public:
	///////////////////////////////////////////////////////////////////////////
	// normal destructor
	~_CScript()	{ }

	///////////////////////////////////////////////////////////////////////////
	// class access done by the friends
};

///////////////////////////////////////////////////////////////////////////////
// a script container
// consists of a reference counting pointer to the actual script object
// automatic destruction when the script is not referenced
class CScript : public global
{
	TPtrCount<_CScript>	cScript;
public:
	CScript(const char* n, const unsigned char* s, size_t sz, CLabel* l, size_t cnt)
		: cScript(new _CScript(n,s,sz,l,cnt))
	{}
	~CScript() {}

	void appendByte(unsigned char a);
	void appendCommand(int a);
	void appendInt(int a);
	void appendL(int l);
	void setLabel(size_t l, size_t pos);
	int getInt(unsigned int &pos);
	int getCommand(unsigned int &pos);

	const char *skipSpaceComment(const char *p);
	const char *skipWord(const char *p);
	void ErrorMessage(const char *mes, const char *pos);
	const char* parseSimpleExpr(const char *p);
	const char* parseSubExpr(const char *p,int limit);
	const char* parseExpr(const char *p);
	const char* parseLine(const char *p);
};




///////////////////////////////////////////////////////////////////////////////
// simple class only since we use c-style allocation and clearing at the moment
class CScriptEngine : public noncopyable
{

	friend class CValue;
private:
	///////////////////////////////////////////////////////////////////////////
	// ŽÀsŒn
	enum STATES		{ OFF=0,RUN, STOP,END,RERUNLINE,GOTO,RETFUNC,ENVSWAP }; // 0...6 ->3bit
	enum NPCSTATE	{ NONE=0, NPC_GIVEN, NPC_DEFAULT };						// 0...2 ->2bit
public:
	enum
	{
		C_NOP=0,C_POS,C_INT,C_PARAM,C_FUNC,C_STR,C_CONSTSTR,C_ARG,
		C_NAME,C_EOL, C_RETINFO,

		C_LOR,C_LAND,C_LE,C_LT,C_GE,C_GT,C_EQ,C_NE,   //operator
		C_XOR,C_OR,C_AND,C_ADD,C_SUB,C_MUL,C_DIV,C_MOD,C_NEG,C_LNOT,C_NOT,C_R_SHIFT,C_L_SHIFT
	};


	///////////////////////////////////////////////////////////////////////////
	// simple Variant type with 2 components
	class CValue
	{
		friend class CScriptEngine;
	public:
		///////////////////////////////////////////////////////////////////////
		int type;
		union
		{
			int num;
			const char *str;
		};
	
		///////////////////////////////////////////////////////////////////////
		// constructions
		CValue() :type(C_INT), num(0)					{}
		CValue(int t, int n) :type(t), num(n)			{}
		CValue(int t, const char *s) :type(t), str(s)	{}
		CValue(int n) : type(C_INT), num(n)				{}
		CValue(const char* str) : type(C_STR), str(aStrdup(str))	{}
		CValue(const CValue& a)
		{
			this->type = a.type;
			switch(a.type)
			{
			case CScriptEngine::C_CONSTSTR:
				this->str = a.str;
				break;
			case CScriptEngine::C_STR:
				this->str = aStrdup(a.str);
				break;
			default:
				this->num = a.num;
				break;
			}
		}
		///////////////////////////////////////////////////////////////////////
		// destructor
		~CValue()	{ clear(); }

		///////////////////////////////////////////////////////////////////////
		// assignments
		CValue& operator=(const CValue& a)
		{
			this->clear();
			this->type = a.type;
			switch(a.type)
			{
			case CScriptEngine::C_CONSTSTR:
				this->str = a.str;
				break;
			case CScriptEngine::C_STR:
				this->str = aStrdup(a.str);
				break;
			default:
				this->num = a.num;
				break;
			}
			return *this;
		}
		CValue& operator=(int n)
		{
			clear();
			this->type = C_INT;
			this->num = n;
			return *this;
		}
		CValue& operator=(const char* s)
		{
			clear();
			this->type = (s) ? C_STR : C_CONSTSTR;
			this->str =  (s) ? aStrdup(s) : "";
			return *this;
		}

		/////////////////////////////////////////////////////////////
		// this is NOT a common shift operator
		// it "shifts" the content from one element to the other and clears the source
		CValue& operator<<(CValue& a)
		{
			this->clear();
			memcpy(this, &a, sizeof(CValue));
			a.type=C_INT;
			a.num=0;
			return *this;
		}
		///////////////////////////////////////////////////////////////////////
		static void xmove(CValue &target, CValue &source)
		{
			target.clear();
			memcpy(&target,&source,sizeof(CValue));
			source.type=C_INT;
			source.num=0;
		}
		///////////////////////////////////////////////////////////////////////
		bool isString()
		{
			return (type==CScriptEngine::C_STR || type==CScriptEngine::C_CONSTSTR);
		}
		///////////////////////////////////////////////////////////////////////
		void clear()
		{
			if( type==CScriptEngine::C_STR )
			{
				aFree( (void*)str );
				type = C_INT;
				num = 0;
			}
		}
	};
private:
	///////////////////////////////////////////////////////////////////////////
	// helper for queueing calling scripts
	// use it dynamically only
	class CCallScript : public noncopyable
	{	
	protected:
		// only CScriptEngine and derived can create and modify this class
		friend class CScriptEngine;		
		// single linked list
		CCallScript* next;
		///////////////////////////////////////////////////////////////////////
		// fully construction with automatic enqueueing
		CCallScript(CCallScript *&root, const char* s, size_t p, unsigned long r, unsigned long o)
			: next(NULL),script(s), pos(p), rid(r), oid(o)
		{	// queue the element at the end of the list starting at root
			if(!root)
				root = this;
			else
			{
				CCallScript *hook=root;
				while(hook->next) hook=hook->next;
				hook->next=this;
			}
		}
		///////////////////////////////////////////////////////////////////////
		// dequeue from root node
		static CCallScript* dequeue(CCallScript *&root)
		{	// dequeue and return the first element from the queue starting at root
			CCallScript *ret=root;
			if(ret)
			{
				root=ret->next;
				ret->next=NULL;
			}
			return ret;
		}
		virtual void setStack(size_t &def,size_t &ptr, size_t max, CValue*&stack)
		{	// dummy, used for transfering a queued stack on derived class
		}
	///////////////////////////////////////////////////////////////////////////
	public:
		virtual ~CCallScript()
		{
			if(next) delete next;
		}
		///////////////////////////////////////////////////////////////////////
		// public data
		const char* script;
		size_t pos;
		unsigned long rid;
		unsigned long oid;
	};
	///////////////////////////////////////////////////////////////////////////
	// helper for queueing calling scripts with associated stack
	// use it dynamically only
	class CCallStack : public CCallScript
	{
	public:
		size_t defsp;
		size_t stack_ptr;
		size_t stack_max;
		CValue* stack_data;
		CCallStack(CCallScript *&root, const char* s, size_t p, size_t d, unsigned long r, unsigned long o, size_t ptr, size_t max, CValue* data)
			: CCallScript(root,s,p,r,o),defsp(d),stack_ptr(ptr), stack_max(max), stack_data(data)
		{   }
		virtual ~CCallStack()
		{
			if(stack_data) delete[] stack_data;
		}
		virtual void setStack(size_t &def, size_t &ptr, size_t &max, CValue*&stack)
		{	// transfer the queued stack
			def = defsp;
			ptr = stack_ptr;
			max = stack_max;
			if(stack) delete[] stack;
			stack = stack_data;
			stack_data = NULL;
		}
	};



	///////////////////////////////////////////////////////////////////////////
	// data section
private:
	CCallScript* queue;			// script execution queue;
	size_t stack_ptr;
	size_t stack_max;
	CValue *stack_data;			// execution stack

	///////////////////////////////////////////////////////////////////////////

	static unsigned long defoid;// id of the default npc

	bool rerun_flag : 1;		// reruning line (used for inputs, menu, select and close)
	NPCSTATE npcstate : 2;		// what npc is used
	STATES state : 3;			// state of execution (externally visible states are OFF or STOP)
	unsigned _unused : 2;

	const char *script;			// the executed programm
	size_t pos;					// position within the programm
	size_t start;				// starting stack pointer for the current command
	size_t end;					// end stack pointer for the current programm (end-start) = number of parameters
	size_t defsp;				// starting sp before running the command (for checking stack violations)

	
public:
	CValue	cExtData;			// additional data from external source

	struct map_session_data* sd;// the mapsession of the caller
	unsigned long rid;			// bl.id of the hosting pc
	unsigned long oid;			// bl.id of the executed npc



	///////////////////////////////////////////////////////////////////////////
public:
	CScriptEngine() : queue(NULL), stack_ptr(0),stack_max(0),stack_data(NULL),
		rerun_flag(false), npcstate(NONE), state(OFF), script(NULL), sd(NULL)
	{ }
	CScriptEngine::~CScriptEngine()
	{
		if(queue) delete(queue);
		if(stack_data) delete[] stack_data;
	}

	void temporaty_init() //!! remove when c++ allocation is enabled
	{
		queue=NULL;
		stack_ptr=0;
		stack_max=0;
		stack_data=NULL;
		rerun_flag=false;
		state=OFF;
		script=NULL;
		sd=NULL;
	}

	///////////////////////////////////////////////////////////////////////////
	// number of parameters
	size_t Arguments()	
	{
		return (end>start)?(end-start):0; 
	}
	///////////////////////////////////////////////////////////////////////////
	// stack access
	CValue&	operator[](size_t i)
	{
		if(start+i > end || start+i >stack_max)
		{	// return a dummy value if out of range
			static CValue dummy;
			dummy.type=C_INT;
			dummy.num=0;
			return dummy;
		}
		return stack_data[start+i];
	}

	void ConvertName(CValue &data);
	int GetInt(CValue &data);
	const char* GetString(CValue &data);

	void push_val(int type, int val);
	void push_str(int type, const char *str);
	void push_copy(size_t pos);
	void pop_stack(size_t start, size_t end);
private:
	///////////////////////////////////////////////////////////////////////////
	// check stacksize and reallocate if necessary
	void alloc()
	{
		if(stack_ptr >= stack_max)
		{
			stack_max += 64;
			CValue *temp = new CValue[stack_max];
			if(stack_data)
			{
				for(size_t i=0; i<stack_ptr;i++)
					temp[i] << stack_data[i];
				delete [] stack_data;
			}
			stack_data = temp;
		}
	}

	///////////////////////////////////////////////////////////////////////////
	// operations
	void op_2int(int op);
	void op_2str(int op);
	void op_2(int op);
	void op_1(int op);
	int run_func();

	int run_main();

public:
	///////////////////////////////////////////////////////////////////////////
	// main entry points
	static int run(const char *rootscript, size_t pos, unsigned long rid, unsigned long oid);
	int restart(unsigned long npcid)
	{	
		if( this->state==STOP && (npcid == this->oid || npcid == this->defoid) )
			return CScriptEngine::run(this->script, this->pos, this->rid, this->oid);
		return 0;
	}

	///////////////////////////////////////////////////////////////////////////
	// checks the npc, create a default npc and spawn it when necessary
	unsigned long send_defaultnpc(bool send=true);

	///////////////////////////////////////////////////////////////////////////
	// status access/query
	bool isRunning()	{ return OFF!=this->state; }
	void Quit()			{ this->state = END; }
	void Stop()			{ this->state = STOP; }
	void Return()		{ this->state = RETFUNC; }
	void EnvSwap()		{ this->state = ENVSWAP; }
	void Goto(size_t p)	
	{
		pos = p; 
		state = GOTO; 
	}
	bool Rerun()
	{
		if( 0==this->rerun_flag )
		{
			this->rerun_flag = 1;
			this->state = RERUNLINE;
		}
		else
			this->rerun_flag = 0;
		return this->rerun_flag;
	}

	///////////////////////////////////////////////////////////////////////////
	// friends with access to internals
	friend int buildin_callfunc(CScriptEngine &st);
	friend int buildin_callsub(CScriptEngine &st);
	friend int buildin_getarg(CScriptEngine &st);
	friend int buildin_return(CScriptEngine &st);
	friend int buildin_menu(CScriptEngine &st);
	friend int buildin_select(CScriptEngine &st);
	friend int buildin_input(CScriptEngine &st);
	friend int buildin_setarray(CScriptEngine &st);
	friend int buildin_if(CScriptEngine &st);
};

char *parse_script(unsigned char *src, size_t line);

int set_var(const char *name, void *v);
int set_var(struct map_session_data &sd, const char *name, void *v);

struct dbt* script_get_label_db();
struct dbt* script_get_userfunc_db();

int script_config_read(const char *cfgName);
int do_init_script();
int do_final_script();

extern char mapreg_txt[256];

#endif

