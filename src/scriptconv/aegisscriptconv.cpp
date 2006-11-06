// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder


#include "aegisscriptconv.h"
#include "aegisscript.h"


/// array for compile-emedded script engine
const unsigned char aegisengine[] = 
{
#include "aegisscript.engine.txt"
};


bool aegisparserstorage::getEngine(const unsigned char*& buf, unsigned long& sz)
{
	buf=aegisengine; sz =sizeof(aegisengine);
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////
bool aegisprinter::print_beautified(basics::CParser_CommentStore& parser, int rtpos)
{
	aegisprinter& prn = *this;
	bool ret = true;

	if( parser.rt[rtpos].symbol.Type == 1 )
	{	// terminals
		prn.print_comments(parser, parser.rt[rtpos].cToken.line);
		switch( parser.rt[rtpos].symbol.idx )
		{
		case AE_IDENTIFIER:
		{	
			const char*str = parser.rt[rtpos].cToken.cLexeme;
			// check for reserved words
			// aegis identifiers can start with numbers
			bool reserved = ( 
						(!basics::stringcheck::isalnum(str[0]) && *str!='_') ||
						0==strcmp(str,"if") ||
						0==strcmp(str,"else") ||
						0==strcmp(str,"for") ||
						0==strcmp(str,"while") ||
						0==strcmp(str,"return") ||
						0==strcmp(str,"goto") ||
						0==strcmp(str,"string") ||
						0==strcmp(str,"int") ||
						0==strcmp(str,"double") ||
						0==strcmp(str,"auto") ||
						0==strcmp(str,"var") );

			if(reserved) prn << '_';
			for(; *str; ++str)
				prn << *str;
			if(reserved) prn << '_';
			break;
		}

		case AE_NL:
		case AE_NLOPT:
		case AE_NEWLINE:
			// ignore newline terminals
			break;
		default:
			// print the token as is
			prn << parser.rt[rtpos].cToken.cLexeme;
			break;
		}
	}
	else if( parser.rt[rtpos].cChildNum==1 )
	{	// nonterminal with only one child, just go down
		print_beautified(parser, parser.rt[rtpos].cChildPos);
	}
	else if( parser.rt[rtpos].cChildNum>1 )
	{	// other nonterminals
		switch( parser.rt[rtpos].symbol.idx )
		{
		case AE_LABEL:
		{	// set labels to zero scope
			int tmpscope = prn.scope;
			prn.scope=0;

			//<Label>      ::= identifier ':' <OptValue> <nl>
			//<OptValue>   ::= <Op Unary> |

			// print the name
			print_beautified(parser, parser.rt[rtpos].cChildPos);
			// print the value just attached
			// so "OnTimer: 2000\n" -> "OnTimer2000:\n"
			print_beautified(parser, parser.rt[rtpos].cChildPos+2);
			prn << ":\n";

			prn.scope = tmpscope;
			break;
		}
		case AE_VARDECL:
		{	//'var' <Expr> <nl>
			prn << "temp ";
			print_beautified(parser, parser.rt[rtpos].cChildPos+1);
			prn << ";\n";
			break;
		}
		case AE_CALLSTM:
		{	
			// function name
			print_beautified(parser, parser.rt[rtpos].cChildPos);
			prn << '(';
			// function parameters
			print_beautified(parser, parser.rt[rtpos].cChildPos+1);
			prn << ')';
			break;
		}
		case AE_CASESTM:
		{	// <Case Stm>  ::= case <Value> <nl> <Stm List> <Case Stm>
			//			   | default <nl> <Stm List> <Case Stm>
			//			   |
			if( parser.rt[rtpos].cChildNum )
			{
				print_beautified(parser, parser.rt[rtpos].cChildPos+0);
				prn << ' ';
				print_beautified(parser, parser.rt[rtpos].cChildPos+1);
				prn << ":\n";
				++prn.scope;
				print_beautified(parser, parser.rt[rtpos].cChildPos+3);
				--prn.scope;
				print_beautified(parser, parser.rt[rtpos].cChildPos+4);
			}
			break;
		}
		case AE_NORMALSTM:
		{	// 'if'  <Expr>  <nl> <Stm List> 'endif' <nl>
			// 'if'  <Expr>  <nl> <Stm List> 'else' <nl> <Stm List> 'endif' <nl>
			// 'while'  <Expr>  <nl> <Stm List> 'endwhile' <nl>
			// 'for' identifier '=' <Value> '..' <Value> <nl> <Stm List> 'endfor' <nl>
			// 'do' <nl> <Stm List> 'until'  <Expr>  <nl>
			// 'choose' <Expr> <nl> <Case Stm> 'endchoose' <nl>
			size_t i = parser.rt[rtpos].cChildPos;
			switch( parser.rt[i].symbol.idx )
			{
			case AE_IF:
			{
				prn << "if( ";
				print_beautified(parser, i+1);
				prn << " )\n{\n";
				++prn.scope;
				print_beautified(parser, parser.rt[rtpos].cChildPos+3);
				--prn.scope;
				if(!prn.newline) prn << '\n';
				prn << "}\n";
				if( AE_ELSE == parser.rt[i].symbol.idx )
				{
					prn << "else\n{";
					++prn.scope;
					print_beautified(parser, parser.rt[rtpos].cChildPos+6);
					--prn.scope;
					if(!prn.newline) prn << '\n';
					prn << "}\n";
				}
				break;
			}
			case AE_WHILE:
			{
				prn << "while( ";
				print_beautified(parser, i+1);
				prn << " )\n{\n";
				++prn.scope;
				print_beautified(parser, parser.rt[rtpos].cChildPos+3);
				--prn.scope;
				if(!prn.newline) prn << '\n';
				prn << "}\n";
				break;
			}
			case AE_FOR:
			{
				bool forward = atoi(parser.rt[i+3].cToken.cLexeme) <= atoi(parser.rt[i+5].cToken.cLexeme);
				prn << "for(";
				print_beautified(parser, i+1);
				prn << "=";
				print_beautified(parser, i+3);
				prn << "; ";
				print_beautified(parser, i+1);
				prn << (forward?"<=":">=");
				print_beautified(parser, i+5);
				prn << "; ";
				prn << (forward?"++":"--");
				print_beautified(parser, i+1);
				prn << ")\n{\n";
				++prn.scope;
				print_beautified(parser, i+7);
				--prn.scope;
				if(!prn.newline) prn << '\n';
				prn << "}\n";
				break;
			}			
			case AE_DO:
			{
				prn << "do\n{\n";
				++prn.scope;
				print_beautified(parser, i+2); // <Stm List>
				if(!prn.newline)
					prn << '\n';
				--prn.scope;
				prn << "}\n";
				prn << "while( !(";	// negate because aegis does "until" but we do "while"
				print_beautified(parser, i+4);
				prn << ") );\n";
				break;
			}
			case AE_CHOOSE:
			{
				prn << "switch( ";
				print_beautified(parser, i+1); // <expr>
				prn << " )\n{\n";
				print_beautified(parser, i+3); // <Case Stm>
				if(!prn.newline)
					prn << '\n';
				prn << "}\n";
				break;
			}
			default:
				// ignore, handled otherwise
				break;
			}
			break;
		}
		case AE_CALLLIST:
		{	// <Expr> <Call List>
			// <Expr>
			print_beautified(parser, parser.rt[rtpos].cChildPos);
			if( parser.rt[rtpos].cChildNum > 1 )
			{
				prn << ", ";
				print_beautified(parser, parser.rt[rtpos].cChildPos+1);
			}
			break;
		}
		case AE_LCTRSTM:
		case AE_EXPRSTM:
		{	// 'break' <nl> or 'continue' <nl>
			// <Expr> <nl>
			print_beautified(parser, parser.rt[rtpos].cChildPos);
			if( parser.rt[parser.rt[rtpos].cChildPos].symbol.idx == AE_IDENTIFIER )
			{	// function without parameters
				prn << "()";
			}
			prn << ';';
			prn << '\n';
			break;
		}
		case AE_RETURNSTM:
		{
			// 'return' <arg> <nl>
			// 'return' <nl>
			print_beautified(parser, parser.rt[rtpos].cChildPos);
			if( parser.rt[rtpos].cChildNum > 2 )
			{
				prn << ' ';
				print_beautified(parser, parser.rt[rtpos].cChildPos+1 );
			}
			prn << ";\n";
			break;
		}
		case AE_NPC2:
		{	// <Obj> StringLiteral StringLiteral <Sprite> DecLiteral DecLiteral DecLiteral DecLiteral DecLiteral <nl> <Block>
			// translates to ontouch npc with warp sprite
			size_t i = parser.rt[rtpos].cChildPos;
			char name[32];
			// strip quotes from name
			str2strip_quotes(name, sizeof(name),parser.rt[i+2].cToken.cLexeme);

			static const char* dirnames[8] = 
			{
				"north",
				"northeast",
				"east",
				"southeast",
				"south",
				"southwest",
				"west",
				"northwest",
			};
			prn.print_comments(parser, parser.rt[i+1].cToken.line);
			prn << "npc ";			
			prn.print_id(name);
			prn << " (name=\"";
			prn.print_name(name);
			prn << "\"";

			prn <<  ", map=" << parser.rt[i+1].cToken.cLexeme <<
					", xpos="  << parser.rt[i+4].cToken.cLexeme << 
					", ypos="  << parser.rt[i+5].cToken.cLexeme << 
					", dir=" << dirnames[atoi(parser.rt[i+6].cToken.cLexeme)&0x07] << 
					", sprite=\"" << parser.rt[i+3].cToken.cLexeme << "\"";

			if( 0!=atoi(parser.rt[i+7].cToken.cLexeme) ||
				0!=atoi(parser.rt[i+8].cToken.cLexeme) )
			{
				prn << ", touchup={"
					<< parser.rt[i+6].cToken.cLexeme << ", "
					<< parser.rt[i+7].cToken.cLexeme << '}';
			}
			prn << ')';
			prn << '\n';
			// script body
			prn << "{\n";
			++prn.scope;
			prn << "end;\n";
			// aegis body, should start with a label (most likly OnClick)
			print_beautified(parser, parser.rt[rtpos].cChildPos+10);
			--prn.scope;
			if(!prn.newline) prn << '\n';
			prn << "}\n";
			break;
		}
		case AE_WARP2:
		{	// 'warp' StringLiteral StringLiteral DecLiteral DecLiteral DecLiteral DecLiteral <nl> <Block>
			// translates to ontouch npc with warp sprite
			// can translate it to a real warp later

			size_t i = parser.rt[rtpos].cChildPos;
			char name[32];
			// strip quotes from name
			str2strip_quotes(name, sizeof(name),parser.rt[i+2].cToken.cLexeme);

			prn.print_comments(parser, parser.rt[i].cToken.line);
			prn << "npc ";			
			prn.print_id(name);
			prn << "(name=\"";
			prn.print_name(name);
			prn << "\", pos={" 
				<< parser.rt[i+1].cToken.cLexeme << ", "
				<< parser.rt[i+3].cToken.cLexeme <<", " 
				<< parser.rt[i+4].cToken.cLexeme << '}'
				<< ", sprite=45/*\"WARP\"*/"
			// might add testing for valid touchup values 
			// though warps without would be useless
				   ", touchup={"
				<< parser.rt[i+5].cToken.cLexeme << ", "
				<< parser.rt[i+6].cToken.cLexeme << '}'
				<< ')';
			prn << '\n';
			// script body
			prn << "{\n";
			++prn.scope;
			prn << "end;\n";
			// aegis body, should start with an ontouch label
			print_beautified(parser, parser.rt[rtpos].cChildPos+8);
			--prn.scope;
			if(!prn.newline) prn << '\n';
			prn << "}\n";
			break;
		}
		case AE_NL:
		case AE_NLOPT:
		case AE_NEWLINE:
			// ignore newline terminals
			break;
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


///////////////////////////////////////////////////////////////////////////////////////
bool aegisParser::process(const char*name) const
{
	bool ok = true;
	bool run = true;

	// Open input file
	if( !parser->input.open(name) )
	{
		fprintf(stderr, "Could not open input file %s\n", name);
		return false;
	}
	else
	{
		fprintf(stderr, "processing input file %s\n", name);
	}

	while(run)
	{
		short p = parser->parse(AE_DECL);
		if (p < 0)
		{	// an error
			fprintf(stderr, "Parse Error in file '%s', line %i, col %i\n", name, parser->input.line, parser->input.column);

			parser->print_expects();

			run = false;
			ok = false;
		}
		else if(0 == p)
		{	// finished
			run = false;
		}			
		if( ok && parser->rt[0].symbol.idx==AE_DECL && parser->rt[0].cChildNum )
		{
			basics::CStackElement *child = &(parser->rt[parser->rt[0].cChildPos]);
			if( child &&
				( child->symbol.idx == AE_NPC2 ||
				  child->symbol.idx == AE_WARP2
					
				  )
			  )
			{
				if( (option&OPT_PRINTTREE)==OPT_PRINTTREE )
				{
					fprintf(stderr, "(%li)----------------------------------------\n", (unsigned long)parser->rt.size());
					parser->print_rt_tree(0,0, false);
				}

				if( (option&OPT_TRANSFORM)==OPT_TRANSFORM )
				{
					//////////////////////////////////////////////////////////
					// tree transformation
					parsenode pnode(*parser);
					fprintf(stderr, "----------------------------------------\n");
					pnode.print_tree();
				}
				if( (option&OPT_BEAUTIFY)==OPT_BEAUTIFY && this->prn.output )
				{
					this->prn.scope=0;
					this->prn << '\n';

					//	this->prn << "not yet implemented\n";

					this->prn.print_beautified(*parser, 0);
					this->prn.print_comments(*parser, 0xFFFFFFFF);
				}					
				//////////////////////////////////////////////////////////
				// reinitialize parser
				parser->reinit();
//					fprintf(stderr, "............................................(%i)\n", global::getcount());
			}
		}
	}
	parser->reset();
	return ok;
}


