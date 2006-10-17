
#include "basevariant.h"
#include "basestring.h"
		

///////////////////////////////////////////////////////////////////////////////
NAMESPACE_BEGIN(basics)
///////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////
//
void Variant::_value::assign(const _value &v)
{
	clear();
	switch(v.cType)
	{
	case VAR_INTEGER:
		cType = VAR_INTEGER;
		cInteger = v.cInteger; 
		break;
	case VAR_FLOAT:
		cType = VAR_FLOAT;
		cFloat = v.cFloat; 
		break;
	case VAR_STRING:
		cType = VAR_STRING;
		cString = new string<>( *v.cString );
		break;
	case VAR_ARRAY:
		cType = VAR_ARRAY;
		cArray = new vector<Variant>( *v.cArray );
		break;
	case VAR_NONE: // already initialized for this
	default:
		break;
	}
}
void Variant::_value::assign(const int val)
{
	clear();
	cType = VAR_INTEGER;
	cInteger = val; 
}
void Variant::_value::assign(const double val)
{
	clear();
	cType = VAR_FLOAT;
	cFloat = val;
}
void Variant::_value::assign(const char* val)
{
	clear();
	cType = VAR_STRING;
	cString = new string<>(val);
}
void Variant::_value::assign(const string<>& val)
{
	clear();
	cType = VAR_STRING;
	cString = new string<>(val);
}

///////////////////////////////////////////////////////////////////////
//
void Variant::_value::clear()
{	
	switch(cType)
	{
	case VAR_ARRAY:
		if(cArray) delete cArray;
		break;
	case VAR_STRING:
		if(cString) delete cString;
		break;
	case VAR_INTEGER:
	case VAR_FLOAT:
	default:
		break;
	}
	cType = VAR_NONE;
	cInteger = 0;
}

///////////////////////////////////////////////////////////////////////
void Variant::_value::setarray(size_t cnt)
{	
	if( this->cType == VAR_ARRAY )
	{	// resize the current array
		this->cArray->resize(cnt);
	}
	else
	{	// generate an array from this node and set cArray[0] with current content
		vector<Variant> *temp = new vector<Variant>;
		temp->resize(cnt);

		// copy stuff into cArray[0]
		temp->operator[](0).cValue->cType = this->cType;
		switch(this->cType)
		{
		case VAR_INTEGER:
			temp->operator[](0).cValue->cInteger = this->cInteger; 
			break;
		case VAR_FLOAT:
			temp->operator[](0).cValue->cFloat = this->cFloat; 
			break;
		case VAR_STRING:
			temp->operator[](0).cValue->cString = this->cString;
			break;
		case VAR_ARRAY:
			temp->operator[](0).cValue->cArray = this->cArray;
			break;
		case VAR_NONE: // already initialized for this
		default:
			break;
		}
		// make this an array
		this->cType = VAR_ARRAY;
		this->cArray= temp;
	}
}
///////////////////////////////////////////////////////////////////////
void Variant::_value::addarray(size_t cnt)
{	
	if( this->cType == VAR_ARRAY )
	{	// go into elements
		vector<Variant>::iterator iter(*cArray);
		for(; iter; ++iter)
			iter->addarray(cnt);
	}
	else
	{	// make this node an array
		setarray(cnt);
	}
}




///////////////////////////////////////////////////////////////////////////
// Type Initialize Variant (aka copy)
void Variant::assign(const Variant& v, bool ref)
{
	if( cValue.isAutoCreate() )
		cValue = v.cValue;			// just share the pointer
	else
		*(cValue) = *(v.cValue);	// copy the content
	if(ref) cValue.setAutoCreate();	// disable copy-on-write for references
}
///////////////////////////////////////////////////////////////////////////
// Type Initialize Integer
void Variant::assign(int val)
{
	cValue->assign(val);
}
///////////////////////////////////////////////////////////////////////////
// Type Initialize double
void Variant::assign(double val)
{
	cValue->assign(val);
}
///////////////////////////////////////////////////////////////////////////
// Type Initialize String
void Variant::assign(const char* val)
{
	cValue->assign(val);
}
///////////////////////////////////////////////////////////////////////////
// Create Array from actual element
void Variant::setarray(size_t cnt)
{	
	if(cnt>0)
	{
		cValue->setarray(cnt);
	}
	else
		clear();
}
///////////////////////////////////////////////////////////////////////////
// Create Array by vectorizing deepest elements
void Variant::addarray(size_t cnt)
{	
	if(cnt>0)
	{
		cValue->addarray(cnt);
	}
	else
		clear();
}



