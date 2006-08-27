// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#define PT_EOF              0 /* (EOF) */
#define PT_ERROR            1 /* (Error) */
#define PT_WHITESPACE       2 /* (Whitespace) */
#define PT_COMMENTEND       3 /* (Comment End) */
#define PT_COMMENTLINE      4 /* (Comment Line) */
#define PT_COMMENTSTART     5 /* (Comment Start) */
#define PT_APOST            6 /* '' */
#define PT_MINUS            7 /* '-' */
#define PT_MINUSMINUS       8 /* '--' */
#define PT_EXCLAM           9 /* '!' */
#define PT_EXCLAMEQ         10 /* '!=' */
#define PT_PERCENT          11 /* '%' */
#define PT_AMP              12 /* '&' */
#define PT_AMPAMP           13 /* '&&' */
#define PT_AMPEQ            14 /* '&=' */
#define PT_LPARAN           15 /* '(' */
#define PT_RPARAN           16 /* ')' */
#define PT_TIMES            17 /* '*' */
#define PT_TIMESEQ          18 /* '*=' */
#define PT_COMMA            19 /* ',' */
#define PT_DOT              20 /* '.' */
#define PT_DIV              21 /* '/' */
#define PT_DIVEQ            22 /* '/=' */
#define PT_COLON            23 /* ':' */
#define PT_COLONCOLON       24 /* '::' */
#define PT_SEMI             25 /* ';' */
#define PT_QUESTION         26 /* '?' */
#define PT_LBRACKET         27 /* '[' */
#define PT_RBRACKET         28 /* ']' */
#define PT_CARET            29 /* '^' */
#define PT_CARETEQ          30 /* '^=' */
#define PT_LBRACE           31 /* '{' */
#define PT_PIPE             32 /* '|' */
#define PT_PIPEPIPE         33 /* '||' */
#define PT_PIPEEQ           34 /* '|=' */
#define PT_RBRACE           35 /* '}' */
#define PT_TILDE            36 /* '~' */
#define PT_PLUS             37 /* '+' */
#define PT_PLUSPLUS         38 /* '++' */
#define PT_PLUSEQ           39 /* '+=' */
#define PT_LT               40 /* '<' */
#define PT_LTLT             41 /* '<<' */
#define PT_LTLTEQ           42 /* '<<=' */
#define PT_LTEQ             43 /* '<=' */
#define PT_EQ               44 /* '=' */
#define PT_MINUSEQ          45 /* '-=' */
#define PT_EQEQ             46 /* '==' */
#define PT_GT               47 /* '>' */
#define PT_GTEQ             48 /* '>=' */
#define PT_GTGT             49 /* '>>' */
#define PT_GTGTEQ           50 /* '>>=' */
#define PT_ACCOUNT          51 /* account */
#define PT_AUTO             52 /* auto */
#define PT_BREAK            53 /* break */
#define PT_CASE             54 /* case */
#define PT_CHAR             55 /* char */
#define PT_CHARLITERAL      56 /* CharLiteral */
#define PT_CONST            57 /* const */
#define PT_CONTINUE         58 /* continue */
#define PT_DECLITERAL       59 /* DecLiteral */
#define PT_DEFAULT          60 /* default */
#define PT_DO               61 /* do */
#define PT_DOUBLE           62 /* double */
#define PT_ELSE             63 /* else */
#define PT_END              64 /* end */
#define PT_EXTERN           65 /* extern */
#define PT_FLOATLITERAL     66 /* FloatLiteral */
#define PT_FOR              67 /* for */
#define PT_GLOBAL           68 /* global */
#define PT_GOTO             69 /* goto */
#define PT_HEXLITERAL       70 /* HexLiteral */
#define PT_ID               71 /* Id */
#define PT_IF               72 /* if */
#define PT_INT              73 /* int */
#define PT_OLDDUPHEAD       74 /* OldDupHead */
#define PT_OLDFUNCHEAD      75 /* OldFuncHead */
#define PT_OLDMAPFLAGHEAD   76 /* OldMapFlagHead */
#define PT_OLDMINSCRIPTHEAD 77 /* OldMinScriptHead */
#define PT_OLDMONSTERHEAD   78 /* OldMonsterHead */
#define PT_OLDSCRIPTHEAD    79 /* OldScriptHead */
#define PT_OLDSHOPHEAD      80 /* OldShopHead */
#define PT_OLDWARPHEAD      81 /* OldWarpHead */
#define PT_RETURN           82 /* return */
#define PT_SIZEOF           83 /* sizeof */
#define PT_STRING           84 /* string */
#define PT_STRINGLITERAL    85 /* StringLiteral */
#define PT_SWITCH           86 /* switch */
#define PT_TEMP             87 /* temp */
#define PT_WHILE            88 /* while */
#define PT_ARG              89 /* <Arg> */
#define PT_ARRAY            90 /* <Array> */
#define PT_BLOCK            91 /* <Block> */
#define PT_CALLARG          92 /* <Call Arg> */
#define PT_CALLLIST         93 /* <Call List> */
#define PT_CALLLISTE        94 /* <Call Liste> */
#define PT_CALLSTM          95 /* <Call Stm> */
#define PT_CASESTMS         96 /* <Case Stms> */
#define PT_CONDITION        97 /* <Condition> */
#define PT_CONSTE           98 /* <conste> */
#define PT_DECL             99 /* <Decl> */
#define PT_DECLS            100 /* <Decls> */
#define PT_DIR              101 /* <Dir> */
#define PT_EVENT            102 /* <Event> */
#define PT_EXPR             103 /* <Expr> */
#define PT_EXPRLIST         104 /* <Expr List> */
#define PT_FUNC             105 /* <Func> */
#define PT_GOTOSTMS         106 /* <Goto Stms> */
#define PT_ID2              107 /* <ID> */
#define PT_IDELEM           108 /* <IDElem> */
#define PT_IDLIMITER        109 /* <IDLimiter> */
#define PT_IDS              110 /* <IDs> */
#define PT_INITLIST         111 /* <InitList> */
#define PT_ITEM             112 /* <Item> */
#define PT_LABELSTM         113 /* <Label Stm> */
#define PT_LCTRSTMS         114 /* <LCtr Stms> */
#define PT_MAPPOS           115 /* <MapPos> */
#define PT_MOB              116 /* <Mob> */
#define PT_MOD              117 /* <Mod> */
#define PT_MULTILIST        118 /* <MultiList> */
#define PT_NAMEID           119 /* <NameId> */
#define PT_NE               120 /* <NE> */
#define PT_NORMALSTM        121 /* <Normal Stm> */
#define PT_NPC              122 /* <NPC> */
#define PT_OLDDUP           123 /* <OldDup> */
#define PT_OLDFUNC          124 /* <OldFunc> */
#define PT_OLDITEM          125 /* <OldItem> */
#define PT_OLDMAPFLAG       126 /* <OldMapFlag> */
#define PT_OLDMOB           127 /* <OldMob> */
#define PT_OLDNPC           128 /* <OldNPC> */
#define PT_OLDSCRIPT        129 /* <OldScript> */
#define PT_OLDSHOP          130 /* <OldShop> */
#define PT_OLDWARP          131 /* <OldWarp> */
#define PT_OPADDSUB         132 /* <Op AddSub> */
#define PT_OPAND            133 /* <Op And> */
#define PT_OPASSIGN         134 /* <Op Assign> */
#define PT_OPBINAND         135 /* <Op BinAND> */
#define PT_OPBINOR          136 /* <Op BinOR> */
#define PT_OPBINXOR         137 /* <Op BinXOR> */
#define PT_OPCAST           138 /* <Op Cast> */
#define PT_OPCOMPARE        139 /* <Op Compare> */
#define PT_OPEQUATE         140 /* <Op Equate> */
#define PT_OPIF             141 /* <Op If> */
#define PT_OPMULTDIV        142 /* <Op MultDiv> */
#define PT_OPOR             143 /* <Op Or> */
#define PT_OPPOINTER        144 /* <Op Pointer> */
#define PT_OPPOST           145 /* <Op Post> */
#define PT_OPPRE            146 /* <Op Pre> */
#define PT_OPSHIFT          147 /* <Op Shift> */
#define PT_OPSIZEOF         148 /* <Op SizeOf> */
#define PT_OPUNARY          149 /* <Op Unary> */
#define PT_PARAM            150 /* <Param> */
#define PT_PARAMS           151 /* <Params> */
#define PT_PARAMSE          152 /* <Paramse> */
#define PT_POS              153 /* <Pos> */
#define PT_PRICE            154 /* <Price> */
#define PT_PRICELIST        155 /* <PriceList> */
#define PT_RETURNSTMS       156 /* <Return Stms> */
#define PT_RETVALUES        157 /* <RetValues> */
#define PT_SCALAR           158 /* <Scalar> */
#define PT_SCALARE          159 /* <Scalare> */
#define PT_SCRIPT           160 /* <Script> */
#define PT_SHOP             161 /* <Shop> */
#define PT_SPRITEID         162 /* <SpriteId> */
#define PT_STM              163 /* <Stm> */
#define PT_STMLIST          164 /* <Stm List> */
#define PT_TYPE             165 /* <Type> */
#define PT_VALUE            166 /* <Value> */
#define PT_VAR              167 /* <Var> */
#define PT_VARDECL          168 /* <Var Decl> */
#define PT_VARLIST          169 /* <Var List> */
#define PT_WARP             170 /* <Warp> */
