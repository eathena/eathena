#include "basepacket.h"



///////////////////////////////////////////////////////////////////////////////
NAMESPACE_BEGIN(basics)
///////////////////////////////////////////////////////////////////////////////


packetbase::packetbase()
:	_buf(NULL)
,	_len(0)
,	_max(0)
,	_fields()
{ }

packetbase::packetbase(size_t num_fields)
:	_buf(NULL)
,	_len(0)
,	_max(0)
,	_fields(num_fields)
{ }

packetbase::~packetbase()
{
	delete [] _buf;
	_buf = NULL;
	_len = 0;
	_max = 0;
	_fields.clear();
}

void packetbase::init()
{
	ptrvector<packetfield>::iterator field(_fields);
	if( _buf == NULL )
	{// create the buffer
		for( _max = 0; field; field++ )
			_max += field->_sizeof();
		_len = _max;
		_buf = new uint8[_len];
		field = _fields;
	}
	// set the buffer of the fields
	size_t off;
	for( off = 0; field; field++ )
	{
		field->_setbuf(_buf + off);
		off += field->_sizeof();
	}
}

/// One of the fields was resized.
/// Move data around and realloc the buffer if necessary.
void packetbase::on_resize(packetfield& target, size_t new_len)
{
	ptrvector<packetfield>::iterator field(_fields);
	size_t off;

	// get target field
	for( off = 0; field; field++ )
	{
		if( field() == &target )
			break;// target found
		off += field->_sizeof();
	}
	if( !field )
		return;// not found - TODO error message or exception

	size_t old_len = field->_sizeof();

	if( new_len > old_len )
	{// size increased
		size_t needed = new_len - old_len;
		size_t available = _max - _len;

		if( available < needed )
		{// not enough space - reallocate
			_max += needed;
			uint8* tmp = new uint8[_max];
			memcpy(tmp, _buf, off+old_len);// start -> old_len
			memset(tmp+off+old_len, 0, needed);// old_len -> new_len
			memcpy(tmp+off+new_len, _buf+off+old_len, _len-off-old_len);// new_len -> end
			delete [] _buf;
			_buf = tmp;

			// update field buffers up to the target
			ptrvector<packetfield>::iterator it(_fields);
			for( off = 0; it < field; it++ )
			{
				it->_setbuf(_buf + off);
				off += it->_sizeof();
			}
			field->_setbuf(_buf + off);
		}
		else
		{// move the rest of the data
			memmove(_buf+off+new_len, _buf+off+old_len, _len-off-old_len);
		}
		_len += needed;
	}
	else
	{// size decreased
		// move the rest of the data back
		memmove(_buf+off+new_len, _buf+off+old_len, _len-off-old_len);
		_len -= old_len - new_len;
	}

	// update field buffers after the target
	for( field++; field; field++ )
	{
		field->_setbuf(_buf + off);
		off += field->_sizeof();
	}
}

/// Length of the packet
size_t packetbase::length() const
{ 
	return _len;
}

/// Data of the packet
const uint8* packetbase::buffer() const
{ 
	return _buf;
}

///////////////////////////////////////////////////////////////////////////////
// Field with a fixed size
//

/// Size of this field
template<size_t LENGTH>
size_t staticfield<LENGTH>::_sizeof() const
{
	return LENGTH;
}

/// Set the buffer of the field
template<size_t LENGTH>
void staticfield<LENGTH>::_setbuf(uint8* buf)
{
	_buf = buf;
}

///////////////////////////////////////////////////////////////////////////////
// Byte field
field_B::operator uint8() const
{
	return this->operator()();
}

uint8 field_B::operator()() const
{
	return _buf[0];
}

field_B& field_B::operator=(const field_B& b)
{
	_buf[0] = b._buf[0];
	return *this;
}

uint8 field_B::operator=(uint8 val)
{
	_buf[0] = val;
	return val;
}

///////////////////////////////////////////////////////////////////////////////
// Word field
field_W::operator uint16() const
{
	return this->operator()();
}

uint16 field_W::operator()() const
{
	return
		( uint16(_buf[0])         )|
		( uint16(_buf[1]) << 0x08 );
}

field_W& field_W::operator=(const field_W& w)
{
	memcpy(_buf, w._buf, 2);
	return *this;
}

uint16 field_W::operator=(uint16 val)
{
	_buf[0] = uint8( (val & 0x00FF)         );
	_buf[1] = uint8( (val & 0xFF00) >> 0x08 );
	return val;
}

///////////////////////////////////////////////////////////////////////////////
// Long field
field_L::operator uint32() const
{
	return this->operator()();
}

uint32 field_L::operator()() const
{
	return
		( uint32(_buf[0])         )|
		( uint32(_buf[1]) << 0x08 )|
		( uint32(_buf[2]) << 0x10 )|
		( uint32(_buf[3]) << 0x18 );
}

field_L& field_L::operator=(const field_L& w)
{
	memcpy(_buf, w._buf, 4);
	return *this;
}

uint32 field_L::operator=(uint32 val)
{
	_buf[0] = uint8( (val & 0x000000FF)         );
	_buf[1] = uint8( (val & 0x0000FF00) >> 0x08 );
	_buf[2] = uint8( (val & 0x00FF0000) >> 0x10 );
	_buf[3] = uint8( (val & 0xFF000000) >> 0x18 );
	return val;
}

///////////////////////////////////////////////////////////////////////////////
/// String - fixed size
template<size_t LENGTH>
field_staticstring<LENGTH>::operator const uint8*&()
{
	return this->operator()();
}

template<size_t LENGTH>
const uint8*& field_staticstring<LENGTH>::operator()() const
{
	return _buf;
}

template<size_t LENGTH>
size_t field_staticstring<LENGTH>::length() const
{
	return strnlen(_buf, LENGTH);
}

template<size_t LENGTH>
size_t field_staticstring<LENGTH>::capacity() const
{
	return LENGTH;
}

template<size_t LENGTH>
field_staticstring<LENGTH>& field_staticstring<LENGTH>::operator=(const field_staticstring<LENGTH>& str)
{
	strncpy(_buf, str._buf, LENGTH);
	return *this;
}

template<size_t LENGTH>
const uint8*& field_staticstring<LENGTH>::operator=(const uint8* str)
{
	strncpy(_buf, str, LENGTH);
	return *this;
}

///////////////////////////////////////////////////////////////////////////////
/// Array - fixed size
template<class T, size_t SIZE>
T& field_staticarray<T,SIZE>::operator[](size_t inx)
{
	return _arr[i];
}

template<class T, size_t SIZE>
size_t field_staticarray<T,SIZE>::_sizeof() const
{
	size_t size = 0;
	size_t i;
	for( i = 0; i < SIZE; i++ )
		size += _arr[i]._sizeof();
	return size;
}

template<class T, size_t SIZE>
void field_staticarray<T,SIZE>::_setbuf(uint8* buf)
{
	size_t off = 0;
	size_t i;
	for( i = 0; i < SIZE; i++ )
	{
		_arr[i]._setbuf(buf + off);
		off += _arr[i]._sizeof();
	}
}



///////////////////////////////////////////////////////////////////////////////
void test_packet(void)
{
#if defined(DEBUG)
	//## TODO
#endif//DEBUG
}



///////////////////////////////////////////////////////////////////////////////
NAMESPACE_END(basics)
///////////////////////////////////////////////////////////////////////////////
