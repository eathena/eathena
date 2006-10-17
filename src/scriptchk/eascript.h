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
#define PT_EQEQ          47 /* '==' */
#define PT_GT            48 /* '>' */
#define PT_GTEQ          49 /* '>=' */
#define PT_GTGT          50 /* '>>' */
#define PT_GTGTEQ        51 /* '>>=' */
#define PT_ACCOUNT       52 /* account */
#define PT_AUTO          53 /* auto */
#define PT_BINLITERAL    54 /* BinLiteral */
#define PT_BREAK         55 /* break */
#define PT_CASE          56 /* case */
#define PT_CHAR          57 /* char */
#define PT_CHARLITERAL   58 /* CharLiteral */
#define PT_CONST         59 /* const */
#define PT_CONTINUE      60 /* continue */
#define PT_DECLITERAL    61 /* DecLiteral */
#define PT_DEFAULT       62 /* default */
#define PT_DEFINE        63 /* define */
#define PT_DO            64 /* do */
#define PT_DOUBLE        65 /* double */
#define PT_ELSE          66 /* else */
#define PT_END           67 /* end */
#define PT_EXTERN        68 /* extern */
#define PT_FLOATLITERAL  69 /* FloatLiteral */
#define PT_FOR           70 /* for */
#define PT_GLOBAL        71 /* global */
#define PT_GOTO          72 /* goto */
#define PT_GUILD         73 /* guild */
#define PT_HEXLITERAL    74 /* HexLiteral */
#define PT_IDENTIFIER    75 /* identifier */
#define PT_IF            76 /* if */
#define PT_INCLUDE       77 /* include */
#define PT_INT           78 /* int */
#define PT_PARTY         79 /* party */
#define PT_RETURN        80 /* return */
#define PT_SIZEOF        81 /* sizeof */
#define PT_STRING        82 /* string */
#define PT_STRINGLITERAL 83 /* StringLiteral */
#define PT_SWITCH        84 /* switch */
#define PT_TEMP          85 /* temp */
#define PT_VAR           86 /* var */
#define PT_WHILE         87 /* while */
#define PT_ACCESSFUNC    88 /* <AccessFunc> */
#define PT_ACCESSVAR     89 /* <AccessVar> */
#define PT_ARRAY         90 /* <Array> */
#define PT_BLOCK         91 /* <Block> */
#define PT_CASESTM       92 /* <Case Stm> */
#define PT_CONCAT        93 /* <Concat> */
#define PT_CONDITION     94 /* <Condition> */
#define PT_CONSTELEM     95 /* <Const Elem> */
#define PT_CONSTLIST     96 /* <Const List> */
#define PT_CONSTOBJ      97 /* <Const Obj> */
#define PT_CONSTUNARY    98 /* <Const Unary> */
#define PT_CONSTVALUE    99 /* <Const Value> */
#define PT_CONSTE        100 /* <conste> */
#define PT_DECL          101 /* <Decl> */
#define PT_DECLS         102 /* <Decls> */
#define PT_DEFINE2       103 /* <Define> */
#define PT_DUPLICATE     104 /* <Duplicate> */
#define PT_EVAL          105 /* <Eval> */
#define PT_EXPR          106 /* <Expr> */
#define PT_EXPRLIST      107 /* <Expr List> */
#define PT_FUNCDECL      108 /* <Func Decl> */
#define PT_FUNCTION      109 /* <Function> */
#define PT_GOTOSTM       110 /* <Goto Stm> */
#define PT_INCLUDE2      111 /* <Include> */
#define PT_ITEM          112 /* <Item> */
#define PT_ITEMLIST      113 /* <ItemList> */
#define PT_LABELSTM      114 /* <Label Stm> */
#define PT_LCTRSTM       115 /* <LCtr Stm> */
#define PT_MOD           116 /* <Mod> */
#define PT_MODE          117 /* <Mode> */
#define PT_NORMALSTM     118 /* <Normal Stm> */
#define PT_OBJDECL       119 /* <Obj Decl> */
#define PT_OBJID         120 /* <Obj Id> */
#define PT_OBJINST       121 /* <Obj Inst> */
#define PT_OBJLIST       122 /* <Obj List> */
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
#define PT_OVERWRITE     147 /* <Overwrite> */
#define PT_PAIR          148 /* <Pair> */
#define PT_PARAM         149 /* <Param> */
#define PT_PARAMS        150 /* <Params> */
#define PT_PARAMSE       151 /* <Paramse> */
#define PT_PERCENTAGE    152 /* <Percentage> */
#define PT_RANGE         153 /* <Range> */
#define PT_RETURNSTM     154 /* <Return Stm> */
#define PT_SCALAR        155 /* <Scalar> */
#define PT_SCALARE       156 /* <Scalare> */
#define PT_SCRIPT        157 /* <Script> */
#define PT_SPECITEM      158 /* <Spec Item> */
#define PT_SPECLIST      159 /* <Spec List> */
#define PT_SPLICE        160 /* <Splice> */
#define PT_STM           161 /* <Stm> */
#define PT_STMLIST       162 /* <Stm List> */
#define PT_SUBPROG       163 /* <SubProg> */
#define PT_TYPE          164 /* <Type> */
#define PT_VAR2          165 /* <Var> */
#define PT_VARDECL       166 /* <Var Decl> */
#define PT_VARLIST       167 /* <Var List> */
#define PT_VARARRAY      168 /* <VarArray> */
#define PT_VARASSIGN     169 /* <VarAssign> */