///////////////////////////////////////////////////////////////////////////
// Access to array elements
const Variant& Variant::operator[](size_t inx) const
{	// on Arrays return the element, out of bounds return element 0
	// other array accesses return this object
	return (cValue->isArray()) ? ((inx<cValue->size()) ? cValue->cArray->operator[](inx) : cValue->cArray->operator[](0)) : *this;
	return *this;
}
Variant& Variant::operator[](size_t inx)
{	// on Arrays return the element, out of bounds return element 0
	// other array accesses return this object
	return (cValue->isArray()) ? ((inx<cValue->size()) ? cValue->cArray->operator[](inx) : cValue->cArray->operator[](0)) : *this;
	return *this;
}

///////////////////////////////////////////////////////////////////////////
// local conversions
void Variant::convert2string()
{
	if(cValue->cType!=VAR_STRING)
	{
		switch(cValue->cType)
		{
		case VAR_NONE:
			assign("");
			break;
		case VAR_INTEGER:
			assign( string<>((int)(cValue->cInteger)) );
			break;
		case VAR_FLOAT:
			assign( string<>(cValue->cFloat) );
			break;
		case VAR_STRING:
			break;
		case VAR_ARRAY:
			assign( cValue->cArray->operator[](0).getString() );
			break;
		}
	}
}
void Variant::convert2float()
{
	if(cValue->cType!=VAR_FLOAT)
	{
		assign( this->getFloat() );
	}
}
void Variant::convert2int()
{
	if(cValue->cType!=VAR_INTEGER)
	{
		assign( this->getInt() );
	}
}
void Variant::convert2number()
{
	if(cValue->cType!=VAR_INTEGER && cValue->cType!=VAR_FLOAT)
	{
		if( this->getFloat() != floor(this->getFloat()) )
			assign( this->getFloat() );
		else
			assign( this->getInt() );
	}
}
void Variant::cast(int type)
{	
	switch(type)
	{
	case VAR_INTEGER:
		convert2int();
	case VAR_STRING:
		convert2string();
	case VAR_FLOAT:
		convert2float();
	}
}

///////////////////////////////////////////////////////////////////////////
// access conversion 
string<> Variant::getString() const
{
	switch(cValue->cType)
	{
	case VAR_NONE:
		break;
	case VAR_INTEGER:
		return string<>((int)(cValue->cInteger));
	case VAR_FLOAT:
		return string<>(cValue->cFloat);
	case VAR_STRING:
		return *cValue->cString;
	case VAR_ARRAY:
		return cValue->cArray->operator[](0).getString();
	}	
	return string<>("");
}
double Variant::getFloat() const
{
	switch(cValue->cType)
	{
	case VAR_NONE:
		break;
	case VAR_INTEGER:
		return (int)(cValue->cInteger);
	case VAR_FLOAT:
		return cValue->cFloat;
	case VAR_STRING:
		return atof(*cValue->cString);
	case VAR_ARRAY:
		return cValue->cArray->operator[](0).getFloat();
	}	
	return 0.0;
}
int Variant::getInt() const
{
	switch(cValue->cType)
	{
	case VAR_NONE:
		break;
	case VAR_INTEGER:
		return (int)(cValue->cInteger);
	case VAR_FLOAT:
		return (int)( floor(0.5+cValue->cFloat) );
	case VAR_STRING:
		return atoi(*cValue->cString);
	case VAR_ARRAY:
		return cValue->cArray->operator[](0).getInt();
	}	
	return 0;
}

