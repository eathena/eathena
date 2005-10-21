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
#define PT_EXTERN           63 /* extern */
#define PT_FLOATLITERAL     64 /* FloatLiteral */
#define PT_FOR              65 /* for */
#define PT_GLOBAL           66 /* global */
#define PT_GOTO             67 /* goto */
#define PT_HEXLITERAL       68 /* HexLiteral */
#define PT_ID               69 /* Id */
#define PT_IF               70 /* if */
#define PT_INT              71 /* int */
#define PT_OLDDUPHEAD       72 /* OldDupHead */
#define PT_OLDFUNCHEAD      73 /* OldFuncHead */
#define PT_OLDMAPFLAGHEAD   74 /* OldMapFlagHead */
#define PT_OLDMINSCRIPTHEAD 75 /* OldMinScriptHead */
#define PT_OLDMONSTERHEAD   76 /* OldMonsterHead */
#define PT_OLDSCRIPTHEAD    77 /* OldScriptHead */
#define PT_OLDSHOPHEAD      78 /* OldShopHead */
#define PT_OLDWARPHEAD      79 /* OldWarpHead */
#define PT_RETURN           80 /* return */
#define PT_SIZEOF           81 /* sizeof */
#define PT_STRING           82 /* string */
#define PT_STRINGLITERAL    83 /* StringLiteral */
#define PT_SWITCH           84 /* switch */
#define PT_TEMP             85 /* temp */
#define PT_WHILE            86 /* while */
#define PT_ARG              87 /* <Arg> */
#define PT_ARRAY            88 /* <Array> */
#define PT_BLOCK            89 /* <Block> */
#define PT_CALLARG          90 /* <Call Arg> */
#define PT_CALLLIST         91 /* <Call List> */
#define PT_CALLLISTE        92 /* <Call Liste> */
#define PT_CALLSTM          93 /* <Call Stm> */
#define PT_CARG             94 /* <CArg> */
#define PT_CASESTMS         95 /* <Case Stms> */
#define PT_CONSTE           96 /* <conste> */
#define PT_DECL             97 /* <Decl> */
#define PT_DECLS            98 /* <Decls> */
#define PT_DIR              99 /* <Dir> */
#define PT_EVENT            100 /* <Event> */
#define PT_EXPR             101 /* <Expr> */
#define PT_EXPRLIST         102 /* <Expr List> */
#define PT_FUNC             103 /* <Func> */
#define PT_GOTOSTMS         104 /* <Goto Stms> */
#define PT_ID2              105 /* <ID> */
#define PT_INITLIST         106 /* <InitList> */
#define PT_LABELSTM         107 /* <Label Stm> */
#define PT_LCTRSTMS         108 /* <LCtr Stms> */
#define PT_MAPPOS           109 /* <MapPos> */
#define PT_MOB              110 /* <Mob> */
#define PT_MOD              111 /* <Mod> */
#define PT_MULTILIST        112 /* <MultiList> */
#define PT_NAMEID           113 /* <NameId> */
#define PT_NORMALSTM        114 /* <Normal Stm> */
#define PT_NPC              115 /* <NPC> */
#define PT_OLDDUP           116 /* <OldDup> */
#define PT_OLDFUNC          117 /* <OldFunc> */
#define PT_OLDMAPFLAG       118 /* <OldMapFlag> */
#define PT_OLDMOB           119 /* <OldMob> */
#define PT_OLDNPC           120 /* <OldNPC> */
#define PT_OLDSCRIPT        121 /* <OldScript> */
#define PT_OLDSHOP          122 /* <OldShop> */
#define PT_OLDWARP          123 /* <OldWarp> */
#define PT_OPADDSUB         124 /* <Op AddSub> */
#define PT_OPAND            125 /* <Op And> */
#define PT_OPASSIGN         126 /* <Op Assign> */
#define PT_OPBINAND         127 /* <Op BinAND> */
#define PT_OPBINOR          128 /* <Op BinOR> */
#define PT_OPBINXOR         129 /* <Op BinXOR> */
#define PT_OPCAST           130 /* <Op Cast> */
#define PT_OPCOMPARE        131 /* <Op Compare> */
#define PT_OPEQUATE         132 /* <Op Equate> */
#define PT_OPIF             133 /* <Op If> */
#define PT_OPMULTDIV        134 /* <Op MultDiv> */
#define PT_OPOR             135 /* <Op Or> */
#define PT_OPPOINTER        136 /* <Op Pointer> */
#define PT_OPPOST           137 /* <Op Post> */
#define PT_OPPRE            138 /* <Op Pre> */
#define PT_OPSHIFT          139 /* <Op Shift> */
#define PT_OPSIZEOF         140 /* <Op SizeOf> */
#define PT_OPUNARY          141 /* <Op Unary> */
#define PT_PARAM            142 /* <Param> */
#define PT_PARAMS           143 /* <Params> */
#define PT_PARAMSE          144 /* <Paramse> */
#define PT_POS              145 /* <Pos> */
#define PT_PRICE            146 /* <Price> */
#define PT_PRICELIST        147 /* <PriceList> */
#define PT_RETURNSTMS       148 /* <Return Stms> */
#define PT_RETVALUES        149 /* <RetValues> */
#define PT_SCALAR           150 /* <Scalar> */
#define PT_SCALARE          151 /* <Scalare> */
#define PT_SCRIPT           152 /* <Script> */
#define PT_SHOP             153 /* <Shop> */
#define PT_SPRITEID         154 /* <SpriteId> */
#define PT_STM              155 /* <Stm> */
#define PT_STMLIST          156 /* <Stm List> */
#define PT_TYPE             157 /* <Type> */
#define PT_VALUE            158 /* <Value> */
#define PT_VAR              159 /* <Var> */
#define PT_VARDECL          160 /* <Var Decl> */
#define PT_VARLIST          161 /* <Var List> */
#define PT_WARP             162 /* <Warp> */
