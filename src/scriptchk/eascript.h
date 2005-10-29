#define PT_EOF              0 /* (EOF) */
#define PT_ERROR            1 /* (Error) */
#define PT_WHITESPACE       2 /* (Whitespace) */
#define PT_COMMENTEND       3 /* (Comment End) */
#define PT_COMMENTLINE      4 /* (Comment Line) */
#define PT_COMMENTSTART     5 /* (Comment Start) */
#define PT_MINUS            6 /* '-' */
#define PT_MINUSMINUS       7 /* '--' */
#define PT_EXCLAM           8 /* '!' */
#define PT_EXCLAMEQ         9 /* '!=' */
#define PT_PERCENT          10 /* '%' */
#define PT_AMP              11 /* '&' */
#define PT_AMPAMP           12 /* '&&' */
#define PT_AMPEQ            13 /* '&=' */
#define PT_LPARAN           14 /* '(' */
#define PT_RPARAN           15 /* ')' */
#define PT_TIMES            16 /* '*' */
#define PT_TIMESEQ          17 /* '*=' */
#define PT_COMMA            18 /* ',' */
#define PT_DOT              19 /* '.' */
#define PT_DIV              20 /* '/' */
#define PT_DIVEQ            21 /* '/=' */
#define PT_COLON            22 /* ':' */
#define PT_COLONCOLON       23 /* '::' */
#define PT_SEMI             24 /* ';' */
#define PT_QUESTION         25 /* '?' */
#define PT_LBRACKET         26 /* '[' */
#define PT_RBRACKET         27 /* ']' */
#define PT_CARET            28 /* '^' */
#define PT_CARETEQ          29 /* '^=' */
#define PT_LBRACE           30 /* '{' */
#define PT_PIPE             31 /* '|' */
#define PT_PIPEPIPE         32 /* '||' */
#define PT_PIPEEQ           33 /* '|=' */
#define PT_RBRACE           34 /* '}' */
#define PT_TILDE            35 /* '~' */
#define PT_PLUS             36 /* '+' */
#define PT_PLUSPLUS         37 /* '++' */
#define PT_PLUSEQ           38 /* '+=' */
#define PT_LT               39 /* '<' */
#define PT_LTLT             40 /* '<<' */
#define PT_LTLTEQ           41 /* '<<=' */
#define PT_LTEQ             42 /* '<=' */
#define PT_EQ               43 /* '=' */
#define PT_MINUSEQ          44 /* '-=' */
#define PT_EQEQ             45 /* '==' */
#define PT_GT               46 /* '>' */
#define PT_GTEQ             47 /* '>=' */
#define PT_GTGT             48 /* '>>' */
#define PT_GTGTEQ           49 /* '>>=' */
#define PT_ACCOUNT          50 /* account */
#define PT_AUTO             51 /* auto */
#define PT_BREAK            52 /* break */
#define PT_CASE             53 /* case */
#define PT_CHAR             54 /* char */
#define PT_CHARLITERAL      55 /* CharLiteral */
#define PT_CONST            56 /* const */
#define PT_CONTINUE         57 /* continue */
#define PT_DECLITERAL       58 /* DecLiteral */
#define PT_DEFAULT          59 /* default */
#define PT_DO               60 /* do */
#define PT_DOUBLE           61 /* double */
#define PT_ELSE             62 /* else */
#define PT_END              63 /* end */
#define PT_EXTERN           64 /* extern */
#define PT_FLOATLITERAL     65 /* FloatLiteral */
#define PT_FOR              66 /* for */
#define PT_GLOBAL           67 /* global */
#define PT_GOTO             68 /* goto */
#define PT_HEXLITERAL       69 /* HexLiteral */
#define PT_ID               70 /* Id */
#define PT_IF               71 /* if */
#define PT_INT              72 /* int */
#define PT_OLDDUPHEAD       73 /* OldDupHead */
#define PT_OLDFUNCHEAD      74 /* OldFuncHead */
#define PT_OLDMAPFLAGHEAD   75 /* OldMapFlagHead */
#define PT_OLDMINSCRIPTHEAD 76 /* OldMinScriptHead */
#define PT_OLDMONSTERHEAD   77 /* OldMonsterHead */
#define PT_OLDSCRIPTHEAD    78 /* OldScriptHead */
#define PT_OLDSHOPHEAD      79 /* OldShopHead */
#define PT_OLDWARPHEAD      80 /* OldWarpHead */
#define PT_RETURN           81 /* return */
#define PT_SIZEOF           82 /* sizeof */
#define PT_STRING           83 /* string */
#define PT_STRINGLITERAL    84 /* StringLiteral */
#define PT_SWITCH           85 /* switch */
#define PT_TEMP             86 /* temp */
#define PT_WHILE            87 /* while */
#define PT_ARG              88 /* <Arg> */
#define PT_ARRAY            89 /* <Array> */
#define PT_BLOCK            90 /* <Block> */
#define PT_CALLARG          91 /* <Call Arg> */
#define PT_CALLLIST         92 /* <Call List> */
#define PT_CALLLISTE        93 /* <Call Liste> */
#define PT_CALLSTM          94 /* <Call Stm> */
#define PT_CARG             95 /* <CArg> */
#define PT_CASESTMS         96 /* <Case Stms> */
#define PT_CONSTE           97 /* <conste> */
#define PT_DECL             98 /* <Decl> */
#define PT_DECLS            99 /* <Decls> */
#define PT_DIR              100 /* <Dir> */
#define PT_EVENT            101 /* <Event> */
#define PT_EXPR             102 /* <Expr> */
#define PT_EXPRLIST         103 /* <Expr List> */
#define PT_FUNC             104 /* <Func> */
#define PT_GOTOSTMS         105 /* <Goto Stms> */
#define PT_ID2              106 /* <ID> */
#define PT_INITLIST         107 /* <InitList> */
#define PT_LABELSTM         108 /* <Label Stm> */
#define PT_LCTRSTMS         109 /* <LCtr Stms> */
#define PT_MAPPOS           110 /* <MapPos> */
#define PT_MOB              111 /* <Mob> */
#define PT_MOD              112 /* <Mod> */
#define PT_MULTILIST        113 /* <MultiList> */
#define PT_NAMEID           114 /* <NameId> */
#define PT_NORMALSTM        115 /* <Normal Stm> */
#define PT_NPC              116 /* <NPC> */
#define PT_OLDDUP           117 /* <OldDup> */
#define PT_OLDFUNC          118 /* <OldFunc> */
#define PT_OLDMAPFLAG       119 /* <OldMapFlag> */
#define PT_OLDMOB           120 /* <OldMob> */
#define PT_OLDNPC           121 /* <OldNPC> */
#define PT_OLDSCRIPT        122 /* <OldScript> */
#define PT_OLDSHOP          123 /* <OldShop> */
#define PT_OLDWARP          124 /* <OldWarp> */
#define PT_OPADDSUB         125 /* <Op AddSub> */
#define PT_OPAND            126 /* <Op And> */
#define PT_OPASSIGN         127 /* <Op Assign> */
#define PT_OPBINAND         128 /* <Op BinAND> */
#define PT_OPBINOR          129 /* <Op BinOR> */
#define PT_OPBINXOR         130 /* <Op BinXOR> */
#define PT_OPCAST           131 /* <Op Cast> */
#define PT_OPCOMPARE        132 /* <Op Compare> */
#define PT_OPEQUATE         133 /* <Op Equate> */
#define PT_OPIF             134 /* <Op If> */
#define PT_OPMULTDIV        135 /* <Op MultDiv> */
#define PT_OPOR             136 /* <Op Or> */
#define PT_OPPOINTER        137 /* <Op Pointer> */
#define PT_OPPOST           138 /* <Op Post> */
#define PT_OPPRE            139 /* <Op Pre> */
#define PT_OPSHIFT          140 /* <Op Shift> */
#define PT_OPSIZEOF         141 /* <Op SizeOf> */
#define PT_OPUNARY          142 /* <Op Unary> */
#define PT_PARAM            143 /* <Param> */
#define PT_PARAMS           144 /* <Params> */
#define PT_PARAMSE          145 /* <Paramse> */
#define PT_POS              146 /* <Pos> */
#define PT_PRICE            147 /* <Price> */
#define PT_PRICELIST        148 /* <PriceList> */
#define PT_RETURNSTMS       149 /* <Return Stms> */
#define PT_RETVALUES        150 /* <RetValues> */
#define PT_SCALAR           151 /* <Scalar> */
#define PT_SCALARE          152 /* <Scalare> */
#define PT_SCRIPT           153 /* <Script> */
#define PT_SHOP             154 /* <Shop> */
#define PT_SPRITEID         155 /* <SpriteId> */
#define PT_STM              156 /* <Stm> */
#define PT_STMLIST          157 /* <Stm List> */
#define PT_TYPE             158 /* <Type> */
#define PT_VALUE            159 /* <Value> */
#define PT_VAR              160 /* <Var> */
#define PT_VARDECL          161 /* <Var Decl> */
#define PT_VARLIST          162 /* <Var List> */
#define PT_WARP             163 /* <Warp> */