///////////////////////////////////////////////////////////////////////////
// unary operations
const Variant Variant::operator-() const
{
	if( isValid() )
	{
		Variant temp(*this);
		if( temp.isArray() )
		{	// go through the array
			vector<Variant>::iterator iter(*temp.cValue->cArray);
			for(; iter; ++iter)
				*iter = -(*iter);
		}
		else 
		{
			temp.convert2number();
			if( temp.isFloat() )
				temp.cValue->cFloat = -(temp.cValue->cFloat);
			else
				temp.cValue->cInteger = -(temp.cValue->cInteger);
		}
		return temp;
	}
	return *this;
}
const Variant Variant::operator~() const
{
	if( isValid() )
	{
		Variant temp(*this);
		if( temp.isArray() )
		{	// go through the array
			vector<Variant>::iterator iter(*temp.cValue->cArray);
			for(; iter; ++iter)
				*iter = ~(*iter);
		}
		else
		{
			temp.convert2int();
			temp.cValue->cInteger = ~(temp.cValue->cInteger);
		}
		return temp;
	}
	return *this;
}
const Variant Variant::operator!() const
{
	if( isValid() )
	{
		Variant temp(*this);
		if( temp.isArray() )
		{	// go through the array
			vector<Variant>::iterator iter(*temp.cValue->cArray);
			for(; iter; ++iter)
				*iter = !(*iter);
		}
		else
		{
			temp.convert2int();
			temp.cValue->cInteger = !(temp.cValue->cInteger);
		}
		return temp;
	}
	return *this;
}



///////////////////////////////////////////////////////////////////////////
// arithmetic operations
const Variant& Variant::operator+=(const Variant& v)
{
	if( !isValid() || !v.isValid() )
	{	// invalid inputs, invalid output
		clear();
	}
	////////////////////////////
	// simple types at the end
	else if( isArray() && v.isArray() )
	{	// add two arrays
		// don't check for dimension mismatches, just pad with zeros
		vector<Variant>::iterator iterA(*this->cValue->cArray);
		vector<Variant>::iterator iterV(*v.cValue->cArray);
		for(; iterA && iterV; ++iterA, ++iterV)
			*iterA += *iterV;
	}
	else if( isArray() && !v.isArray() )
	{	// add a given skalar recursively to a local array
		vector<Variant>::iterator iterA(*this->cValue->cArray);
		for(; iterA; ++iterA)
			*iterA += v;
	}
	else if( !isArray() && v.isArray() )
	{	// add a local skalar to a given array
		Variant temp(*this);
		this->setarray(v.size());
		vector<Variant>::iterator iterA(*this->cValue->cArray);
		vector<Variant>::iterator iterV(*v.cValue->cArray);
		for(; iterA && iterV; ++iterA, ++iterV)
		{	
			*iterA  = temp;
			*iterA += *iterV;
		}
	}
	////////////////////////////
	// simple types at the end
	else if( isString() || v.isString() )
	{	// add as strings
		convert2string();
		*cValue->cString += v.getString();
	}
	else if( isFloat() || v.isFloat() )
	{
		convert2float();
		cValue->cFloat += v.getFloat();
	}
	else
	{
		convert2int();
		cValue->cInteger += v.getInt();
	}
	return *this;
}

const Variant& Variant::operator-=(const Variant& v)
{
	if( !isValid() || !v.isValid() )
	{	// invalid inputs, invalid output
		clear();
	}
	////////////////////////////
	// simple types at the end
	else if( isArray() && v.isArray() )
	{	// add two arrays
		// don't check for dimension mismatches, just pad with zeros
		vector<Variant>::iterator iterA(*this->cValue->cArray);
		vector<Variant>::iterator iterV(*v.cValue->cArray);
		for(; iterA && iterV; ++iterA, ++iterV)
			*iterA -= *iterV;
	}
	else if( isArray() && !v.isArray() )
	{	// add a given skalar recursively to a local array
		vector<Variant>::iterator iterA(*this->cValue->cArray);
		for(; iterA; ++iterA)
			*iterA -= v;
	}
	else if( !isArray() && v.isArray() )
	{	// add a local skalar to a given array
		Variant temp(*this);
		this->setarray(v.size());
		vector<Variant>::iterator iterA(*this->cValue->cArray);
		vector<Variant>::iterator iterV(*v.cValue->cArray);
		for(; iterA && iterV; ++iterA, ++iterV)
		{	
			*iterA  = temp;
			*iterA -= *iterV;
		}
	}
	////////////////////////////
	// simple types at the end
	//else if( isString() || v.isString() )
	//{	// '-' not defined for strings
	//}
	else if( isFloat() || v.isFloat() )
	{
		convert2float();
		cValue->cFloat -= v.getFloat();
	}
	else
	{
		convert2int();
		cValue->cInteger -= v.getInt();
	}
	return *this;
}

