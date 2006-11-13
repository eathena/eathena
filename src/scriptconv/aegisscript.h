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
#define AE_IDENTIFIER    58 /* identifier */
#define AE_IF            59 /* if */
#define AE_NEWLINE       60 /* NewLine */
#define AE_NPC           61 /* npc */
#define AE_NPCV          62 /* npcv */
#define AE_RETURN        63 /* return */
#define AE_STRINGLITERAL 64 /* StringLiteral */
#define AE_UNTIL         65 /* until */
#define AE_VAR           66 /* var */
#define AE_WARP          67 /* warp */
#define AE_WHILE         68 /* while */
#define AE_ARRAY         69 /* <Array> */
#define AE_BLOCK         70 /* <Block> */
#define AE_CALLARG       71 /* <Call Arg> */
#define AE_CALLLIST      72 /* <Call List> */
#define AE_CALLSTM       73 /* <Call Stm> */
#define AE_CASESTM       74 /* <Case Stm> */
#define AE_CHOOSESTM     75 /* <Choose Stm> */
#define AE_DECL          76 /* <Decl> */
#define AE_DOSTM         77 /* <Do Stm> */
#define AE_ELSESTM       78 /* <Else Stm> */
#define AE_ELSEIFSTM     79 /* <Elseif Stm> */
#define AE_EXPR          80 /* <Expr> */
#define AE_EXPRSTM       81 /* <Expr Stm> */
#define AE_FORSTM        82 /* <For Stm> */
#define AE_IFSTM         83 /* <If Stm> */
#define AE_LABEL         84 /* <Label> */
#define AE_LCTRSTM       85 /* <LCtr Stm> */
#define AE_NL            86 /* <nl> */
#define AE_NLOPT         87 /* <nl Opt> */
#define AE_NORMALSTM     88 /* <Normal Stm> */
#define AE_NPC2          89 /* <NPC> */
#define AE_NPCARRAY      90 /* <NPCArray> */
#define AE_OBJ           91 /* <Obj> */
#define AE_OPADDSUB      92 /* <Op AddSub> */
#define AE_OPAND         93 /* <Op And> */
#define AE_OPASSIGN      94 /* <Op Assign> */
#define AE_OPBINAND      95 /* <Op BinAND> */
#define AE_OPBINOR       96 /* <Op BinOR> */
#define AE_OPBINXOR      97 /* <Op BinXOR> */
#define AE_OPCOMPARE     98 /* <Op Compare> */
#define AE_OPEQUATE      99 /* <Op Equate> */
#define AE_OPIF          100 /* <Op If> */
#define AE_OPMULTDIV     101 /* <Op MultDiv> */
#define AE_OPOR          102 /* <Op Or> */
#define AE_OPSHIFT       103 /* <Op Shift> */
#define AE_OPUNARY       104 /* <Op Unary> */
#define AE_OPTVALUE      105 /* <OptValue> */
#define AE_PROGRAM       106 /* <Program> */
#define AE_RETURNSTM     107 /* <Return Stm> */
#define AE_SPRITE        108 /* <Sprite> */
#define AE_START         109 /* <Start> */
#define AE_STM           110 /* <Stm> */
#define AE_STMLIST       111 /* <Stm List> */
#define AE_VALUE         112 /* <Value> */
#define AE_VARDECL       113 /* <Var Decl> */
#define AE_WARP2         114 /* <Warp> */
#define AE_WHILESTM      115 /* <While Stm> */
