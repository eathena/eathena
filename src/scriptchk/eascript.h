#define PT_EOF           0 /* (EOF) */
#define PT_ERROR         1 /* (Error) */
#define PT_WHITESPACE    2 /* (Whitespace) */
#define PT_COMMENTEND    3 /* (Comment End) */
#define PT_COMMENTLINE   4 /* (Comment Line) */
#define PT_COMMENTSTART  5 /* (Comment Start) */
#define PT_MINUS         6 /* - */
#define PT_MINUSMINUS    7 /* -- */
#define PT_EXCLAM        8 /* '!' */
#define PT_EXCLAMEQ      9 /* '!=' */
#define PT_PERCENT       10 /* '%' */
#define PT_PERCENTEQ     11 /* '%=' */
#define PT_AMP           12 /* '&' */
#define PT_AMPAMP        13 /* '&&' */
#define PT_AMPEQ         14 /* '&=' */
#define PT_LPARAN        15 /* '(' */
#define PT_RPARAN        16 /* ')' */
#define PT_TIMES         17 /* '*' */
#define PT_TIMESEQ       18 /* '*=' */
#define PT_COMMA         19 /* ',' */
#define PT_DOT           20 /* '.' */
#define PT_DIV           21 /* '/' */
#define PT_DIVEQ         22 /* '/=' */
#define PT_COLON         23 /* ':' */
#define PT_COLONTIMES    24 /* ':*' */
#define PT_COLONCOLON    25 /* '::' */
#define PT_SEMI          26 /* ';' */
#define PT_QUESTION      27 /* '?' */
#define PT_LBRACKET      28 /* '[' */
#define PT_RBRACKET      29 /* ']' */
#define PT_CARET         30 /* '^' */
#define PT_CARETEQ       31 /* '^=' */
#define PT_LBRACE        32 /* '{' */
#define PT_PIPE          33 /* '|' */
#define PT_PIPEPIPE      34 /* '||' */
#define PT_PIPEEQ        35 /* '|=' */
#define PT_RBRACE        36 /* '}' */
#define PT_TILDE         37 /* '~' */
#define PT_PLUS          38 /* '+' */
#define PT_PLUSPLUS      39 /* '++' */
#define PT_PLUSEQ        40 /* '+=' */
#define PT_LT            41 /* '<' */
#define PT_LTLT          42 /* '<<' */
#define PT_LTLTEQ        43 /* '<<=' */
#define PT_LTEQ          44 /* '<=' */
#define PT_LTGT          45 /* '<>' */
#define PT_EQ            46 /* '=' */
#define PT_MINUSEQ       47 /* '-=' */
#define PT_EQTILDE       48 /* '=~' */
#define PT_EQEQ          49 /* '==' */
#define PT_GT            50 /* '>' */
#define PT_GTEQ          51 /* '>=' */
#define PT_GTGT          52 /* '>>' */
#define PT_GTGTEQ        53 /* '>>=' */
#define PT_AUTO          54 /* auto */
#define PT_BINLITERAL    55 /* BinLiteral */
#define PT_BOOLFALSE     56 /* boolfalse */
#define PT_BOOLTRUE      57 /* booltrue */
#define PT_BREAK         58 /* break */
#define PT_CASE          59 /* case */
#define PT_CHARLITERAL   60 /* CharLiteral */
#define PT_CONST         61 /* const */
#define PT_CONTINUE      62 /* continue */
#define PT_DECLITERAL    63 /* DecLiteral */
#define PT_DEFAULT       64 /* default */
#define PT_DEFINE        65 /* define */
#define PT_DO            66 /* do */
#define PT_DOUBLE        67 /* double */
#define PT_ELSE          68 /* else */
#define PT_END           69 /* end */
#define PT_FLOATLITERAL  70 /* FloatLiteral */
#define PT_FOR           71 /* for */
#define PT_FUNCTION      72 /* function */
#define PT_GOSUB         73 /* gosub */
#define PT_GOTO          74 /* goto */
#define PT_HEXLITERAL    75 /* HexLiteral */
#define PT_IDENTIFIER    76 /* identifier */
#define PT_IF            77 /* if */
#define PT_INCLUDE       78 /* include */
#define PT_INT           79 /* int */
#define PT_OCTLITERAL    80 /* OctLiteral */
#define PT_REGEXLITERAL  81 /* RegExLiteral */
#define PT_RETURN        82 /* return */
#define PT_SIZEOF        83 /* sizeof */
#define PT_STRING        84 /* string */
#define PT_STRINGLITERAL 85 /* StringLiteral */
#define PT_SWITCH        86 /* switch */
#define PT_VAR           87 /* var */
#define PT_WHILE         88 /* while */
#define PT_ARRAY         89 /* <Array> */
#define PT_BLOCK         90 /* <Block> */
#define PT_CASESTM       91 /* <Case Stm> */
#define PT_CONCAT        92 /* <Concat> */
#define PT_CONSTVALUE    93 /* <Const Value> */
#define PT_CONSTOPT      94 /* <constopt> */
#define PT_DECL          95 /* <Decl> */
#define PT_DECLS         96 /* <Decls> */
#define PT_DEFINEDECL    97 /* <Define Decl> */
#define PT_DOSTM         98 /* <Do Stm> */
#define PT_DUPLICATE     99 /* <Duplicate> */
#define PT_EVAL          100 /* <Eval> */
#define PT_EXPR          101 /* <Expr> */
#define PT_EXPRLIST      102 /* <Expr List> */
#define PT_EXPRSTM       103 /* <Expr Stm> */
#define PT_FORSTM        104 /* <For Stm> */
#define PT_FUNCCALL      105 /* <Func Call> */
#define PT_FUNCDECL      106 /* <Func Decl> */
#define PT_GLOBALFUNC    107 /* <GlobalFunc> */
#define PT_GOSUBSTM      108 /* <Gosub Stm> */
#define PT_GOTOSTM       109 /* <Goto Stm> */
#define PT_IFSTM         110 /* <If Stm> */
#define PT_INCLUDEDECL   111 /* <Include Decl> */
#define PT_ITEM          112 /* <Item> */
#define PT_ITEMLIST      113 /* <ItemList> */
#define PT_ITEMOVERWRITE 114 /* <ItemOverwrite> */
#define PT_LABELSTM      115 /* <Label Stm> */
#define PT_LCTRSTM       116 /* <LCtr Stm> */
#define PT_MEMBERFUNC    117 /* <MemberFunc> */
#define PT_MEMBERVAR     118 /* <MemberVar> */
#define PT_NORMALSTM     119 /* <Normal Stm> */
#define PT_OBJDECL       120 /* <Obj Decl> */
#define PT_OBJID         121 /* <Obj Id> */
#define PT_OBJINST       122 /* <Obj Inst> */
#define PT_OBJLIST       123 /* <Obj List> */
#define PT_OBJSCRIPT     124 /* <Obj Script> */
#define PT_OBJTYPE       125 /* <Obj Type> */
#define PT_OPADDSUB      126 /* <Op AddSub> */
#define PT_OPAND         127 /* <Op And> */
#define PT_OPASSIGN      128 /* <Op Assign> */
#define PT_OPBINAND      129 /* <Op BinAND> */
#define PT_OPBINOR       130 /* <Op BinOR> */
#define PT_OPBINXOR      131 /* <Op BinXOR> */
#define PT_OPCAST        132 /* <Op Cast> */
#define PT_OPCOMPARE     133 /* <Op Compare> */
#define PT_OPEQUATE      134 /* <Op Equate> */
#define PT_OPIF          135 /* <Op If> */
#define PT_OPMULTDIV     136 /* <Op MultDiv> */
#define PT_OPOR          137 /* <Op Or> */
#define PT_OPPOINTER     138 /* <Op Pointer> */
#define PT_OPPOST        139 /* <Op Post> */
#define PT_OPPRE         140 /* <Op Pre> */
#define PT_OPSHIFT       141 /* <Op Shift> */
#define PT_OPSIZEOF      142 /* <Op SizeOf> */
#define PT_OPUNARY       143 /* <Op Unary> */
#define PT_ORDERMOB      144 /* <order mob> */
#define PT_ORDERNPC      145 /* <order npc> */
#define PT_ORDERSCR      146 /* <order scr> */
#define PT_ORDERTOUCH    147 /* <order touch> */
#define PT_ORDERWARP     148 /* <order warp> */
#define PT_PARAM         149 /* <Param> */
#define PT_PARAMS        150 /* <Params> */
#define PT_PARAMSE       151 /* <Paramse> */
#define PT_RANGE         152 /* <Range> */
#define PT_REGEXPR       153 /* <RegExpr> */
#define PT_RETURNSTM     154 /* <Return Stm> */
#define PT_SCOPEFUNC     155 /* <ScopeFunc> */
#define PT_SCOPEVAR      156 /* <ScopeVar> */
#define PT_SPECITEM      157 /* <Spec Item> */
#define PT_SPECLIST      158 /* <Spec List> */
#define PT_SPLICE        159 /* <Splice> */
#define PT_STM           160 /* <Stm> */
#define PT_STMLIST       161 /* <Stm List> */
#define PT_SUBDECL       162 /* <Sub Decl> */
#define PT_SUBPROG       163 /* <SubProg> */
#define PT_SWITCHSTM     164 /* <Switch Stm> */
#define PT_VARDECL       165 /* <Var Decl> */
#define PT_VARASSIGN     166 /* <VarAssign> */
#define PT_VARIABLE      167 /* <Variable> */
#define PT_VARLIST       168 /* <VarList> */
#define PT_VARTYPE       169 /* <VarType> */
#define PT_VARTYPEOPT    170 /* <VarTypeopt> */
#define PT_WHILESTM      171 /* <While Stm> */
