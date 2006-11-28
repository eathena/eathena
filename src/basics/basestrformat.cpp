#include "basestrformat.h"
#include "basestring.h"
#include "baseinet.h"



#include <limits.h>
#include <float.h>
#include <ctype.h>

NAMESPACE_BEGIN(basics)


/*------------------------------------------------------------------------------

  Author:    Andy Rushton
  Copyright: (c) Andy Rushton, 2004
  License:   BSD License, see ../docs/license.html


  I have chosen to partially re-invent the wheel here. This is because the obvious
  solution to the problem of in-memory formatted output is to use sprintf(), but
  this is a potentially dangerous operation since it will quite happily charge off
  the end of the string it is printing to and thereby corrupt memory. Building in
  potential bear-traps by using arbitrary-sized internal buffers is not part of
  any quality-orientated design philosophy. Simply buggering around with huge
  buffers is not a solution to this problem, it just pushes the problem into a
  different shape.

  However, sprintf() is acceptable if used in strictly controlled conditions that
  make overflow impossible. This is what I do here. I break the format string up
  to get the individual formatting codes for each argument and use sprintf() to
  format just the numeric substitutions. String substitutions are handled
  directly.

  Notes:

  serious problems apparently with unsigned short - getting the argument value doesn't work
  at least on SunOS4. Therefore I will pass this value as an unsigned int and see if that fixes it.

--------------------------------------------------------------------------------*/




/*
static const int max_int_length = 20;  // allow for up to 64 bits;
static const int max_mantissa_length = (DBL_MAX_EXP*4/10);


static ssize_t my_snprintf(char   *buf, size_t sz, const char   * fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	ssize_t result = vsnprintf(buf, sz, fmt, args);
	va_end(args);
	return result;
}
static ssize_t my_snprintf(wchar_t*buf, size_t sz, const wchar_t* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	ssize_t result = vswprintf(buf, sz, fmt, args);
	va_end(args);
	return result;
}
*/
////////////////////////////////////////////////////////////////////////////////

