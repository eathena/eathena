#include "base.h"
#include "showmsg.h"
#include "malloc.h"

#ifdef __GNUC__ 
// message convention for gnu compilers
#warning "INFO: build with gnu compiler, using defines with variable arguments"

#define VPRINTF	vprintf
#define PRINTF	printf

#else // no __GNUC__ 
// message convention for visual c compilers
#pragma message ( "INFO: no gnu compiler, using variable argument functions" )


//  ansi compatible printf with control sequence parser for windows
//  fast hack, handle with care, not everything implemented
//
//  printf("\x1b[1;31;40m");	// Bright red on black
//  printf("\x1b[3;33;45m");	// Blinking yellow on magenta (blink not implemented)
//  printf("\x1b[1;30;47m");	// Bright black (grey) on dim white
//
//  Style           Foreground      Background
//  1st Digit       2nd Digit       3rd Digit		RGB
//  0 - Reset       30 - Black      40 - Black		000
//  1 - FG Bright   31 - Red        41 - Red		100
//  2 - Unknown     32 - Green      42 - Green		010
//  3 - Blink       33 - Yellow     43 - Yellow		110
//  4 - Underline   34 - Blue       44 - Blue		001
//  5 - BG Bright   35 - Magenta    45 - Magenta	101
//  6 - Unknown     36 - Cyan       46 - Cyan		011
//  7 - Reverse     37 - White      47 - White		111
//  8 - Concealed (invisible)
// \033[2J : clear screen and go up/left (0, 0 position)
// \033[K  : clear line from actual position to end of the line

/*
!!todo!!

\033[#;#H - Cursor Position (CUP)
    The first # specifies the line number, the second # specifies the column. The default for both is 1

\033[#A - Cursor Up (CUU)
    Moves the cursor UP # number of lines

\033[#B - Cursor Down (CUD)
    Moves the cursor DOWN # number of lines

\033[#C - Cursor Forward (CUF)
    Moves the cursor RIGHT # number of columns

\033[#D - Cursor Backward (CUB)
    Moves the cursor LEFT # number of columns

\033[#;#f - Horizontal & Vertical Position
    (same as \033[#;#H)

\033[s - Save Cursor Position (SCP)
    The current cursor position is saved. 

\033[u - Restore cursor position (RCP)
    Restores the cursor position saved with the (SCP) sequence \033[s.
	(addition, restore to 0,0 if nothinh was saved before)

\033[2J - Erase Display (ED)
    Clears the screen and moves to the home position

\033[K - Erase Line (EL)
    Clears the current line from the cursor position

\033[#;...;#m - Set Graphics Rendition (SGR) 

*/








