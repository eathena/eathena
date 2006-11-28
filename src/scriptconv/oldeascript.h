#define EA_EOF              0 /* (EOF) */
#define EA_ERROR            1 /* (Error) */
#define EA_WHITESPACE       2 /* (Whitespace) */
#define EA_COMMENTEND       3 /* (Comment End) */
#define EA_COMMENTLINE      4 /* (Comment Line) */
#define EA_COMMENTSTART     5 /* (Comment Start) */
#define EA_MINUS            6 /* '-' */
#define EA_EXCLAM           7 /* '!' */
#define EA_EXCLAMEQ         8 /* '!=' */
#define EA_PERCENT          9 /* '%' */
#define EA_AMP              10 /* '&' */
#define EA_AMPAMP           11 /* '&&' */
#define EA_LPARAN           12 /* '(' */
#define EA_RPARAN           13 /* ')' */
#define EA_TIMES            14 /* '*' */
#define EA_COMMA            15 /* ',' */
#define EA_DIV              16 /* '/' */
#define EA_COLON            17 /* ':' */
#define EA_SEMI             18 /* ';' */
#define EA_LBRACKET         19 /* '[' */
#define EA_RBRACKET         20 /* ']' */
#define EA_CARET            21 /* '^' */
#define EA_LBRACE           22 /* '{' */
#define EA_PIPE             23 /* '|' */
#define EA_PIPEPIPE         24 /* '||' */
#define EA_RBRACE           25 /* '}' */
#define EA_TILDE            26 /* '~' */
#define EA_PLUS             27 /* '+' */
#define EA_LT               28 /* '<' */
#define EA_LTLT             29 /* '<<' */
#define EA_LTEQ             30 /* '<=' */
#define EA_EQEQ             31 /* '==' */
#define EA_GT               32 /* '>' */
#define EA_GTEQ             33 /* '>=' */
#define EA_GTGT             34 /* '>>' */
#define EA_BREAK            35 /* break */
#define EA_CASE             36 /* case */
#define EA_CONTINUE         37 /* continue */
#define EA_DECLITERAL       38 /* DecLiteral */
#define EA_DEFAULT          39 /* default */
#define EA_DO               40 /* do */
#define EA_ELSE             41 /* else */
#define EA_END              42 /* end */
#define EA_FOR              43 /* for */
#define EA_FUNCTION         44 /* function */
#define EA_GOTO             45 /* goto */
#define EA_HEXLITERAL       46 /* HexLiteral */
#define EA_IDENTIFIER       47 /* identifier */
#define EA_IF               48 /* if */
#define EA_OLDDUPHEAD       49 /* OldDupHead */
#define EA_OLDFUNCHEAD      50 /* OldFuncHead */
#define EA_OLDITEMDBHEAD    51 /* OldItemDBHead */
#define EA_OLDITEMDBHEAD_EA 52 /* 'OldItemDBHead_eA' */
#define EA_OLDMAPFLAGHEAD   53 /* OldMapFlagHead */
#define EA_OLDMINSCRIPTHEAD 54 /* OldMinScriptHead */
#define EA_OLDMOBDBHEAD     55 /* OldMobDBHead */
#define EA_OLDMOBDBHEAD_EA  56 /* 'OldMobDBHead_eA' */
#define EA_OLDMONSTERHEAD   57 /* OldMonsterHead */
#define EA_OLDSCRIPTHEAD    58 /* OldScriptHead */
#define EA_OLDSHOPHEAD      59 /* OldShopHead */
#define EA_OLDWARPHEAD      60 /* OldWarpHead */
#define EA_RETURN           61 /* return */
#define EA_STRINGLITERAL    62 /* StringLiteral */
#define EA_SWITCH           63 /* switch */
#define EA_WHILE            64 /* while */
#define EA_ARG              65 /* <Arg> */
#define EA_BLOCK            66 /* <Block> */
#define EA_CALLARG          67 /* <Call Arg> */
#define EA_CALLLIST         68 /* <Call List> */
#define EA_CALLLISTE        69 /* <Call Liste> */
#define EA_CALLSTM          70 /* <Call Stm> */
#define EA_CASESTM          71 /* <Case Stm> */
#define EA_CONDITION        72 /* <Condition> */
#define EA_DECL             73 /* <Decl> */
#define EA_DECLS            74 /* <Decls> */
#define EA_DOSTM            75 /* <Do Stm> */
#define EA_EVALUATION       76 /* <Evaluation> */
#define EA_EXPR             77 /* <Expr> */
#define EA_EXPRLIST         78 /* <Expr List> */
#define EA_EXPRSTM          79 /* <Expr Stm> */
#define EA_FORSTM           80 /* <For Stm> */
#define EA_FUNCCALL         81 /* <FuncCall> */
#define EA_GOTOSTM          82 /* <Goto Stm> */
#define EA_IFSTM            83 /* <If Stm> */
#define EA_LABELSTM         84 /* <Label Stm> */
#define EA_LCTRSTM          85 /* <LCtr Stm> */
#define EA_NORMALSTM        86 /* <Normal Stm> */
#define EA_OLDDUP           87 /* <OldDup> */
#define EA_OLDFUNC          88 /* <OldFunc> */
#define EA_OLDITEMDB        89 /* <OldItemDB> */
#define EA_OLDMAPFLAG       90 /* <OldMapFlag> */
#define EA_OLDMOB           91 /* <OldMob> */
#define EA_OLDMOBDB         92 /* <OldMobDB> */
#define EA_OLDNPC           93 /* <OldNPC> */
#define EA_OLDSCRIPT        94 /* <OldScript> */
#define EA_OLDSHOP          95 /* <OldShop> */
#define EA_OLDWARP          96 /* <OldWarp> */
#define EA_OPADDSUB         97 /* <Op AddSub> */
#define EA_OPAND            98 /* <Op And> */
#define EA_OPASSIGN         99 /* <Op Assign> */
#define EA_OPBINAND         100 /* <Op BinAND> */
#define EA_OPBINOR          101 /* <Op BinOR> */
#define EA_OPBINXOR         102 /* <Op BinXOR> */
#define EA_OPCOMPARE        103 /* <Op Compare> */
#define EA_OPEQUATE         104 /* <Op Equate> */
#define EA_OPIF             105 /* <Op If> */
#define EA_OPMULTDIV        106 /* <Op MultDiv> */
#define EA_OPOR             107 /* <Op Or> */
#define EA_OPPOINTER        108 /* <Op Pointer> */
#define EA_OPSHIFT          109 /* <Op Shift> */
#define EA_OPUNARY          110 /* <Op Unary> */
#define EA_RANGE            111 /* <Range> */
#define EA_RETURNSTM        112 /* <Return Stm> */
#define EA_STM              113 /* <Stm> */
#define EA_STMLIST          114 /* <Stm List> */
#define EA_SUBFUNCTION      115 /* <SubFunction> */
#define EA_SWITCHSTM        116 /* <Switch Stm> */
#define EA_VALUE            117 /* <Value> */
#define EA_WHILESTM         118 /* <While Stm> */