template<typename T> ssize_t dvsprintf(stringoperator<T>& formatted, const T* fmt, va_list args)
{
	ssize_t start_length = formatted.size();
	if(fmt)
	{	
		while (*fmt)
		{
			switch (*fmt)
			{
			case '\\':
			{
				fmt++;
				switch (*fmt)
				{
				case 'b' : formatted.append('\b',1); fmt++; break;
				case 'f' : formatted.append('\f',1); fmt++; break;
				case 'n' : formatted.append('\n',1); fmt++; break;
				case 'r' : formatted.append('\r',1); fmt++; break;
				case 't' : formatted.append('\t',1); fmt++; break;
				case 'v' : formatted.append('\v',1); fmt++; break;
				case '\\': formatted.append('\\',1); fmt++; break;
				case '\?': formatted.append('\?',1); fmt++; break;
				case '\'': formatted.append('\'',1); fmt++; break;
				case '\"': formatted.append('\"',1); fmt++; break;
				default: break;
				}
				break;
			}
			case '%':
			{
				bool left_justified = false;
				bool alternate = false;
				char pluschar = '\0';
				char padchar  = '\0';


				// [ flags ]
				bool found = true;
				while(found && *fmt)
				{
					fmt++;
					switch (*fmt)
					{
					case '-': 
						left_justified = true;
						break;
					case '+':
					case ' ':
						pluschar = *fmt;
						break;
					case '0':
						padchar = *fmt;
						break;
					case '#':
						alternate = true;
						break;
					default: 
						found = false;
						break;
					}
				}
				
				// [ width ]
				int width = 0;
				if (*fmt == '*')
				{
					fmt++;
					width = (int)va_arg(args, size_t);
					if(width < 0)
					{
						left_justified = true;
						width = -width;
					}
				}
				else
				{
					while( stringcheck::isdigit(*fmt) )
					{
						width *= 10;
						width +=(*fmt++ - '0');
					}
				}

				// [ . precision ]
				int precision = -1;
				if (*fmt == '.')
				{
					fmt++;
					if (*fmt == '*')
					{
						fmt++;
						precision = (int)va_arg(args, size_t);
						if (precision < 0)
						{
							left_justified = true;
							precision = -precision;
						}
					}
					else
					{
						precision = 0;
						while( stringcheck::isdigit(*fmt) )
						{
							precision *= 10;
							precision +=(*fmt++ - '0');
						}
					}
				}

				// [ modifier ]
				T modifier = '\0';
				switch (*fmt)
				{
				case 'h': 
				case 'l': case 'L':
					modifier = *fmt++;
					break;
				default:
					break;
				}

				// [ conversion ]
				T conversion = *fmt++;
				switch (conversion)
				{
				case 'a': 
				{
					ipaddress value =(ulong)va_arg(args, size_t);
					formatted << value;
					break;
				}
				case 'd': 
				case 'i':
				{
					int64 value;
					switch (modifier)
					{
					case 'h':
						value =(short)va_arg(args, size_t);
						break;
					case 'l':
						value = (long)va_arg(args, size_t);
						break;
					default:
						value = (int)va_arg(args, size_t);
					break;
					}

					_itostring<T>(formatted, value, 10, true, width, left_justified, padchar, pluschar);
					
					break;
				}
				case 'u':
				case 'o':
				case 'X': case 'x':
				{
					uint64 value;
					switch (modifier)
					{
					case 'h':
						value =(ushort)va_arg(args, size_t);
						break;
					case 'l':
						value = (unsigned long)va_arg(args, size_t);
						break;
					default:
						value = (unsigned int)va_arg(args, size_t);
						break;
					}

					if( conversion=='o' )
					{	// oct
						_octtostring<T>(formatted, value, width, left_justified, padchar, alternate);
					}
					else if( conversion=='x' || conversion=='X' )
					{	// hex
						_hextostring<T>(formatted, value, width, left_justified, padchar, alternate, conversion=='x');
					}
					else
					{	// decimal
						_itostring<T>(formatted, value, 10, false, width, left_justified, padchar, pluschar);
					}
					break;
				}
				case 'p':
				{
					uint64 value = (uint64)((size_t)va_arg(args, void*));
					// have two hex digits per byte
					int psz = 2*sizeof(void*) + 2*alternate;
					// in case of padding zeros, add the 0x before the zeros
					if(alternate && padchar=='0')
					{
						formatted.append('0', 1);
						formatted.append('x', 1);
					}
					// add the left padding 
					if(!left_justified && width > psz )
						formatted.append( (padchar=='\0')?' ':padchar, width-psz);
					// when not padding zeros, add the 0x just at the number
					if(alternate && padchar!='0')
					{
						formatted.append('0', 1);
						formatted.append('x', 1);
					}
					// print pointers with fixed void* size at the current system (8 digits for 32bit / 16 for 64bit)
					_hextostring<T>(formatted, value, 2*sizeof(void*), false, '0', false, true);
					if( left_justified && width > psz )
						formatted.append(' ', width-psz);
					
					break;
				}
				case 'f':
				case 'E': case 'e':
				case 'G': case 'g':
				{
					double value = (modifier == 'L') ? (double)va_arg(args, long double):(double)va_arg(args, double);

					_ftostring<T>(formatted, value, precision, conversion, width, left_justified, padchar, pluschar, alternate);

					break;
				}
				case 'c':
				case 'C':
				{
					T value =(T)va_arg(args, size_t);
					if(!left_justified && width>1) formatted.append(' ',width-1);
					formatted.append(value,1);
					if (left_justified && width>1) formatted.append(' ',width-1);
					break;
				}
				case 's':
				{
					T* value = va_arg(args, T*);
					size_t length = hstrlen(value);
					if( precision>0 && length>(size_t)precision ) length = precision;
					if( !left_justified && width>(int)length) formatted.append(' ',width-length);
					formatted.append(value, length);
					if(  left_justified && width>(int)length) formatted.append(' ',width-length);
					
					break;
				}
				case 'n':
				{
					int* result = va_arg(args, int*);
					*result = formatted.size() - start_length;
					break;
				}
				case '%':
				default:
					formatted.append(conversion,1);
					break;
				}
				break;
			}
			default:
				formatted.append(*fmt++,1);
				break;
			}
		}
	}
	return formatted.size() - start_length;
}
template ssize_t dvsprintf<char   >(stringoperator<char   >& formatted, const char   * fmt, va_list args);
template ssize_t dvsprintf<wchar_t>(stringoperator<wchar_t>& formatted, const wchar_t* fmt, va_list args);


template<typename T> ssize_t dsprintf(stringoperator<T>& formatted, const T* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	ssize_t result = dvsprintf(formatted, fmt, args);
	va_end(args);
	return result;
}
template ssize_t dsprintf<char   >(stringoperator<char   >& formatted, const char   * fmt, ...);
template ssize_t dsprintf<wchar_t>(stringoperator<wchar_t>& formatted, const wchar_t* fmt, ...);


template<typename T> string<T> dvprintf(const T* fmt, va_list args)
{
	string<T> formatted;
	dvsprintf(formatted, fmt, args);
	return formatted;
}
template string<char   > dvprintf<char   >(const char   * fmt, va_list args);
template string<wchar_t> dvprintf<wchar_t>(const wchar_t* fmt, va_list args);


template<typename T> string<T> dprintf(const T* fmt, ...)
{
	string<T> formatted;
	va_list args;
	va_start(args, fmt);
	dvsprintf(formatted, fmt, args);
	va_end(args);
	return formatted;
}
template string<char   > dprintf<char   >(const char   * fmt, ...);
template string<wchar_t> dprintf<wchar_t>(const wchar_t* fmt, ...);




NAMESPACE_END(basics)
