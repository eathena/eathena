#define PT_EOF           0 /* (EOF) */
#define PT_ERROR         1 /* (Error) */
#define PT_WHITESPACE    2 /* (Whitespace) */
#define PT_COMMENTEND    3 /* (Comment End) */
#define PT_COMMENTLINE   4 /* (Comment Line) */
#define PT_COMMENTSTART  5 /* (Comment Start) */
#define PT_MINUS         6 /* '-' */
#define PT_MINUSMINUS    7 /* '--' */
#define PT_EXCLAM        8 /* '!' */
#define PT_EXCLAMEQ      9 /* '!=' */
#define PT_PERCENT       10 /* '%' */
#define PT_AMP           11 /* '&' */
#define PT_AMPAMP        12 /* '&&' */
#define PT_AMPEQ         13 /* '&=' */
#define PT_LPARAN        14 /* '(' */
#define PT_RPARAN        15 /* ')' */
#define PT_TIMES         16 /* '*' */
#define PT_TIMESEQ       17 /* '*=' */
#define PT_COMMA         18 /* ',' */
#define PT_DOT           19 /* '.' */
#define PT_DIV           20 /* '/' */
#define PT_DIVEQ         21 /* '/=' */
#define PT_COLON         22 /* ':' */
#define PT_COLONTIMES    23 /* ':*' */
#define PT_COLONCOLON    24 /* '::' */
#define PT_SEMI          25 /* ';' */
#define PT_QUESTION      26 /* '?' */
#define PT_LBRACKET      27 /* '[' */
#define PT_RBRACKET      28 /* ']' */
#define PT_CARET         29 /* '^' */
#define PT_CARETEQ       30 /* '^=' */
#define PT_LBRACE        31 /* '{' */
#define PT_PIPE          32 /* '|' */
#define PT_PIPEPIPE      33 /* '||' */
#define PT_PIPEEQ        34 /* '|=' */
#define PT_RBRACE        35 /* '}' */
#define PT_TILDE         36 /* '~' */
#define PT_PLUS          37 /* '+' */
#define PT_PLUSPLUS      38 /* '++' */
#define PT_PLUSEQ        39 /* '+=' */
#define PT_LT            40 /* '<' */
#define PT_LTLT          41 /* '<<' */
#define PT_LTLTEQ        42 /* '<<=' */
#define PT_LTEQ          43 /* '<=' */
#define PT_LTGT          44 /* '<>' */
#define PT_EQ            45 /* '=' */
#define PT_MINUSEQ       46 /* '-=' */
#define PT_EQTILDE       47 /* '=~' */
#define PT_EQEQ          48 /* '==' */
#define PT_GT            49 /* '>' */
#define PT_GTEQ          50 /* '>=' */
#define PT_GTGT          51 /* '>>' */
#define PT_GTGTEQ        52 /* '>>=' */
#define PT_AUTO          53 /* auto */
#define PT_BINLITERAL    54 /* BinLiteral */
#define PT_BOOLFALSE     55 /* boolfalse */
#define PT_BOOLTRUE      56 /* booltrue */
#define PT_BREAK         57 /* break */
#define PT_CASE          58 /* case */
#define PT_CHARLITERAL   59 /* CharLiteral */
#define PT_CONST         60 /* const */
#define PT_CONTINUE      61 /* continue */
#define PT_DECLITERAL    62 /* DecLiteral */
#define PT_DEFAULT       63 /* default */
#define PT_DEFINE        64 /* define */
#define PT_DO            65 /* do */
#define PT_DOUBLE        66 /* double */
#define PT_ELSE          67 /* else */
#define PT_END           68 /* end */
#define PT_FLOATLITERAL  69 /* FloatLiteral */
#define PT_FOR           70 /* for */
#define PT_FUNCTION      71 /* function */
#define PT_GOSUB         72 /* gosub */
#define PT_GOTO          73 /* goto */
#define PT_HEXLITERAL    74 /* HexLiteral */
#define PT_IDENTIFIER    75 /* identifier */
#define PT_IF            76 /* if */
#define PT_INCLUDE       77 /* include */
#define PT_INT           78 /* int */
#define PT_OCTLITERAL    79 /* OctLiteral */
#define PT_REGEXLITERAL  80 /* RegExLiteral */
#define PT_RETURN        81 /* return */
#define PT_SIZEOF        82 /* sizeof */
#define PT_STRING        83 /* string */
#define PT_STRINGLITERAL 84 /* StringLiteral */
#define PT_SWITCH        85 /* switch */
#define PT_VAR           86 /* var */
#define PT_WHILE         87 /* while */
#define PT_ARRAY         88 /* <Array> */
#define PT_BLOCK         89 /* <Block> */
#define PT_CASESTM       90 /* <Case Stm> */
#define PT_CONCAT        91 /* <Concat> */
#define PT_CONSTVALUE    92 /* <Const Value> */
#define PT_CONSTOPT      93 /* <constopt> */
#define PT_DECL          94 /* <Decl> */
#define PT_DECLS         95 /* <Decls> */
#define PT_DEFINEDECL    96 /* <Define Decl> */
#define PT_DOSTM         97 /* <Do Stm> */
#define PT_DUPLICATE     98 /* <Duplicate> */
#define PT_EVAL          99 /* <Eval> */
#define PT_EXPR          100 /* <Expr> */
#define PT_EXPRLIST      101 /* <Expr List> */
#define PT_EXPRSTM       102 /* <Expr Stm> */
#define PT_FORSTM        103 /* <For Stm> */
#define PT_FUNCCALL      104 /* <Func Call> */
#define PT_FUNCDECL      105 /* <Func Decl> */
#define PT_GOSUBSTM      106 /* <Gosub Stm> */
#define PT_GOTOSTM       107 /* <Goto Stm> */
#define PT_IFSTM         108 /* <If Stm> */
#define PT_INCLUDEDECL   109 /* <Include Decl> */
#define PT_ITEM          110 /* <Item> */
#define PT_ITEMLIST      111 /* <ItemList> */
#define PT_ITEMOVERWRITE 112 /* <ItemOverwrite> */
#define PT_LABELSTM      113 /* <Label Stm> */
#define PT_LCTRSTM       114 /* <LCtr Stm> */
#define PT_MEMBERFUNC    115 /* <MemberFunc> */
#define PT_MEMBERVAR     116 /* <MemberVar> */
#define PT_NORMALSTM     117 /* <Normal Stm> */
#define PT_OBJDECL       118 /* <Obj Decl> */
#define PT_OBJID         119 /* <Obj Id> */
#define PT_OBJINST       120 /* <Obj Inst> */
#define PT_OBJLIST       121 /* <Obj List> */
#define PT_OBJSCRIPT     122 /* <Obj Script> */
#define PT_OBJTYPE       123 /* <Obj Type> */
#define PT_OPADDSUB      124 /* <Op AddSub> */
#define PT_OPAND         125 /* <Op And> */
#define PT_OPASSIGN      126 /* <Op Assign> */
#define PT_OPBINAND      127 /* <Op BinAND> */
#define PT_OPBINOR       128 /* <Op BinOR> */
#define PT_OPBINXOR      129 /* <Op BinXOR> */
#define PT_OPCAST        130 /* <Op Cast> */
#define PT_OPCOMPARE     131 /* <Op Compare> */
#define PT_OPEQUATE      132 /* <Op Equate> */
#define PT_OPIF          133 /* <Op If> */
#define PT_OPMULTDIV     134 /* <Op MultDiv> */
#define PT_OPOR          135 /* <Op Or> */
#define PT_OPPOINTER     136 /* <Op Pointer> */
#define PT_OPPOST        137 /* <Op Post> */
#define PT_OPPRE         138 /* <Op Pre> */
#define PT_OPSHIFT       139 /* <Op Shift> */
#define PT_OPSIZEOF      140 /* <Op SizeOf> */
#define PT_OPUNARY       141 /* <Op Unary> */
#define PT_ORDERMOB      142 /* <order mob> */
#define PT_ORDERNPC      143 /* <order npc> */
#define PT_ORDERSCR      144 /* <order scr> */
#define PT_ORDERTOUCH    145 /* <order touch> */
#define PT_ORDERWARP     146 /* <order warp> */
#define PT_PARAM         147 /* <Param> */
#define PT_PARAMS        148 /* <Params> */
#define PT_PARAMSE       149 /* <Paramse> */
#define PT_RANGE         150 /* <Range> */
#define PT_REGEXPR       151 /* <RegExpr> */
#define PT_RETURNSTM     152 /* <Return Stm> */
#define PT_SCOPEFUNC     153 /* <ScopeFunc> */
#define PT_SCOPEVAR      154 /* <ScopeVar> */
#define PT_SPECITEM      155 /* <Spec Item> */
#define PT_SPECLIST      156 /* <Spec List> */
#define PT_SPLICE        157 /* <Splice> */
#define PT_STM           158 /* <Stm> */
#define PT_STMLIST       159 /* <Stm List> */
#define PT_SUBDECL       160 /* <Sub Decl> */
#define PT_SUBPROG       161 /* <SubProg> */
#define PT_SWITCHSTM     162 /* <Switch Stm> */
#define PT_VARDECL       163 /* <Var Decl> */
#define PT_VARASSIGN     164 /* <VarAssign> */
#define PT_VARIABLE      165 /* <Variable> */
#define PT_VARLIST       166 /* <VarList> */
#define PT_VARTYPE       167 /* <VarType> */
#define PT_VARTYPEOPT    168 /* <VarTypeopt> */
#define PT_WHILESTM      169 /* <While Stm> */