const Variant& Variant::operator*=(const Variant& v)
{
	if( !isValid() || !v.isValid() )
	{	// invalid inputs, invalid output
		clear();
	}
	////////////////////////////
	// simple types at the end
	else if( isArray() && v.isArray() )
	{	// add two arrays
		// don't check for dimension mismatches, just pad with zeros
		vector<Variant>::iterator iterA(*this->cValue->cArray);
		vector<Variant>::iterator iterV(*v.cValue->cArray);
		for(; iterA && iterV; ++iterA, ++iterV)
			*iterA *= *iterV;
		// for the rest of the elements, multiply with 0
		for(; iterA ; ++iterA)
			*iterA *= 0;
	}
	else if( isArray() && !v.isArray() )
	{	// add a given skalar recursively to a local array
		vector<Variant>::iterator iterA(*this->cValue->cArray);
		for(; iterA; ++iterA)
			*iterA *= v;
	}
	else if( !isArray() && v.isArray() )
	{	// add a local skalar to a given array
		Variant temp(*this);
		this->setarray(v.size());
		vector<Variant>::iterator iterA(*this->cValue->cArray);
		vector<Variant>::iterator iterV(*v.cValue->cArray);
		for(; iterA && iterV; ++iterA, ++iterV)
		{	
			*iterA  = temp;
			*iterA *= *iterV;
		}
	}
	////////////////////////////
	// simple types at the end
	//else if( isString() || v.isString() )
	//{	// '-' not defined for strings
	//}
	else if( isFloat() || v.isFloat() )
	{
		convert2float();
		cValue->cFloat *= v.getFloat();
	}
	else
	{
		convert2int();
		cValue->cInteger *= v.getInt();
	}
	return *this;
}

const Variant& Variant::operator/=(const Variant& v)
{
	if( !isValid() || !v.isValid() )
	{	// invalid inputs, invalid output
		clear();
	}
	////////////////////////////
	// simple types at the end
	else if( isArray() && v.isArray() )
	{	// add two arrays
		// don't check for dimension mismatches, just pad with zeros
		vector<Variant>::iterator iterA(*this->cValue->cArray);
		vector<Variant>::iterator iterV(*v.cValue->cArray);
		for(; iterA && iterV; ++iterA, ++iterV)
			*iterA /= *iterV;
		// for the rest of the elements, divide by 0 -> invalid
		for(; iterA; ++iterA)
			iterA->clear();
	}
	else if( isArray() && !v.isArray() )
	{	// add a given skalar recursively to a local array
		vector<Variant>::iterator iterA(*this->cValue->cArray);
		for(; iterA; ++iterA)
			*iterA /= v;
	}
	else if( !isArray() && v.isArray() )
	{	// add a local skalar to a given array
		Variant temp(*this);
		this->setarray(v.size());
		vector<Variant>::iterator iterA(*this->cValue->cArray);
		vector<Variant>::iterator iterV(*v.cValue->cArray);
		for(; iterA && iterV; ++iterA, ++iterV)
		{	
			*iterA  = temp;
			*iterA /= *iterV;
		}
	}
	////////////////////////////
	// simple types at the end
	//else if( isString() || v.isString() )
	//{	// '-' not defined for strings
	//}
	else if( isFloat() || v.isFloat() )
	{
		
		if( 0==v.getFloat() )
			clear();
		else
		{
			convert2float();
			cValue->cFloat /= v.getFloat();
		}
	}
	else
	{
		if( 0==v.getInt() )
			clear();
		else
		{
			convert2int();
			cValue->cInteger /= v.getInt();
		}
	}
	return *this;
}


///////////////////////////////////////////////////////////////////////////
// Define postfix increment operator.
Variant Variant::operator++(int)
{
	Variant temp(*this);
	if( isValid() )
	{
		if( isArray() )
		{	
			vector<Variant>::iterator iterA(*this->cValue->cArray);
			for(; iterA; ++iterA)
				++(*iterA);
		}
		else 
		{	
			convert2number();
			*this+=1;
		}
	}
	return temp;
}
Variant Variant::operator--(int)
{
	Variant temp(*this);
	if( isValid() )
	{
		if( isArray() )
		{	
			vector<Variant>::iterator iterA(*this->cValue->cArray);
			for(; iterA; ++iterA)
				--(*iterA);
		}
		else 
		{	
			convert2number();
			*this-=1;
		}
	}
	return temp;
}
///////////////////////////////////////////////////////////////////////////
// Define prefix increment operator.
Variant& Variant::operator++() 
{
	if( isValid() )
	{
		if( isArray() )
		{	
			vector<Variant>::iterator iterA(*this->cValue->cArray);
			for(; iterA; ++iterA)
				++(*iterA);
		}
		else 
		{	
			convert2number();
			*this+=1;
		}
	}
	return *this;
}
Variant& Variant::operator--() 
{
	if( isValid() )
	{
		if( isArray() )
		{	
			vector<Variant>::iterator iterA(*this->cValue->cArray);
			for(; iterA; ++iterA)
				--(*iterA);
		}
		else 
		{	
			convert2number();
			*this-=1;
		}
	}
	return *this;
}

