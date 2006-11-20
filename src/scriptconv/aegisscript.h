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
#define AE_DECLITERAL    48 /* DecLiteral */
#define AE_DEFAULT       49 /* default */
#define AE_DO            50 /* do */
#define AE_ELSE          51 /* else */
#define AE_ELSEIF        52 /* elseif */
#define AE_ENDCHOOSE     53 /* endchoose */
#define AE_ENDFOR        54 /* endfor */
#define AE_ENDIF         55 /* endif */
#define AE_ENDWHILE      56 /* endwhile */
#define AE_FOR           57 /* for */
#define AE_HIDDENWARP    58 /* hiddenwarp */
#define AE_IDENTIFIER    59 /* identifier */
#define AE_IF            60 /* if */
#define AE_LOT           61 /* lot */
#define AE_NEWLINE       62 /* NewLine */
#define AE_NPC           63 /* npc */
#define AE_NPCV          64 /* npcv */
#define AE_PUTMOB        65 /* putmob */
#define AE_RETURN        66 /* return */
#define AE_STRINGLITERAL 67 /* StringLiteral */
#define AE_TRADER        68 /* trader */
#define AE_UNTIL         69 /* until */
#define AE_VAR           70 /* var */
#define AE_WARP          71 /* warp */
#define AE_WHILE         72 /* while */
#define AE_ARRAY         73 /* <Array> */
#define AE_BLOCK         74 /* <Block> */
#define AE_CALLARG       75 /* <Call Arg> */
#define AE_CALLLIST      76 /* <Call List> */
#define AE_CALLSTM       77 /* <Call Stm> */
#define AE_CASESTM       78 /* <Case Stm> */
#define AE_CHOOSESTM     79 /* <Choose Stm> */
#define AE_DECL          80 /* <Decl> */
#define AE_DOSTM         81 /* <Do Stm> */
#define AE_ELSESTM       82 /* <Else Stm> */
#define AE_ELSEIFSTM     83 /* <Elseif Stm> */
#define AE_EXPR          84 /* <Expr> */
#define AE_EXPRSTM       85 /* <Expr Stm> */
#define AE_FORSTM        86 /* <For Stm> */
#define AE_HEAD          87 /* <Head> */
#define AE_IFSTM         88 /* <If Stm> */
#define AE_LABEL         89 /* <Label> */
#define AE_LCTRSTM       90 /* <LCtr Stm> */
#define AE_LOTARRAY      91 /* <LOTArray> */
#define AE_MOBOBJ        92 /* <MobObj> */
#define AE_NL            93 /* <nl> */
#define AE_NLOPT         94 /* <nl Opt> */
#define AE_NORMALSTM     95 /* <Normal Stm> */
#define AE_NPCARRAY      96 /* <NPCArray> */
#define AE_NPCOBJ        97 /* <NPCObj> */
#define AE_OPADDSUB      98 /* <Op AddSub> */
#define AE_OPAND         99 /* <Op And> */
#define AE_OPASSIGN      100 /* <Op Assign> */
#define AE_OPBINAND      101 /* <Op BinAND> */
#define AE_OPBINOR       102 /* <Op BinOR> */
#define AE_OPBINXOR      103 /* <Op BinXOR> */
#define AE_OPCOMPARE     104 /* <Op Compare> */
#define AE_OPEQUATE      105 /* <Op Equate> */
#define AE_OPIF          106 /* <Op If> */
#define AE_OPMULTDIV     107 /* <Op MultDiv> */
#define AE_OPOR          108 /* <Op Or> */
#define AE_OPSHIFT       109 /* <Op Shift> */
#define AE_OPUNARY       110 /* <Op Unary> */
#define AE_OPTVALUE      111 /* <OptValue> */
#define AE_PROGRAM       112 /* <Program> */
#define AE_RANDINIT      113 /* <Rand Init> */
#define AE_RETURNSTM     114 /* <Return Stm> */
#define AE_START         115 /* <Start> */
#define AE_STM           116 /* <Stm> */
#define AE_STMLIST       117 /* <Stm List> */
#define AE_VALUE         118 /* <Value> */
#define AE_VARDECL       119 /* <Var Decl> */
#define AE_VARINIT       120 /* <Var Init> */
#define AE_WARPNPCOBJ    121 /* <WarpNPCObj> */
#define AE_WARPOBJ       122 /* <WarpObj> */
#define AE_WHILESTM      123 /* <While Stm> */
