#define PT_EOF              0 /* (EOF) */
#define PT_ERROR            1 /* (Error) */
#define PT_WHITESPACE       2 /* (Whitespace) */
#define PT_COMMENTEND       3 /* (Comment End) */
#define PT_COMMENTLINE      4 /* (Comment Line) */
#define PT_COMMENTSTART     5 /* (Comment Start) */
#define PT_MINUS            6 /* '-' */
#define PT_EXCLAM           7 /* '!' */
#define PT_EXCLAMEQ         8 /* '!=' */
#define PT_PERCENT          9 /* '%' */
#define PT_AMP              10 /* '&' */
#define PT_AMPAMP           11 /* '&&' */
#define PT_AMPEQ            12 /* '&=' */
#define PT_LPARAN           13 /* '(' */
#define PT_RPARAN           14 /* ')' */
#define PT_TIMES            15 /* '*' */
#define PT_TIMESEQ          16 /* '*=' */
#define PT_COMMA            17 /* ',' */
#define PT_DOT              18 /* '.' */
#define PT_DIV              19 /* '/' */
#define PT_DIVEQ            20 /* '/=' */
#define PT_COLON            21 /* ':' */
#define PT_SEMI             22 /* ';' */
#define PT_QUESTION         23 /* '?' */
#define PT_LBRACKET         24 /* '[' */
#define PT_RBRACKET         25 /* ']' */
#define PT_CARET            26 /* '^' */
#define PT_CARETEQ          27 /* '^=' */
#define PT_LBRACE           28 /* '{' */
#define PT_PIPE             29 /* '|' */
#define PT_PIPEPIPE         30 /* '||' */
#define PT_PIPEEQ           31 /* '|=' */
#define PT_RBRACE           32 /* '}' */
#define PT_TILDE            33 /* '~' */
#define PT_PLUS             34 /* '+' */
#define PT_PLUSEQ           35 /* '+=' */
#define PT_LT               36 /* '<' */
#define PT_LTLT             37 /* '<<' */
#define PT_LTLTEQ           38 /* '<<=' */
#define PT_LTEQ             39 /* '<=' */
#define PT_EQ               40 /* '=' */
#define PT_MINUSEQ          41 /* '-=' */
#define PT_EQEQ             42 /* '==' */
#define PT_GT               43 /* '>' */
#define PT_GTEQ             44 /* '>=' */
#define PT_GTGT             45 /* '>>' */
#define PT_GTGTEQ           46 /* '>>=' */
#define PT_BREAK            47 /* break */
#define PT_CASE             48 /* case */
#define PT_CHARLITERAL      49 /* CharLiteral */
#define PT_CONTINUE         50 /* continue */
#define PT_DECLITERAL       51 /* DecLiteral */
#define PT_DEFAULT          52 /* default */
#define PT_DO               53 /* do */
#define PT_ELSE             54 /* else */
#define PT_END              55 /* end */
#define PT_FLOATLITERAL     56 /* FloatLiteral */
#define PT_FOR              57 /* for */
#define PT_GOTO             58 /* goto */
#define PT_HEXLITERAL       59 /* HexLiteral */
#define PT_ID               60 /* Id */
#define PT_IF               61 /* if */
#define PT_OLDDUPHEAD       62 /* OldDupHead */
#define PT_OLDFUNCHEAD      63 /* OldFuncHead */
#define PT_OLDITEMDBHEAD    64 /* OldItemDBHead */
#define PT_OLDITEMDBHEAD_EA 65 /* 'OldItemDBHead_eA' */
#define PT_OLDMAPFLAGHEAD   66 /* OldMapFlagHead */
#define PT_OLDMINSCRIPTHEAD 67 /* OldMinScriptHead */
#define PT_OLDMOBDBHEAD     68 /* OldMobDBHead */
#define PT_OLDMOBDBHEAD_EA  69 /* 'OldMobDBHead_eA' */
#define PT_OLDMONSTERHEAD   70 /* OldMonsterHead */
#define PT_OLDSCRIPTHEAD    71 /* OldScriptHead */
#define PT_OLDSHOPHEAD      72 /* OldShopHead */
#define PT_OLDWARPHEAD      73 /* OldWarpHead */
#define PT_RETURN           74 /* return */
#define PT_STRINGLITERAL    75 /* StringLiteral */
#define PT_SWITCH           76 /* switch */
#define PT_WHILE            77 /* while */
#define PT_ARG              78 /* <Arg> */
#define PT_BLOCK            79 /* <Block> */
#define PT_CALLARG          80 /* <Call Arg> */
#define PT_CALLLIST         81 /* <Call List> */
#define PT_CALLLISTE        82 /* <Call Liste> */
#define PT_CALLSTM          83 /* <Call Stm> */
#define PT_CASESTMS         84 /* <Case Stms> */
#define PT_CONDITION        85 /* <Condition> */
#define PT_DECL             86 /* <Decl> */
#define PT_DECLS            87 /* <Decls> */
#define PT_EXPR             88 /* <Expr> */
#define PT_EXPRLIST         89 /* <Expr List> */
#define PT_FUNCTION         90 /* <function> */
#define PT_GOTOSTMS         91 /* <Goto Stms> */
#define PT_ID2              92 /* <ID> */
#define PT_LABELSTM         93 /* <Label Stm> */
#define PT_LCTRSTMS         94 /* <LCtr Stms> */
#define PT_NORMALSTM        95 /* <Normal Stm> */
#define PT_OLDDUP           96 /* <OldDup> */
#define PT_OLDFUNC          97 /* <OldFunc> */
#define PT_OLDITEMDB        98 /* <OldItemDB> */
#define PT_OLDMAPFLAG       99 /* <OldMapFlag> */
#define PT_OLDMOB           100 /* <OldMob> */
#define PT_OLDMOBDB         101 /* <OldMobDB> */
#define PT_OLDNPC           102 /* <OldNPC> */
#define PT_OLDSCRIPT        103 /* <OldScript> */
#define PT_OLDSHOP          104 /* <OldShop> */
#define PT_OLDWARP          105 /* <OldWarp> */
#define PT_OPADDSUB         106 /* <Op AddSub> */
#define PT_OPAND            107 /* <Op And> */
#define PT_OPASSIGN         108 /* <Op Assign> */
#define PT_OPBINAND         109 /* <Op BinAND> */
#define PT_OPBINOR          110 /* <Op BinOR> */
#define PT_OPBINXOR         111 /* <Op BinXOR> */
#define PT_OPCOMPARE        112 /* <Op Compare> */
#define PT_OPEQUATE         113 /* <Op Equate> */
#define PT_OPIF             114 /* <Op If> */
#define PT_OPMULTDIV        115 /* <Op MultDiv> */
#define PT_OPOR             116 /* <Op Or> */
#define PT_OPPOINTER        117 /* <Op Pointer> */
#define PT_OPSHIFT          118 /* <Op Shift> */
#define PT_OPUNARY          119 /* <Op Unary> */
#define PT_RANGE            120 /* <Range> */
#define PT_RETURNSTMS       121 /* <Return Stms> */
#define PT_STM              122 /* <Stm> */
#define PT_STMLIST          123 /* <Stm List> */
#define PT_VALUE            124 /* <Value> */
