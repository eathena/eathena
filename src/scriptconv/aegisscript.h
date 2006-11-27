#define AE_EOF           0 /* (EOF) */
#define AE_ERROR         1 /* (Error) */
#define AE_WHITESPACE    2 /* (Whitespace) */
#define AE_COMMENTEND    3 /* (Comment End) */
#define AE_COMMENTLINE   4 /* (Comment Line) */
#define AE_COMMENTSTART  5 /* (Comment Start) */
#define AE_MINUS         6 /* '-' */
#define AE_EXCLAM        7 /* '!' */
#define AE_EXCLAMEQ      8 /* '!=' */
#define AE_PERCENT       9 /* '%' */
#define AE_AMP           10 /* '&' */
#define AE_AMPAMP        11 /* '&&' */
#define AE_AMPEQ         12 /* '&=' */
#define AE_LPARAN        13 /* '(' */
#define AE_RPARAN        14 /* ')' */
#define AE_TIMES         15 /* '*' */
#define AE_TIMESEQ       16 /* '*=' */
#define AE_DOTDOT        17 /* '..' */
#define AE_DIV           18 /* '/' */
#define AE_DIVEQ         19 /* '/=' */
#define AE_COLON         20 /* ':' */
#define AE_QUESTION      21 /* '?' */
#define AE_LBRACKET      22 /* '[' */
#define AE_RBRACKET      23 /* ']' */
#define AE_CARET         24 /* '^' */
#define AE_CARETEQ       25 /* '^=' */
#define AE_PIPE          26 /* '|' */
#define AE_PIPEPIPE      27 /* '||' */
#define AE_PIPEEQ        28 /* '|=' */
#define AE_TILDE         29 /* '~' */
#define AE_PLUS          30 /* '+' */
#define AE_PLUSEQ        31 /* '+=' */
#define AE_LT            32 /* '<' */
#define AE_LTLT          33 /* '<<' */
#define AE_LTLTEQ        34 /* '<<=' */
#define AE_LTEQ          35 /* '<=' */
#define AE_EQ            36 /* '=' */
#define AE_MINUSEQ       37 /* '-=' */
#define AE_EQEQ          38 /* '==' */
#define AE_GT            39 /* '>' */
#define AE_GTEQ          40 /* '>=' */
#define AE_GTGT          41 /* '>>' */
#define AE_GTGTEQ        42 /* '>>=' */
#define AE_ARENAGUIDE    43 /* arenaguide */
#define AE_BREAK         44 /* break */
#define AE_CASE          45 /* case */
#define AE_CHOOSE        46 /* choose */
#define AE_CONTINUE      47 /* continue */
#define AE_DECLARE       48 /* declare */
#define AE_DECLITERAL    49 /* DecLiteral */
#define AE_DEFAULT       50 /* default */
#define AE_DEFINE        51 /* define */
#define AE_DO            52 /* do */
#define AE_EFFECT        53 /* effect */
#define AE_ELSE          54 /* else */
#define AE_ELSEIF        55 /* elseif */
#define AE_ENDCHOOSE     56 /* endchoose */
#define AE_ENDFOR        57 /* endfor */
#define AE_ENDIF         58 /* endif */
#define AE_ENDWHILE      59 /* endwhile */
#define AE_FOR           60 /* for */
#define AE_GUIDE         61 /* guide */
#define AE_HIDDENWARP    62 /* hiddenwarp */
#define AE_IDENTIFIER    63 /* identifier */
#define AE_IF            64 /* if */
#define AE_LOT           65 /* lot */
#define AE_MOB           66 /* mob */
#define AE_NEWLINE       67 /* NewLine */
#define AE_NPC           68 /* npc */
#define AE_NPCV          69 /* npcv */
#define AE_PUTMOB        70 /* putmob */
#define AE_RETURN        71 /* return */
#define AE_STRINGLITERAL 72 /* StringLiteral */
#define AE_TRADER        73 /* trader */
#define AE_UNTIL         74 /* until */
#define AE_VAR           75 /* var */
#define AE_WARP          76 /* warp */
#define AE_WHILE         77 /* while */
#define AE_ARRAY         78 /* <Array> */
#define AE_BLOCK         79 /* <Block> */
#define AE_CALLARG       80 /* <Call Arg> */
#define AE_CALLLIST      81 /* <Call List> */
#define AE_CALLSTM       82 /* <Call Stm> */
#define AE_CASESTM       83 /* <Case Stm> */
#define AE_CHOOSESTM     84 /* <Choose Stm> */
#define AE_DECL          85 /* <Decl> */
#define AE_DECLARATION   86 /* <Declaration> */
#define AE_DEFINITION    87 /* <Definition> */
#define AE_DOSTM         88 /* <Do Stm> */
#define AE_EFFECTOBJ     89 /* <EffectObj> */
#define AE_ELSESTM       90 /* <Else Stm> */
#define AE_ELSEIFSTM     91 /* <Elseif Stm> */
#define AE_EXPR          92 /* <Expr> */
#define AE_EXPRSTM       93 /* <Expr Stm> */
#define AE_FORSTM        94 /* <For Stm> */
#define AE_HEAD          95 /* <Head> */
#define AE_IFSTM         96 /* <If Stm> */
#define AE_LABEL         97 /* <Label> */
#define AE_LCTRSTM       98 /* <LCtr Stm> */
#define AE_LOTARRAY      99 /* <LOTArray> */
#define AE_MOBDECL       100 /* <MobDecl> */
#define AE_MOBOBJ        101 /* <MobObj> */
#define AE_NL            102 /* <nl> */
#define AE_NLOPT         103 /* <nl Opt> */
#define AE_NORMALSTM     104 /* <Normal Stm> */
#define AE_NPCARRAY      105 /* <NPCArray> */
#define AE_NPCOBJ        106 /* <NPCObj> */
#define AE_OPADDSUB      107 /* <Op AddSub> */
#define AE_OPAND         108 /* <Op And> */
#define AE_OPASSIGN      109 /* <Op Assign> */
#define AE_OPBINAND      110 /* <Op BinAND> */
#define AE_OPBINOR       111 /* <Op BinOR> */
#define AE_OPBINXOR      112 /* <Op BinXOR> */
#define AE_OPCOMPARE     113 /* <Op Compare> */
#define AE_OPEQUATE      114 /* <Op Equate> */
#define AE_OPIF          115 /* <Op If> */
#define AE_OPMULTDIV     116 /* <Op MultDiv> */
#define AE_OPOR          117 /* <Op Or> */
#define AE_OPSHIFT       118 /* <Op Shift> */
#define AE_OPUNARY       119 /* <Op Unary> */
#define AE_OPTVALUE      120 /* <OptValue> */
#define AE_PROGRAM       121 /* <Program> */
#define AE_RANDINIT      122 /* <Rand Init> */
#define AE_RETURNSTM     123 /* <Return Stm> */
#define AE_START         124 /* <Start> */
#define AE_STM           125 /* <Stm> */
#define AE_STMLIST       126 /* <Stm List> */
#define AE_VALUE         127 /* <Value> */
#define AE_VARDECL       128 /* <Var Decl> */
#define AE_VARINIT       129 /* <Var Init> */
#define AE_WARPNPCOBJ    130 /* <WarpNPCObj> */
#define AE_WARPOBJ       131 /* <WarpObj> */
#define AE_WHILESTM      132 /* <While Stm> */