int	VPRINTF(const char *fmt, va_list argptr)
{
	static char		tempbuf[4096]; // initially using a static fixed buffer size 
	static Mutex	mtx;
	ScopeLock		sl(mtx);
	size_t sz  = 4096; // initial buffer size
	unsigned long	written;
	static HANDLE	handle;
	char *p, *q, *ibuf = tempbuf;

	if (!handle)
		handle = GetStdHandle(STD_OUTPUT_HANDLE);


	do{
		// print
		if( vsnprintf(ibuf, sz, fmt, argptr) >=0 ) // returns -1 in case of error
			break; // print ok, can break
		// otherwise
		// aFree the memory if it was dynamically alloced
		if(ibuf!=tempbuf) aFree(ibuf);
		// double the size of the buffer
		sz *= 2;
		ibuf = (char*)aMalloc( sz*sizeof(char));
		// and loop in again
	}while(1); 


	// start with processing
	p = ibuf;
	while ((q = strchr(p, 0x1b)) != NULL)
	{	// find the escape character
		if( 0==WriteConsole(handle, p, q-p, &written, 0) ) // write up to the escape
			WriteFile(handle, p, q-p, &written, 0);


		if( q[1]!='[' )
		{	// write the escape char (whatever purpose it has) 
			if(0==WriteConsole(handle, q, 1, &written, 0) )
				WriteFile(handle,q, 1, &written, 0);
			p=q+1;	//and start searching again
		}
		else
		{	// from here, we will skip the '\033['
			// we break at the first unprocessible position
			// assuming regular text is starting there
			q=q+2;	// skip escape and bracket
			if( (q[0]=='2') && (q[1]=='J') )
			{	//// \033[2J : clear screen and go up/left (0, 0 position)
				
				CONSOLE_SCREEN_BUFFER_INFO info;
				const COORD origin = {0,0};
				int cnt;
				GetConsoleScreenBufferInfo(handle,&info);	// Get screen rows and columns.
				cnt = info.dwSize.X * info.dwSize.Y;	// Number of chars on screen.
				
				FillConsoleOutputAttribute(handle,info.wAttributes,cnt,origin,NULL);
				FillConsoleOutputCharacter(handle,' ',             cnt,origin,NULL);
				SetConsoleCursorPosition(handle, origin); 

				p = q+2; // skip the sequence and search again
			}
			else if( (q[0]=='K') )
			{	//// \033[K  : clear line from actual position to end of the line
				SHORT cnt;
				CONSOLE_SCREEN_BUFFER_INFO info;
				GetConsoleScreenBufferInfo(handle, &info);
				cnt = info.dwSize.X - info.dwCursorPosition.X; // how many spaces until line is full
				FillConsoleOutputAttribute(handle, info.wAttributes, cnt, info.dwCursorPosition, NULL);
				FillConsoleOutputCharacter(handle, ' ',              cnt, info.dwCursorPosition, NULL);

				p = q+1; // skip the sequence and search again
			}
			else
			{	// we need to parse for sequence number
				CONSOLE_SCREEN_BUFFER_INFO info;
				GetConsoleScreenBufferInfo(handle, &info);
				while(1)
				{
					if( !isdigit((int)((unsigned char)*q)) ) 
					{	// no number
						// something is fishy, we break
						p=q;
						break;
					}

					switch(*q)
					{
					case '0': // reset
						info.wAttributes = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
						break;
					case '1': // set foreground intensity
						info.wAttributes |= FOREGROUND_INTENSITY;
						break;
					case '5':  // set background intensity
						info.wAttributes |= BACKGROUND_INTENSITY;
						break;
					case '7': // reverse colors (just xor them)
						info.wAttributes ^= FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE |
											BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE;
						break;
					case '3':
						if( isdigit((int)((unsigned char)q[1])) ) // a two digit number
						{	// foreground
							int num = q[1]-'0';
							if(num==9) info.wAttributes |= FOREGROUND_INTENSITY;
							if(num>7) num=7;	// set white for 37, 38 and 39
							info.wAttributes &= ~(FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE);
							if( (num & 0x01)>0 ) // lowest bit set = red
								info.wAttributes |= FOREGROUND_RED;
							if( (num & 0x02)>0 ) // second bit set = green
								info.wAttributes |= FOREGROUND_GREEN;
							if( (num & 0x04)>0 ) // third bit set = blue
								info.wAttributes |= FOREGROUND_BLUE;
						}
						// otherwise it is for blinking
						// which is not implemented
						break;
					case '4':
						if( isdigit((int)((unsigned char)q[1])) ) // a two digit number
						{	// background
							int num = q[1]-'0';
							if(num==9) info.wAttributes |= BACKGROUND_INTENSITY;
							if(num>7) num=7;	// set white for 47, 48 and 49
							info.wAttributes &= ~(BACKGROUND_RED|BACKGROUND_GREEN|BACKGROUND_BLUE);
							if( (num & 0x01)>0 ) // lowest bit set = red
								info.wAttributes |= BACKGROUND_RED;
							if( (num & 0x02)>0 ) // second bit set = green
								info.wAttributes |= BACKGROUND_GREEN;
							if( (num & 0x04)>0 ) // third bit set = blue
								info.wAttributes |= BACKGROUND_BLUE;
						}
						// otherwise it is for underline
						// which is not implemented
						break;
					// not implemented at all
					case '2':
					case '6':
					case '8':
					case '9':
						break;
					}//end case

					if( isdigit((int)((unsigned char)q[1])) ) // skip any one or two digit numbers
						q++;
					q++;

					if(*q != ';')		// no seperator? 
					{					// then we do not have to go on
						if(*q == 'm')	// check for finish marker
							q++;		// skip it when found
						p=q;			// but we also break when not found
						break;	
					}
					q++;				// we have to go on with the next char after the seperator
				}// end while
				SetConsoleTextAttribute(handle, info.wAttributes); // set the attribute
			}
		}
	}
	if (*p)	// write the rest of the buffer
		if( 0==WriteConsole(handle, p, strlen(p), &written, 0) )
			WriteFile(handle,p, strlen(p), &written, 0);
	// aFree the buffer if allocated dynamically
	if(ibuf!=tempbuf) aFree(ibuf);
	return 0;
}

int	PRINTF(const char *fmt, ...)
{	
	int ret;
	va_list argptr;
	va_start(argptr, fmt);
	ret = VPRINTF(fmt,argptr);
	va_end(argptr);
	return ret;
}

#endif// no __GNUC__ 



int _vShowMessage(enum msg_type flag, const char *string, va_list ap)
{	// Return: 0 = Successful, 1 = Failed.
	const char *prefix = "";

	if(flag == MSG_DEBUG && !SHOW_DEBUG_MSG)
		return 0;

	if( (NULL==string) || (0==*string) ) {
		ShowError("Empty string flag %i passed to _ShowMessage().\n", flag);
		return 1;
	}
	switch (flag) {
		case MSG_NONE:	// just a message without prefix
			break;
		case MSG_STATUS: //Bright Green (To inform about good things)
			prefix = CL_BT_GREEN"[Status]"CL_RESET": ";
			break;
		case MSG_SQL: //Bright Violet (For dumping out anything related with SQL)
			prefix = CL_BT_MAGENTA"[SQL]"CL_RESET":    ";
			break;
		case MSG_INFORMATION: //Bright White (Variable information)
			prefix = CL_BT_WHITE"[Info]"CL_RESET":   ";
			break;
		case MSG_CONSOLE: //
			prefix = CL_LT_CYAN"[Console]"CL_RESET":";
			break;
		case MSG_NOTICE: //Bright White (Less than a warning)
			prefix = CL_BT_WHITE"[Notice]"CL_RESET": ";
			break;
		case MSG_WARNING: //Bright Yellow
			prefix = CL_BT_YELLOW"[Warning]"CL_RESET":";
			break;
		case MSG_DEBUG:
			prefix = CL_BT_CYAN"[Debug]"CL_RESET":  ";
			break;
		case MSG_ERROR: //Bright Red  (Regular errors)
			prefix = CL_BT_RED"[Error]"CL_RESET":  ";
			break;
		case MSG_FATALERROR: //Bright Red (Fatal errors, abort(); if possible)
			prefix = CL_BT_RED"[Fatal]"CL_RESET":  ";
			break;
		default:
			ShowError("In function _ShowMessage() -> Invalid flag passed.\n");
			return 1;
	}

	if(flag!=MSG_NONE)
		PRINTF("%s ", prefix);
	VPRINTF(string, ap);
	fflush(stdout);

	return 0;
}


int _ShowMessage(enum msg_type flag, const char *string, ...){ 

	int ret;
	va_list ap;
	va_start(ap,string);
	ret = _vShowMessage(flag,string,ap);
	va_end(ap);
	return ret;
}