///////////////////////////////////////////////////////////////////////////
// binary/logic operations
const Variant& Variant::operator&=(const Variant& v)
{
	if( !isValid() || !v.isValid() )
	{	// invalid inputs, invalid output
		clear();
	}
	////////////////////////////
	// simple types at the end
	else if( isArray() && v.isArray() )
	{	// add two arrays
		// don't check for dimension mismatches, just pad with zeros
		vector<Variant>::iterator iterA(*this->cValue->cArray);
		vector<Variant>::iterator iterV(*v.cValue->cArray);
		for(; iterA && iterV; ++iterA, ++iterV)
			*iterA &= *iterV;
		// for the rest of the elements, use 0
		for(; iterA; ++iterA)
			*iterA = 0;
	}
	else if( isArray() && !v.isArray() )
	{	// add a given skalar recursively to a local array
		vector<Variant>::iterator iterA(*this->cValue->cArray);
		for(; iterA; ++iterA)
			*iterA &= v;
	}
	else if( !isArray() && v.isArray() )
	{	// add a local skalar to a given array
		Variant temp(*this);
		this->setarray(v.size());
		vector<Variant>::iterator iterA(*this->cValue->cArray);
		vector<Variant>::iterator iterV(*v.cValue->cArray);
		for(; iterA && iterV; ++iterA, ++iterV)
		{	
			*iterA  = temp;
			*iterA &= *iterV;
		}
	}
	////////////////////////////
	// simple types at the end
	else if( isString() || v.isString() )
	{	// make it append two strings
		convert2string();
		string<>* pstr = cValue->cString;
		*pstr += v.getString();
	}
	else
	{
		convert2int();
		cValue->cInteger &= v.getInt();
	}
	return *this;
}

const Variant& Variant::operator|=(const Variant& v)
{
	if( !isValid() || !v.isValid() )
	{	// invalid inputs, invalid output
		clear();
	}
	////////////////////////////
	// simple types at the end
	else if( isArray() && v.isArray() )
	{	// add two arrays
		// don't check for dimension mismatches, just pad with zeros
		vector<Variant>::iterator iterA(*this->cValue->cArray);
		vector<Variant>::iterator iterV(*v.cValue->cArray);
		for(; iterA && iterV; ++iterA, ++iterV)
			*iterA |= *iterV;
		// for the rest of the elements, do nothing
	}
	else if( isArray() && !v.isArray() )
	{	// add a given skalar recursively to a local array
		vector<Variant>::iterator iterA(*this->cValue->cArray);
		for(; iterA; ++iterA)
			*iterA |= v;
	}
	else if( !isArray() && v.isArray() )
	{	// add a local skalar to a given array
		Variant temp(*this);
		this->setarray(v.size());
		vector<Variant>::iterator iterA(*this->cValue->cArray);
		vector<Variant>::iterator iterV(*v.cValue->cArray);
		for(; iterA && iterV; ++iterA, ++iterV)
		{	
			*iterA  = temp;
			*iterA |= *iterV;
		}
	}
	////////////////////////////
	// simple types at the end
	//else if( isString() || v.isString() )
	//{	// not defined for strings
	//}
	else
	{
		convert2int();
		cValue->cInteger |= v.getInt();
	}
	return *this;
}

