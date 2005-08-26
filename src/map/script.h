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




// simple class only since we use c-style allocation and clearing at the moment
class CScriptEngine
{
public:
	///////////////////////////////////////////////////////////////////////////
	// ŽÀsŒn
	enum { OFF=0,STOP=1,END,RERUNLINE,GOTO,RETFUNC }; //0...5 ->3bit

	enum
	{
		C_NOP=0,C_POS,C_INT,C_PARAM,C_FUNC,C_STR,C_CONSTSTR,C_ARG,
		C_NAME,C_EOL, C_RETINFO,

		C_LOR,C_LAND,C_LE,C_LT,C_GE,C_GT,C_EQ,C_NE,   //operator
		C_XOR,C_OR,C_AND,C_ADD,C_SUB,C_MUL,C_DIV,C_MOD,C_NEG,C_LNOT,C_NOT,C_R_SHIFT,C_L_SHIFT
	};

	///////////////////////////////////////////////////////////////////////////
	// helper for queueing calling scripts
	// use it dynamically only
	class CCallQueue : public noncopyable
	{	// only CScriptEngine can create and modify this class
		friend class CScriptEngine;		
		// single linked list
		CCallQueue* next;
		///////////////////////////////////////////////////////////////////////
		// fully construction with automatic enqueueing
		CCallQueue(CCallQueue *&root, const char* s, size_t p, unsigned long r, unsigned long o)
			: next(NULL),script(s), pos(p), rid(r), oid(o)
		{	// queue the element at the end of the list starting at root
			if(!root)
				root = this;
			else
			{
				CCallQueue *hook=root;
				while(hook->next) hook=hook->next;
				hook->next=this;
			}
		}
		///////////////////////////////////////////////////////////////////////
		// dequeue from root node
		static CCallQueue* dequeue(CCallQueue *&root)
		{	// dequeue and return the first element from the queue starting at root
			CCallQueue *ret=root;
			if(ret)
			{
				root=ret->next;
				ret->next=NULL;
			}
			return ret;
		}
	///////////////////////////////////////////////////////////////////////////
	public:
		~CCallQueue()
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
	// simple Variant type with 2 cpmponents
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
		CValue() :type(C_INT), num(0)					{}
		CValue(int t, int n) :type(t), num(n)			{}
		CValue(int t, const char *s) :type(t), str(s)	{}
		~CValue()										{}	//!! add proper pointer deallocation
		///////////////////////////////////////////////////////////////////////
		bool isString()
		{
			return (type==CScriptEngine::C_STR || type==CScriptEngine::C_CONSTSTR);
		}
	};
	///////////////////////////////////////////////////////////////////////////
	// stack implementation for the stack machine
	class CStack
	{
	public:
		size_t sp;
		size_t sp_max;
		CValue *stack_data;

		CStack() : sp(0),sp_max(0),stack_data(NULL)	{}
		~CStack()	{ clear(); }
		void clear();
	};

	///////////////////////////////////////////////////////////////////////////
private:
	CCallQueue* queue;			// script execution queue;
	CStack stack;				// execution stack
	///////////////////////////////////////////////////////////////////////////
public:
	bool rerun_flag : 1;		// reruning line (used for inputs, menu, select and close)
	unsigned state : 3;			// state of the execution (externally visible states are OFF or STOP/RERUNLINE for scripts waiting for input)

	const char *script;			// the programm
	size_t pos;					// position within the programm
	size_t start;				// starting sp for the current command
	size_t end;					// end sp for the current programm (end-start) = number of parameters
	size_t defsp;				// starting sp before running the command (for checking stack violations)

	struct map_session_data* sd;// the mapsession of the caller
	unsigned long rid;			// bl.id of the hosting pc
	unsigned long oid;			// bl.id of the executed npc


	///////////////////////////////////////////////////////////////////////////
public:
	CScriptEngine() : queue(NULL), rerun_flag(false), state(OFF), script(NULL), sd(NULL)
	{  }
	~CScriptEngine()
	{
		stack.clear();			//!! will be an automatic destruct later
		if(queue) delete(queue);
	}
	///////////////////////////////////////////////////////////////////////////
	// number of parameters
	size_t Parameters()	
	{
		return (end>start)?(end-start):0; 
	}
	///////////////////////////////////////////////////////////////////////////
	// stack access
	CValue&	operator[](size_t i)
	{
		if(start+i > end)
		{	// return a dummy value if out of range
			static CValue dummy;
			dummy.type=C_INT;
			dummy.num=0;
			return dummy;
		}
		return stack.stack_data[start+i]; //!! add necessary operators to stackdata
	}
	CValue&	getDirectData(size_t i)
	{
		if( i>=stack.sp_max )
		{	// return a dummy value if out of range
			static CValue dummy;
			dummy.type=C_INT;
			dummy.num=0;
			return dummy;
		}
		return stack.stack_data[i]; //!! add necessary operators to stackdata
	}
	void ConvertName(CValue &data);
	int GetInt(CValue &data);
	const char* GetString(CValue &data);

	void push_val(int type, int val);
	void push_str(int type, const char *str);
	void push_copy(size_t pos);
	void pop_stack(size_t start, size_t end);
	int pop_val();

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
	// main entry point
	static int run(const char *rootscript, size_t pos, unsigned long rid, unsigned long oid);
	
	///////////////////////////////////////////////////////////////////////////
	// script queueing
	void enqueue(const char* script, size_t pos, unsigned long rid, unsigned long oid)
	{	// will be queued automatically
		new CScriptEngine::CCallQueue(queue, script, pos, rid, oid);
	}
	CScriptEngine::CCallQueue* dequeue()
	{	// will be dequeued automatically
		return CCallQueue::dequeue(queue);
	}
	///////////////////////////////////////////////////////////////////////////
	// status query
	bool isRunning()	{ return 0!=state; }
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

