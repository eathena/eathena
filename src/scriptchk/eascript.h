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
#define PT_BREAK         55 /* break */
#define PT_CASE          56 /* case */
#define PT_CHARLITERAL   57 /* CharLiteral */
#define PT_CONST         58 /* const */
#define PT_CONTINUE      59 /* continue */
#define PT_DECLITERAL    60 /* DecLiteral */
#define PT_DEFAULT       61 /* default */
#define PT_DEFINE        62 /* define */
#define PT_DO            63 /* do */
#define PT_DOUBLE        64 /* double */
#define PT_ELSE          65 /* else */
#define PT_END           66 /* end */
#define PT_FLOATLITERAL  67 /* FloatLiteral */
#define PT_FOR           68 /* for */
#define PT_FUNCTION      69 /* function */
#define PT_GOTO          70 /* goto */
#define PT_HEXLITERAL    71 /* HexLiteral */
#define PT_IDENTIFIER    72 /* identifier */
#define PT_IF            73 /* if */
#define PT_INCLUDE       74 /* include */
#define PT_INT           75 /* int */
#define PT_OCTLITERAL    76 /* OctLiteral */
#define PT_REGEXLITERAL  77 /* RegExLiteral */
#define PT_RETURN        78 /* return */
#define PT_SIZEOF        79 /* sizeof */
#define PT_STRING        80 /* string */
#define PT_STRINGLITERAL 81 /* StringLiteral */
#define PT_SWITCH        82 /* switch */
#define PT_VAR           83 /* var */
#define PT_WHILE         84 /* while */
#define PT_ACCESSFUNC    85 /* <AccessFunc> */
#define PT_ACCESSVAR     86 /* <AccessVar> */
#define PT_ARRAY         87 /* <Array> */
#define PT_BLOCK         88 /* <Block> */
#define PT_CASESTM       89 /* <Case Stm> */
#define PT_CONCAT        90 /* <Concat> */
#define PT_CONSTELEM     91 /* <Const Elem> */
#define PT_CONSTLIST     92 /* <Const List> */
#define PT_CONSTOBJ      93 /* <Const Obj> */
#define PT_CONSTUNARY    94 /* <Const Unary> */
#define PT_CONSTVALUE    95 /* <Const Value> */
#define PT_CONSTOPT      96 /* <constopt> */
#define PT_DECL          97 /* <Decl> */
#define PT_DECLS         98 /* <Decls> */
#define PT_DEFINEDECL    99 /* <Define Decl> */
#define PT_DOSTM         100 /* <Do Stm> */
#define PT_DUPLICATE     101 /* <Duplicate> */
#define PT_EVAL          102 /* <Eval> */
#define PT_EXPR          103 /* <Expr> */
#define PT_EXPRLIST      104 /* <Expr List> */
#define PT_EXPRSTM       105 /* <Expr Stm> */
#define PT_FORSTM        106 /* <For Stm> */
#define PT_FUNCCALL      107 /* <Func Call> */
#define PT_FUNCDECL      108 /* <Func Decl> */
#define PT_GOTOSTM       109 /* <Goto Stm> */
#define PT_IFSTM         110 /* <If Stm> */
#define PT_INCLUDEDECL   111 /* <Include Decl> */
#define PT_ITEM          112 /* <Item> */
#define PT_ITEMLIST      113 /* <ItemList> */
#define PT_LABELSTM      114 /* <Label Stm> */
#define PT_LCTRSTM       115 /* <LCtr Stm> */
#define PT_NORMALSTM     116 /* <Normal Stm> */
#define PT_OBJDECL       117 /* <Obj Decl> */
#define PT_OBJID         118 /* <Obj Id> */
#define PT_OBJINST       119 /* <Obj Inst> */
#define PT_OBJLIST       120 /* <Obj List> */
#define PT_OBJTYPE       121 /* <Obj Type> */
#define PT_OPADDSUB      122 /* <Op AddSub> */
#define PT_OPAND         123 /* <Op And> */
#define PT_OPASSIGN      124 /* <Op Assign> */
#define PT_OPBINAND      125 /* <Op BinAND> */
#define PT_OPBINOR       126 /* <Op BinOR> */
#define PT_OPBINXOR      127 /* <Op BinXOR> */
#define PT_OPCAST        128 /* <Op Cast> */
#define PT_OPCOMPARE     129 /* <Op Compare> */
#define PT_OPEQUATE      130 /* <Op Equate> */
#define PT_OPIF          131 /* <Op If> */
#define PT_OPMULTDIV     132 /* <Op MultDiv> */
#define PT_OPOR          133 /* <Op Or> */
#define PT_OPPOINTER     134 /* <Op Pointer> */
#define PT_OPPOST        135 /* <Op Post> */
#define PT_OPPRE         136 /* <Op Pre> */
#define PT_OPSHIFT       137 /* <Op Shift> */
#define PT_OPSIZEOF      138 /* <Op SizeOf> */
#define PT_OPUNARY       139 /* <Op Unary> */
#define PT_ORDERMOB      140 /* <order mob> */
#define PT_ORDERNPC      141 /* <order npc> */
#define PT_ORDERSCR      142 /* <order scr> */
#define PT_ORDERTOUCH    143 /* <order touch> */
#define PT_ORDERWARP     144 /* <order warp> */
#define PT_OVERWRITE     145 /* <Overwrite> */
#define PT_PAIR          146 /* <Pair> */
#define PT_PARAM         147 /* <Param> */
#define PT_PARAMS        148 /* <Params> */
#define PT_PARAMSE       149 /* <Paramse> */
#define PT_PERCENTAGE    150 /* <Percentage> */
#define PT_RANGE         151 /* <Range> */
#define PT_REGEXPR       152 /* <RegExpr> */
#define PT_RETURNSTM     153 /* <Return Stm> */
#define PT_SCOPENAME     154 /* <ScopeName> */
#define PT_SCRIPT        155 /* <Script> */
#define PT_SPECITEM      156 /* <Spec Item> */
#define PT_SPECLIST      157 /* <Spec List> */
#define PT_SPLICE        158 /* <Splice> */
#define PT_STM           159 /* <Stm> */
#define PT_STMLIST       160 /* <Stm List> */
#define PT_SUBDECL       161 /* <Sub Decl> */
#define PT_SUBPROG       162 /* <SubProg> */
#define PT_SWITCHSTM     163 /* <Switch Stm> */
#define PT_VARDECL       164 /* <Var Decl> */
#define PT_VARARRAY      165 /* <VarArray> */
#define PT_VARASSIGN     166 /* <VarAssign> */
#define PT_VARIABLE      167 /* <Variable> */
#define PT_VARLIST       168 /* <VarList> */
#define PT_VARNAME       169 /* <VarName> */
#define PT_VARTYPE       170 /* <VarType> */
#define PT_VARTYPEOPT    171 /* <VarTypeopt> */
#define PT_WHILESTM      172 /* <While Stm> */