const Variant& Variant::operator^=(const Variant& v)
{
	if( !isValid() || !v.isValid() )
	{	// invalid inputs, invalid output
		clear();
	}
	////////////////////////////
	// simple types at the end
	else if( isArray() && v.isArray() )
	{	// add two arrays
		// don't check for dimension mismatches, just pad with zeros
		vector<Variant>::iterator iterA(*this->cValue->cArray);
		vector<Variant>::iterator iterV(*v.cValue->cArray);
		for(; iterA && iterV; ++iterA, ++iterV)
			*iterA ^= *iterV;
		// for the rest of the elements, do nothing
	}
	else if( isArray() && !v.isArray() )
	{	// add a given skalar recursively to a local array
		vector<Variant>::iterator iterA(*this->cValue->cArray);
		for(; iterA; ++iterA)
			*iterA ^= v;
	}
	else if( !isArray() && v.isArray() )
	{	// add a local skalar to a given array
		Variant temp(*this);
		this->setarray(v.size());
		vector<Variant>::iterator iterA(*this->cValue->cArray);
		vector<Variant>::iterator iterV(*v.cValue->cArray);
		for(; iterA && iterV; ++iterA, ++iterV)
		{	
			*iterA  = temp;
			*iterA ^= *iterV;
		}
	}
	////////////////////////////
	// simple types at the end
	//else if( isString() || v.isString() )
	//{	// not defined for strings
	//}
	else
	{
		convert2int();
		cValue->cInteger ^= v.getInt();
	}
	return *this;
}

const Variant& Variant::operator>>=(const Variant& v)
{
	if( !isValid() || !v.isValid() )
	{	// invalid inputs, invalid output
		clear();
	}
	////////////////////////////
	// simple types at the end
	else if( isArray() && v.isArray() )
	{	// add two arrays
		// don't check for dimension mismatches, just pad with zeros
		vector<Variant>::iterator iterA(*this->cValue->cArray);
		vector<Variant>::iterator iterV(*v.cValue->cArray);
		for(; iterA && iterV; ++iterA, ++iterV)
			*iterA >>= *iterV;
		// for the rest of the elements, do nothing
	}
	else if( isArray() && !v.isArray() )
	{	// add a given skalar recursively to a local array
		vector<Variant>::iterator iterA(*this->cValue->cArray);
		for(; iterA; ++iterA)
			*iterA >>= v;
	}
	else if( !isArray() && v.isArray() )
	{	// add a local skalar to a given array
		Variant temp(*this);
		this->setarray(v.size());
		vector<Variant>::iterator iterA(*this->cValue->cArray);
		vector<Variant>::iterator iterV(*v.cValue->cArray);
		for(; iterA && iterV; ++iterA, ++iterV)
		{	
			*iterA  = temp;
			*iterA >>= *iterV;
		}
	}
	else
	{
		convert2int();
		cValue->cInteger >>= v.getInt();
	}
	return *this;
}

const Variant& Variant::operator<<=(const Variant& v)
{
	if( !isValid() || !v.isValid() )
	{	// invalid inputs, invalid output
		clear();
	}
	////////////////////////////
	// simple types at the end
	else if( isArray() && v.isArray() )
	{	// add two arrays
		// don't check for dimension mismatches, just pad with zeros
		vector<Variant>::iterator iterA(*this->cValue->cArray);
		vector<Variant>::iterator iterV(*v.cValue->cArray);
		for(; iterA && iterV; ++iterA, ++iterV)
			*iterA <<= *iterV;
		// for the rest of the elements, do nothing
	}
	else if( isArray() && !v.isArray() )
	{	// add a given skalar recursively to a local array
		vector<Variant>::iterator iterA(*this->cValue->cArray);
		for(; iterA; ++iterA)
			*iterA <<= v;
	}
	else if( !isArray() && v.isArray() )
	{	// add a local skalar to a given array
		Variant temp(*this);
		this->setarray(v.size());
		vector<Variant>::iterator iterA(*this->cValue->cArray);
		vector<Variant>::iterator iterV(*v.cValue->cArray);
		for(; iterA && iterV; ++iterA, ++iterV)
		{	
			*iterA  = temp;
			*iterA <<= *iterV;
		}
	}
	else
	{
		convert2int();
		cValue->cInteger <<= v.getInt();
	}
	return *this;
}


