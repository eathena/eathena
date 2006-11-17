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
#define AE_NEWLINE       61 /* NewLine */
#define AE_NPC           62 /* npc */
#define AE_NPCV          63 /* npcv */
#define AE_RETURN        64 /* return */
#define AE_STRINGLITERAL 65 /* StringLiteral */
#define AE_UNTIL         66 /* until */
#define AE_VAR           67 /* var */
#define AE_WARP          68 /* warp */
#define AE_WHILE         69 /* while */
#define AE_ARRAY         70 /* <Array> */
#define AE_BLOCK         71 /* <Block> */
#define AE_CALLARG       72 /* <Call Arg> */
#define AE_CALLLIST      73 /* <Call List> */
#define AE_CALLSTM       74 /* <Call Stm> */
#define AE_CASESTM       75 /* <Case Stm> */
#define AE_CHOOSESTM     76 /* <Choose Stm> */
#define AE_DECL          77 /* <Decl> */
#define AE_DOSTM         78 /* <Do Stm> */
#define AE_ELSESTM       79 /* <Else Stm> */
#define AE_ELSEIFSTM     80 /* <Elseif Stm> */
#define AE_EXPR          81 /* <Expr> */
#define AE_EXPRSTM       82 /* <Expr Stm> */
#define AE_FORSTM        83 /* <For Stm> */
#define AE_HEAD          84 /* <Head> */
#define AE_IFSTM         85 /* <If Stm> */
#define AE_LABEL         86 /* <Label> */
#define AE_LCTRSTM       87 /* <LCtr Stm> */
#define AE_NL            88 /* <nl> */
#define AE_NLOPT         89 /* <nl Opt> */
#define AE_NORMALSTM     90 /* <Normal Stm> */
#define AE_NPCARRAY      91 /* <NPCArray> */
#define AE_NPCOBJ        92 /* <NPCObj> */
#define AE_OPADDSUB      93 /* <Op AddSub> */
#define AE_OPAND         94 /* <Op And> */
#define AE_OPASSIGN      95 /* <Op Assign> */
#define AE_OPBINAND      96 /* <Op BinAND> */
#define AE_OPBINOR       97 /* <Op BinOR> */
#define AE_OPBINXOR      98 /* <Op BinXOR> */
#define AE_OPCOMPARE     99 /* <Op Compare> */
#define AE_OPEQUATE      100 /* <Op Equate> */
#define AE_OPIF          101 /* <Op If> */
#define AE_OPMULTDIV     102 /* <Op MultDiv> */
#define AE_OPOR          103 /* <Op Or> */
#define AE_OPSHIFT       104 /* <Op Shift> */
#define AE_OPUNARY       105 /* <Op Unary> */
#define AE_OPTVALUE      106 /* <OptValue> */
#define AE_PROGRAM       107 /* <Program> */
#define AE_RANDINIT      108 /* <Rand Init> */
#define AE_RETURNSTM     109 /* <Return Stm> */
#define AE_START         110 /* <Start> */
#define AE_STM           111 /* <Stm> */
#define AE_STMLIST       112 /* <Stm List> */
#define AE_VALUE         113 /* <Value> */
#define AE_VARDECL       114 /* <Var Decl> */
#define AE_VARINIT       115 /* <Var Init> */
#define AE_WARPNPCOBJ    116 /* <WarpNPCObj> */
#define AE_WARPOBJ       117 /* <WarpObj> */
#define AE_WHILESTM      118 /* <While Stm> */
