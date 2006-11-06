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
#define PT_AUTO          52 /* auto */
#define PT_BINLITERAL    53 /* BinLiteral */
#define PT_BREAK         54 /* break */
#define PT_CASE          55 /* case */
#define PT_CHARLITERAL   56 /* CharLiteral */
#define PT_CONST         57 /* const */
#define PT_CONTINUE      58 /* continue */
#define PT_DECLITERAL    59 /* DecLiteral */
#define PT_DEFAULT       60 /* default */
#define PT_DEFINE        61 /* define */
#define PT_DO            62 /* do */
#define PT_DOUBLE        63 /* double */
#define PT_ELSE          64 /* else */
#define PT_END           65 /* end */
#define PT_FLOATLITERAL  66 /* FloatLiteral */
#define PT_FOR           67 /* for */
#define PT_FUNCTION      68 /* function */
#define PT_GOTO          69 /* goto */
#define PT_HEXLITERAL    70 /* HexLiteral */
#define PT_IDENTIFIER    71 /* identifier */
#define PT_IF            72 /* if */
#define PT_INCLUDE       73 /* include */
#define PT_INT           74 /* int */
#define PT_RETURN        75 /* return */
#define PT_SIZEOF        76 /* sizeof */
#define PT_STRING        77 /* string */
#define PT_STRINGLITERAL 78 /* StringLiteral */
#define PT_SWITCH        79 /* switch */
#define PT_VAR           80 /* var */
#define PT_WHILE         81 /* while */
#define PT_ACCESSFUNC    82 /* <AccessFunc> */
#define PT_ACCESSVAR     83 /* <AccessVar> */
#define PT_ARRAY         84 /* <Array> */
#define PT_BLOCK         85 /* <Block> */
#define PT_CASESTM       86 /* <Case Stm> */
#define PT_CONCAT        87 /* <Concat> */
#define PT_CONDITION     88 /* <Condition> */
#define PT_CONSTELEM     89 /* <Const Elem> */
#define PT_CONSTLIST     90 /* <Const List> */
#define PT_CONSTOBJ      91 /* <Const Obj> */
#define PT_CONSTUNARY    92 /* <Const Unary> */
#define PT_CONSTVALUE    93 /* <Const Value> */
#define PT_CONSTE        94 /* <conste> */
#define PT_DECL          95 /* <Decl> */
#define PT_DECLS         96 /* <Decls> */
#define PT_DEFINE2       97 /* <Define> */
#define PT_DUPLICATE     98 /* <Duplicate> */
#define PT_EVAL          99 /* <Eval> */
#define PT_EXPR          100 /* <Expr> */
#define PT_EXPRLIST      101 /* <Expr List> */
#define PT_FUNCDECL      102 /* <Func Decl> */
#define PT_FUNCTION2     103 /* <Function> */
#define PT_GOTOSTM       104 /* <Goto Stm> */
#define PT_INCLUDE2      105 /* <Include> */
#define PT_ITEM          106 /* <Item> */
#define PT_ITEMLIST      107 /* <ItemList> */
#define PT_LABELSTM      108 /* <Label Stm> */
#define PT_LCTRSTM       109 /* <LCtr Stm> */
#define PT_NORMALSTM     110 /* <Normal Stm> */
#define PT_OBJDECL       111 /* <Obj Decl> */
#define PT_OBJID         112 /* <Obj Id> */
#define PT_OBJINST       113 /* <Obj Inst> */
#define PT_OBJLIST       114 /* <Obj List> */
#define PT_OBJTYPE       115 /* <Obj Type> */
#define PT_OPADDSUB      116 /* <Op AddSub> */
#define PT_OPAND         117 /* <Op And> */
#define PT_OPASSIGN      118 /* <Op Assign> */
#define PT_OPBINAND      119 /* <Op BinAND> */
#define PT_OPBINOR       120 /* <Op BinOR> */
#define PT_OPBINXOR      121 /* <Op BinXOR> */
#define PT_OPCAST        122 /* <Op Cast> */
#define PT_OPCOMPARE     123 /* <Op Compare> */
#define PT_OPEQUATE      124 /* <Op Equate> */
#define PT_OPIF          125 /* <Op If> */
#define PT_OPMULTDIV     126 /* <Op MultDiv> */
#define PT_OPOR          127 /* <Op Or> */
#define PT_OPPOINTER     128 /* <Op Pointer> */
#define PT_OPPOST        129 /* <Op Post> */
#define PT_OPPRE         130 /* <Op Pre> */
#define PT_OPSHIFT       131 /* <Op Shift> */
#define PT_OPSIZEOF      132 /* <Op SizeOf> */
#define PT_OPUNARY       133 /* <Op Unary> */
#define PT_ORDERMOB      134 /* <order mob> */
#define PT_ORDERNPC      135 /* <order npc> */
#define PT_ORDERSCR      136 /* <order scr> */
#define PT_ORDERTOUCH    137 /* <order touch> */
#define PT_ORDERWARP     138 /* <order warp> */
#define PT_OVERWRITE     139 /* <Overwrite> */
#define PT_PAIR          140 /* <Pair> */
#define PT_PARAM         141 /* <Param> */
#define PT_PARAMS        142 /* <Params> */
#define PT_PARAMSE       143 /* <Paramse> */
#define PT_PERCENTAGE    144 /* <Percentage> */
#define PT_RANGE         145 /* <Range> */
#define PT_RETURNSTM     146 /* <Return Stm> */
#define PT_SCALAR        147 /* <Scalar> */
#define PT_SCALARE       148 /* <Scalare> */
#define PT_SCOPENAME     149 /* <ScopeName> */
#define PT_SCRIPT        150 /* <Script> */
#define PT_SPECITEM      151 /* <Spec Item> */
#define PT_SPECLIST      152 /* <Spec List> */
#define PT_SPLICE        153 /* <Splice> */
#define PT_STM           154 /* <Stm> */
#define PT_STMLIST       155 /* <Stm List> */
#define PT_SUBDECL       156 /* <Sub Decl> */
#define PT_SUBPROG       157 /* <SubProg> */
#define PT_VAR2          158 /* <Var> */
#define PT_VARDECL       159 /* <Var Decl> */
#define PT_VARLIST       160 /* <Var List> */
#define PT_VARARRAY      161 /* <VarArray> */
#define PT_VARASSIGN     162 /* <VarAssign> */
#define PT_VARNAME       163 /* <VarName> */