///////////////////////////////////////////////////////////////////////////
// compare operations
int compare(const Variant& va, const Variant& vb)
{
	if( va.cValue.pointer() == vb.cValue.pointer() )
	{	// identic pointers
		return 0;
	}
	else if( !va.isValid() && !vb.isValid() )
	{	// both invalid
		return 0;
	}
	else if( !va.isValid() || !vb.isValid() )
	{	// only one invalud
		return (!va.isValid()) ? -1 : 1;
	}
	////////////////////////////
	// simple types at the end
	else if( va.isArray() && vb.isArray() )
	{	// two arrays
		int val = va.size() - vb.size();
		if(val)
		{
			vector<Variant>::iterator iterA(*va.cValue->cArray);
			vector<Variant>::iterator iterV(*vb.cValue->cArray);
			for(; !val && iterA && iterV; ++iterA, ++iterV)
				val = compare(*iterA, *iterV);
		}
		return val;
	}
	else if( va.isArray() && !vb.isArray() )
	{	
		return 1;
	}
	else if( !va.isArray() && vb.isArray() )
	{	
		return -1;
	}
	else if( va.isString() || vb.isString() )
	{	
		return va.getString().compareTo( vb.getString() );
	}
	else if( va.isFloat() || vb.isFloat() )
	{
		double v = va.getFloat() - vb.getFloat();
		return (v<0) ? -1 : (v>0) ? 1 : 0;
	}
	else if( va.isInt() || vb.isInt() )
	{
		return va.getInt() - vb.getInt();
	}
	return 0;
}








void test_variant()
{
#if defined(DEBUG)

	{
		/////////////////////////
		Variant a,b,c;
		a = 8.6;

		Variant ref(a,true);
		ref += 2;


		printf("%s (10.6)\n", (const char*)a.getString());
		printf("%s (10.6)\n", (const char*)ref.getString());

		b = "hallo";

		ref = a+b;

		printf("%s (10.6hallo)\n", (const char*)a.getString());
		printf("%s (10.6hallo)\n", (const char*)ref.getString());

		ref++;
		printf("%s (11.6)\n", (const char*)a.getString());
		printf("%s (11.6)\n", (const char*)ref.getString());

		b.setarray(3);
		b[1] = 1;
		b[2] = 2.2;

		c = a;
		c = b;
		printf("%s (hallo)\n", (const char*)c[0].getString());
		printf("%s (1)\n", (const char*)c[1].getString());
		printf("%s (2.2)\n\n", (const char*)c[2].getString());
		c = a+b;
		printf("%s (11.6hallo)\n", (const char*)c[0].getString());
		printf("%s (12.6)\n", (const char*)c[1].getString());
		printf("%s (13.8)\n\n", (const char*)c[2].getString());
		b = c++;
		printf("%s (12.6)\n", (const char*)c[0].getString());
		printf("%s (13.6)\n", (const char*)c[1].getString());
		printf("%s (14.8)\n\n", (const char*)c[2].getString());

		printf("%s (11.6hallo)\n", (const char*)b[0].getString());
		printf("%s (12.6)\n", (const char*)b[1].getString());
		printf("%s (13.8)\n\n", (const char*)b[2].getString());

		c[1] += c[0];
		c[2] =  4;

		printf("%s (12.6)\n", (const char*)c[0].getString());
		printf("%s (26.2)\n", (const char*)c[1].getString());
		printf("%s (4)\n\n", (const char*)c[2].getString());
		c += a;
		printf("%s (24.2)\n", (const char*)c[0].getString());
		printf("%s (37.8)\n", (const char*)c[1].getString());
		printf("%s (15.6)\n\n", (const char*)c[2].getString());
		
		c[2] /= a;			printf("%s (1.344)\n", (const char*)c[2].getString());
		c[2] &= a;			printf("%s (0)\n", (const char*)c[2].getString());
		c[2] |= a;			printf("%s (12)\n", (const char*)c[2].getString());
		c[2] ^= a;			printf("%s (0)\n", (const char*)c[2].getString());
		c[2] = a && c[2];	printf("%s (0)\n", (const char*)c[2].getString());
		c[2] = a || c[2];	printf("%s (1)\n\n", (const char*)c[2].getString());

		printf("%s (24.2)\n", (const char*)c[0].getString());
		printf("%s (37.8)\n", (const char*)c[1].getString());
		printf("%s (1)\n\n", (const char*)c[2].getString());
	}

	Variant a;
	

#endif//DEBUG
}



///////////////////////////////////////////////////////////////////////////////
NAMESPACE_END(basics)
///////////////////////////////////////////////////////////////////////////////
