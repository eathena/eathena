#include "eaparser.h"



bool eaprinter::print_beautified(basics::CParser_CommentStore& parser, int rtpos, short parent)
{
	eaprinter &prn = *this;
	bool ret = true;

	print_comments(parser, rtpos);

	if( parser.rt[rtpos].symbol.Type == 1 )
	{	// terminals
		switch( parser.rt[rtpos].symbol.idx )
		{
		case PT_RBRACE:
			scope--;
			if(!prn.newline) prn << '\n';
			prn << "}\n";
			break;
		case PT_LBRACE:
			if(!prn.newline) prn << '\n';
			prn << "{\n";
			++prn.scope;
			break;
		case PT_SEMI:
			prn << ";\n";
			break;
		case PT_COMMA:
			prn << ", ";
			break;
		//case PT_LPARAN:
		//case PT_RPARAN:
		default:
			// print the token
			prn << parser.rt[rtpos].cToken.cLexeme;
			break;
		}
	}
	else if( parser.rt[rtpos].cChildNum==1 )
	{	// only one child, just go down
		print_beautified(parser, parser.rt[rtpos].cChildPos);
	}
	else if( parser.rt[rtpos].cChildNum>1 )
	{	// nonterminals
		switch( parser.rt[rtpos].symbol.idx )
		{
		case PT_LABELSTM:
		{
			int tmpscope = prn.scope;
			prn.scope=0;
			print_beautified(parser, parser.rt[rtpos].cChildPos);
			prn << ":\n";
			prn.scope = tmpscope;
			break;
		}
		case PT_GOTOSTM:
		{
			prn << "goto ";
			prn << parser.rt[parser.rt[rtpos].cChildPos+1].cToken.cLexeme;
			prn << ';' << '\n';
			break;
		}
		case PT_IFSTM:
		case PT_WHILESTM:
		{
			// if '(' <Expr> ')' <Normal Stm>
			// if '(' <Expr> ')' <Normal Stm> else <Normal Stm>
			// while '(' <Expr> ')' <Normal Stm>
			if(!prn.newline) prn << '\n';
			print_beautified(parser, parser.rt[rtpos].cChildPos+0);
			prn << "( ";
			print_beautified(parser, parser.rt[rtpos].cChildPos+2);
			prn << " )\n";

			if( PT_BLOCK != parser.rt[ parser.rt[rtpos].cChildPos+4 ].symbol.idx )
				++prn.scope;
			print_beautified(parser, parser.rt[rtpos].cChildPos+4);
			if( PT_BLOCK != parser.rt[ parser.rt[rtpos].cChildPos+4 ].symbol.idx )
				--prn.scope;

			if( parser.rt[rtpos].cChildNum==7 )
			{
				print_beautified(parser, parser.rt[rtpos].cChildPos+5);
				if(!prn.newline) prn << '\n';

				if( PT_BLOCK != parser.rt[ parser.rt[rtpos].cChildPos+6 ].symbol.idx )
					++prn.scope;
				print_beautified(parser, parser.rt[rtpos].cChildPos+6);
				if( PT_BLOCK != parser.rt[ parser.rt[rtpos].cChildPos+6 ].symbol.idx )
					--prn.scope;
			}
			break;
		}
		case PT_DOSTM:
		{	// do <Normal Stm> while '(' <Expr> ')' ';'
			if(!prn.newline) prn << '\n';
			print_beautified(parser, parser.rt[rtpos].cChildPos+0);
			prn << '\n';
			if( PT_BLOCK != parser.rt[ parser.rt[rtpos].cChildPos+1 ].symbol.idx )
				++prn.scope;
			print_beautified(parser, parser.rt[rtpos].cChildPos+1);
			if( PT_BLOCK != parser.rt[ parser.rt[rtpos].cChildPos+1 ].symbol.idx )
				--prn.scope;
			if(!prn.newline) prn << '\n';
			print_beautified(parser, parser.rt[rtpos].cChildPos+2);
			prn << "( ";
			print_beautified(parser, parser.rt[rtpos].cChildPos+4);
			prn << " )";
			print_beautified(parser, parser.rt[rtpos].cChildPos+6);
			break;
		}
		case PT_SWITCHSTM:
		{	// switch '(' <Expr> ')' '{' <Case Stms> '}'
			if(!prn.newline) prn << '\n';
			print_beautified(parser, parser.rt[rtpos].cChildPos+0);
			prn << "( ";
			print_beautified(parser, parser.rt[rtpos].cChildPos+2);
			prn << " )\n";
			prn << "{\n";
			this->cHasDefault=false;
			print_beautified(parser, parser.rt[rtpos].cChildPos+5);
			if(!prn.newline) prn << '\n';
			prn << "}\n";
			break;
		}
		case PT_FORSTM:
		{	// for '(' <Arg> ';' <Arg> ';' <Arg> ')' <Normal Stm>
			if(!prn.newline) prn << '\n';
			print_beautified(parser, parser.rt[rtpos].cChildPos+0);
			prn << '(';
			print_beautified(parser, parser.rt[rtpos].cChildPos+2);
			prn << "; ";
			print_beautified(parser, parser.rt[rtpos].cChildPos+4);
			prn << "; ";
			print_beautified(parser, parser.rt[rtpos].cChildPos+6);
			prn << ")\n";
			if( PT_BLOCK != parser.rt[ parser.rt[rtpos].cChildPos+8 ].symbol.idx )
				++prn.scope;
			print_beautified(parser, parser.rt[rtpos].cChildPos+8);
			if( PT_BLOCK != parser.rt[ parser.rt[rtpos].cChildPos+8 ].symbol.idx )
				--prn.scope;
			break;
		}
		case PT_EXPRSTM:
		{
			size_t j,k;
			if(!prn.newline) prn << '\n';
			k = parser.rt[rtpos].cChildPos+parser.rt[rtpos].cChildNum;
			j = parser.rt[rtpos].cChildPos;
			for(; j<k; ++j)
			{
				print_beautified(parser, j);
			}
			break;
		}
		case PT_CASESTM:
		{	// <Case Stms>  ::= case <Value> ':' <Stm List> <Case Stms>
			//			   | default ':' <Stm List> <Case Stms>
			//			   |
			if( parser.rt[rtpos].cChildNum )
			{
				size_t i = parser.rt[rtpos].cChildPos;

				if(!prn.newline) prn << '\n';
				if( PT_CASE == parser.rt[i].symbol.idx )
				{
					prn << "case ";
					++i;
					print_beautified(parser, i);
				}
				else
				{
					if( this->cHasDefault )
					{
						fprintf(stderr, "multiple default cases, line %i",
							(int)parser.rt[i].cToken.line);
					}
					this->cHasDefault=true;
					prn << "default";
				}
				prn << ":\n";
				if( PT_BLOCK != parser.rt[i+2].symbol.idx )
					++prn.scope;
				print_beautified(parser, i+2, parent);
				if( PT_BLOCK != parser.rt[i+2].symbol.idx )
					--prn.scope;
				print_beautified(parser, i+3, parent);
			}
			break;
		}
		case PT_SPECLIST:
		{	// <Spec Item> ',' <Spec List>
			print_beautified(parser, parser.rt[rtpos].cChildPos+0);
			prn << ',' << ' ';
			print_beautified(parser, parser.rt[rtpos].cChildPos+2);
			break;
		}
		case  PT_DEFINEDECL:
		{	// 'define' identifier '=' <Op If> ';'
			prn << "define ";
			print_beautified(parser, parser.rt[rtpos].cChildPos+1);
			prn << ' ' << '=' << ' ';
			print_beautified(parser, parser.rt[rtpos].cChildPos+3);
			prn << ';' << '\n';
			break;
		}
		case PT_INCLUDEDECL:
		{	// 'include' StringLiteral ';'
			prn << "include ";
			print_beautified(parser, parser.rt[rtpos].cChildPos+1);
			prn << ';' << '\n';
			break;
		}
		case PT_FUNCDECL:
		{	// <VarType> identifier '(' <Paramse> ')' ';'
			// <VarType> identifier '(' <Paramse> ')' <Block>
			print_beautified(parser, parser.rt[rtpos].cChildPos+0);
			prn << ' ';
			print_beautified(parser, parser.rt[rtpos].cChildPos+1);
			prn << '(';
			print_beautified(parser, parser.rt[rtpos].cChildPos+3);
			prn << ')';
			print_beautified(parser, parser.rt[rtpos].cChildPos+5);
			if( !prn.newline) prn << '\n';
			break;
		}
		case PT_OBJDECL:
		{	// <Obj Type> <Obj Id> <Obj List> <Obj Script>
			print_beautified(parser, parser.rt[rtpos].cChildPos+0);
			prn << ' ';
			print_beautified(parser, parser.rt[rtpos].cChildPos+1);
			prn << ' ';
			print_beautified(parser, parser.rt[rtpos].cChildPos+2);
			print_beautified(parser, parser.rt[rtpos].cChildPos+3);
			if( !prn.newline) prn << '\n';
			break;
		}
		case PT_OBJLIST:
		{	// <Obj Inst> <Obj List>
			print_beautified(parser, parser.rt[rtpos].cChildPos+0);
			prn << '\n';
			print_beautified(parser, parser.rt[rtpos].cChildPos+1);
			if( !prn.newline) prn << '\n';
			break;
		}
		default:
		{
			size_t j,k;
			k = parser.rt[rtpos].cChildPos+parser.rt[rtpos].cChildNum;
			for(j=parser.rt[rtpos].cChildPos; j<k; ++j)
			{	// go down
				print_beautified(parser, j);
			}
			break;
		}// end default case
		}// end switch
	}
	return ret;
}
