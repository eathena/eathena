#include "eacompiler.h"


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class Variant
{
	typedef enum _value_type
	{
		VAR_NONE = 0,
		VAR_INTEGER,
		VAR_STRING,
		VAR_FLOAT,
		VAR_ARRAY,
	} var_t;

	class _value
	{
		friend class Variant;
		var_t			cType;
		union
		{	// integer
			int64		cInteger;
			// double
			double		cFloat;
			// string (actually a pointer to a string<> object since a class cannot be a union member)
			void*		cString;
			// array
			Variant*	cArray;
		};
		size_t			cSize;
	public:
		///////////////////////////////////////////////////////////////////////
		// default constructor/destructor
		_value() :cType(VAR_NONE), cInteger(0), cSize(1)
		{}
		~_value()
		{
			clear();
		}
		///////////////////////////////////////////////////////////////////////
		// type constructors
		_value(const _value &v) :cType(VAR_NONE), cInteger(0), cSize(1)
		{	assign(v);
		}
		_value(const int val) :cType(VAR_INTEGER), cInteger(val), cSize(1)
		{}
		_value(const double val) :cType(VAR_FLOAT), cFloat(val), cSize(1)
		{}
		_value(const char* val) :cType(VAR_STRING), cString(new string<>(val)), cSize(1)
		{}
		_value(const string<>& val) :cType(VAR_STRING), cString(new string<>(val)), cSize(1)
		{}

		///////////////////////////////////////////////////////////////////////
		//
		const _value& operator=(const _value &v)
		{
			assign(v);
			return *this; 
		}
		const _value& operator=(const int val)
		{
			assign(val);
			return *this; 
		}
		const _value& operator=(const double val)
		{
			assign(val);
			return *this; 
		}
		const _value& operator=(const char* val)
		{
			assign(val);
			return *this; 
		}
		const _value& operator=(const string<>& val)
		{
			assign(val);
			return *this; 
		}
		///////////////////////////////////////////////////////////////////////
		//
		void assign(const _value &v)
		{
			clear();
			switch(v.cType)
			{
			case VAR_INTEGER:
				cType = VAR_INTEGER;
				cSize=1;
				cInteger = v.cInteger; 
				break;
			case VAR_FLOAT:
				cType = VAR_FLOAT;
				cSize=1;
				cFloat = v.cFloat; 
				break;
			case VAR_STRING:
				cType = VAR_STRING;
				cSize=1;
				cString = (void*)(new string<>( *((string<>*)v.cString)) );
				break;
			case VAR_ARRAY:
			{
				size_t i;
				cType = VAR_ARRAY;
				cSize=v.cSize;
				cArray = new Variant[cSize];
				for(i=0; i<cSize; ++i)
					cArray[i] = v.cArray[i];
				break;
			}
			case VAR_NONE: // already initialized for this
			default:
				break;
			}
		}
		void assign(const int val)
		{
			clear();
			cType = VAR_INTEGER;
			cSize=1;
			cInteger = val; 
		}
		void assign(const double val)
		{
			clear();
			cType = VAR_FLOAT;
			cSize=1;
			cFloat = val;
		}
		void assign(const char* val)
		{
			clear();
			cType = VAR_STRING;
			cSize=1;
			cString = (void*)(new string<>(val));
		}
		void assign(const string<>& val)
		{
			clear();
			cType = VAR_STRING;
			cSize=1;
			cString = (void*)(new string<>(val));
		}

		///////////////////////////////////////////////////////////////////////
		//
		void clear()
		{	
			switch(cType)
			{
			case VAR_ARRAY:
				if(cArray) delete [] cArray;
				break;
			case VAR_STRING:
				if(cString) delete ((string<>*)cString);
				break;
			case VAR_INTEGER:
			case VAR_FLOAT:
			default:
				break;
			}
			cType = VAR_NONE;
			cInteger = 0;
			cSize = 0;
		}
		///////////////////////////////////////////////////////////////////////
		bool isValid() const	{ return cType != VAR_NONE; }	
		bool isInt() const		{ return cType == VAR_INTEGER; }	
		bool isFloat() const	{ return cType == VAR_FLOAT; }	
		bool isString() const	{ return cType == VAR_STRING; }	
		bool isArray() const	{ return cType == VAR_ARRAY; }
		size_t getSize() const	{ return (cType== VAR_ARRAY) ? cSize : 1; }
		///////////////////////////////////////////////////////////////////////
		void setarray(size_t cnt)
		{	// generate an array from this node and set cArray[0] with current content
			Variant *temp = new Variant[cnt];
			// copy stuff into cArray[0]
			temp[0].cValue->cType = this->cType;
			temp[0].cValue->cSize = this->cSize;
			switch(this->cType)
			{
			case VAR_INTEGER:
				temp[0].cValue->cInteger = this->cInteger; 
				break;
			case VAR_FLOAT:
				temp[0].cValue->cFloat = this->cFloat; 
				break;
			case VAR_STRING:
				temp[0].cValue->cString = this->cString;
				break;
			case VAR_ARRAY:
				temp[0].cValue->cArray = this->cArray;
				break;
			case VAR_NONE: // already initialized for this
			default:
				break;
			}
			// make this an array
			this->cType = VAR_ARRAY;
			this->cArray= temp;
			this->cSize = cnt;
		}
		///////////////////////////////////////////////////////////////////////
		void addarray(size_t cnt)
		{	
			if( this->cType == VAR_ARRAY )
			{	// go into elements
				size_t i;
				for(i=0; i<cSize; ++i)
					cArray[i].addarray(cnt);
			}
			else
			{	// make this node an array
				setarray(cnt);
			}
		}

		friend int compare(const Variant& va, const Variant& vb);
	};

	friend class Variant::_value;

	TPtrCommon< _value > cValue;

public:
	///////////////////////////////////////////////////////////////////////////
	// Construction/Destruction
	Variant()	{}
	~Variant()	{}
	///////////////////////////////////////////////////////////////////////////
	// Copy/Assignment
	Variant(const Variant &v, bool ref=false)	{ assign(v, ref); }
	Variant(int val)							{ assign(val); }
	Variant(double val)							{ assign(val); }
	Variant(const char* val)					{ assign(val); }
	const Variant& operator=(const Variant &v)	{ assign(v);   return *this; }
	const Variant& operator=(const int val)		{ assign(val); return *this; }
	const Variant& operator=(const double val)	{ assign(val); return *this; }
	const Variant& operator=(const char* val)	{ assign(val); return *this; }

	///////////////////////////////////////////////////////////////////////
	// type of the variant
	bool isReference() const	{ return cValue.isReference(); }
	void makeValue() const		{ cValue.setaccess(AUTOREF); }		// enable copy-on-write
	void makeVariable() const	{ cValue.setaccess(AUTOCOUNT); }	// disable copy-on-write

	///////////////////////////////////////////////////////////////////////////
	// Type Initialize Variant (aka copy)
	void assign(const Variant& v, bool ref=false)
	{
		if( cValue.isReference() )
			cValue = v.cValue;			// just share the pointer
		else
			*(cValue) = *(v.cValue);	// copy the content
		if(ref) cValue.setaccess(AUTOCOUNT); // disable copy-on-write for references
	}
	///////////////////////////////////////////////////////////////////////////
	// Type Initialize Integer
	void assign(int val)
	{
		cValue->assign(val);
	}
	///////////////////////////////////////////////////////////////////////////
	// Type Initialize double
	void assign(double val)
	{
		cValue->assign(val);
	}
	///////////////////////////////////////////////////////////////////////////
	// Type Initialize String
	void assign(const char* val)
	{
		cValue->assign(val);
	}
	///////////////////////////////////////////////////////////////////////////
	// Create Array from actual element
	void setarray(size_t cnt)
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
	void addarray(size_t cnt)
	{	
		if(cnt>0)
		{
			cValue->addarray(cnt);
		}
		else
			clear();
	}

	///////////////////////////////////////////////////////////////////////////
	// clear this
	void clear()
	{
		cValue->clear();
	}


	///////////////////////////////////////////////////////////////////////////
	// Access to elements
	bool isValid() const	{ return cValue->isValid(); }
	bool isInt() const		{ return cValue->isInt(); }
	bool isFloat() const	{ return cValue->isFloat(); }
	bool isString() const	{ return cValue->isString(); }
	bool isArray() const	{ return cValue->isArray(); }
	size_t getSize() const	{ return cValue->getSize(); }

	///////////////////////////////////////////////////////////////////////////
	// Access to array elements
	const Variant& operator[](size_t inx) const
	{	// on Arrays return the element, out of bounds return element 0
		// other array accesses return this object
		return (cValue->isArray()) ? ((inx<cValue->getSize()) ? cValue->cArray[inx] : cValue->cArray[0]) : *this;
		return *this;
	}
	Variant& operator[](size_t inx)
	{	// on Arrays return the element, out of bounds return element 0
		// other array accesses return this object
		return (cValue->isArray()) ? ((inx<cValue->getSize()) ? cValue->cArray[inx] : cValue->cArray[0]) : *this;
		return *this;
	}

	///////////////////////////////////////////////////////////////////////////
	// local conversions
	void convert2string()
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
				assign( cValue->cArray[0].getString() );
				break;
			}
		}
	}
	void convert2float()
	{
		if(cValue->cType!=VAR_FLOAT)
		{
			assign( this->getFloat() );
		}
	}
	void convert2int()
	{
		if(cValue->cType!=VAR_INTEGER)
		{
			assign( this->getInt() );
		}
	}
	void convert2number()
	{
		if(cValue->cType!=VAR_INTEGER && cValue->cType!=VAR_FLOAT)
		{
			if( this->getFloat() != floor(this->getFloat()) )
				assign( this->getFloat() );
			else
				assign( this->getInt() );
		}
	}
	void cast(int type)
	{	//!! using parser opcodes here
		switch(type)
		{
		case PT_INT:
			convert2int();
		case PT_STRING:
			convert2string();
		case PT_DOUBLE:
			convert2float();
		}
	}
	///////////////////////////////////////////////////////////////////////////
	// access conversion 
	string<> getString() const
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
			return *((string<>*)(cValue->cString));
		case VAR_ARRAY:
			return cValue->cArray[0].getString();
		}	
		return string<>("");
	}
	double getFloat() const
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
			return atof(*((string<>*)(cValue->cString)));
		case VAR_ARRAY:
			return cValue->cArray[0].getFloat();
		}	
		return 0.0;
	}
	int getInt() const
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
			return atoi(*((string<>*)(cValue->cString)));
		case VAR_ARRAY:
			return cValue->cArray[0].getInt();
		}	
		return 0;
	}

	///////////////////////////////////////////////////////////////////////////
	// unary operations
	const Variant operator-() const
	{
		if( isValid() )
		{
			Variant temp(*this);
			if( temp.isArray() )
			{	// go through the array
				size_t i;
				for(i=0; i<temp.cValue->cSize; ++i)
					temp.cValue->cArray[i] = -(temp.cValue->cArray[i]);
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
	const Variant operator~() const
	{
		if( isValid() )
		{
			Variant temp(*this);
			if( temp.isArray() )
			{	// go through the array
				size_t i;
				for(i=0; i<temp.cValue->cSize; ++i)
					temp.cValue->cArray[i] = ~(temp.cValue->cArray[i]);
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
	const Variant operator!() const
	{
		if( isValid() )
		{
			Variant temp(*this);
			if( temp.isArray() )
			{	// go through the array
				size_t i;
				for(i=0; i<temp.cValue->cSize; ++i)
					temp.cValue->cArray[i] = !(temp.cValue->cArray[i]);
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
	const Variant& operator+=(const Variant& v)
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
			size_t i, val = min(cValue->cSize, v.cValue->cSize);
			for(i=0; i<val; ++i)
				cValue->cArray[i] += v.cValue->cArray[i];

		}
		else if( isArray() && !v.isArray() )
		{	// add a given skalar recursively to a local array
			size_t i;
			for(i=0; i<cValue->cSize; ++i)
				cValue->cArray[i] += v;
		}
		else if( !isArray() && v.isArray() )
		{	// add a local skalar to a given array
			size_t i;
			Variant temp(*this);
			this->setarray(v.cValue->cSize);
			for(i=0; i<v.cValue->cSize; ++i)
			{	
				this->cValue->cArray[i]  = temp;
				this->cValue->cArray[i] += v.cValue->cArray[i];
			}
		}
		////////////////////////////
		// simple types at the end
		else if( isString() || v.isString() )
		{	// add as strings
			convert2string();
			string<>* pstr = (string<>*)(cValue->cString);
			*pstr += v.getString();
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
	friend Variant operator+(const Variant& va, const Variant& vb)
	{
		Variant temp(va);
		temp += vb;
		return temp;
	}

	const Variant& operator-=(const Variant& v)
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
			size_t i, val = min(cValue->cSize, v.cValue->cSize);
			for(i=0; i<val; ++i)
				cValue->cArray[i] -= v.cValue->cArray[i];

		}
		else if( isArray() && !v.isArray() )
		{	// add a given skalar recursively to a local array
			size_t i;
			for(i=0; i<cValue->cSize; ++i)
				cValue->cArray[i] -= v;
		}
		else if( !isArray() && v.isArray() )
		{	// add a local skalar to a given array
			size_t i;
			Variant temp(*this);
			this->setarray(v.cValue->cSize);
			for(i=0; i<v.cValue->cSize; ++i)
			{	
				this->cValue->cArray[i]  = temp;
				this->cValue->cArray[i] -= v.cValue->cArray[i];
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
	friend Variant operator-(const Variant& va, const Variant& vb)
	{
		Variant temp(va);
		temp -= vb;
		return temp;

	}

	const Variant& operator*=(const Variant& v)
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
			size_t i, val = min(cValue->cSize, v.cValue->cSize);
			for(i=0; i<val; ++i)
				cValue->cArray[i] *= v.cValue->cArray[i];
			// for the rest of the elements, multiply with 0
			for(   ; i<cValue->cSize; ++i)
				cValue->cArray[i] *= 0;
		}
		else if( isArray() && !v.isArray() )
		{	// add a given skalar recursively to a local array
			size_t i;
			for(i=0; i<cValue->cSize; ++i)
				cValue->cArray[i] *= v;
		}
		else if( !isArray() && v.isArray() )
		{	// add a local skalar to a given array
			size_t i;
			Variant temp(*this);
			this->setarray(v.cValue->cSize);
			for(i=0; i<v.cValue->cSize; ++i)
			{	
				this->cValue->cArray[i]  = temp;
				this->cValue->cArray[i] *= v.cValue->cArray[i];
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
	friend Variant operator*(const Variant& va, const Variant& vb)
	{
		Variant temp(va);
		temp *= vb;
		return temp;

	}
	const Variant& operator/=(const Variant& v)
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
			size_t i, val = min(cValue->cSize, v.cValue->cSize);
			for(i=0; i<val; ++i)
				cValue->cArray[i] /= v.cValue->cArray[i];
			// for the rest of the elements, divide by 0 -> invalid
			for(   ; i<cValue->cSize; ++i)
				cValue->cArray[i].clear();
		}
		else if( isArray() && !v.isArray() )
		{	// add a given skalar recursively to a local array
			size_t i;
			for(i=0; i<cValue->cSize; ++i)
				cValue->cArray[i] /= v;
		}
		else if( !isArray() && v.isArray() )
		{	// add a local skalar to a given array
			size_t i;
			Variant temp(*this);
			this->setarray(v.cValue->cSize);
			for(i=0; i<v.cValue->cSize; ++i)
			{	
				this->cValue->cArray[i]  = temp;
				this->cValue->cArray[i] /= v.cValue->cArray[i];
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
	friend Variant operator/(const Variant& va, const Variant& vb)
	{
		Variant temp(va);
		temp /= vb;
		return temp;
	}
	///////////////////////////////////////////////////////////////////////////
	// Define postfix increment operator.
	Variant operator++(int)
	{
		Variant temp(*this);
		if( isValid() )
		{
			if( isArray() )
			{	
				size_t i;
				for(i=0; i<cValue->cSize; ++i)
					cValue->cArray[i]++;
			}
			else 
			{	
				convert2number();
				*this+=1;
			}
		}
		return temp;
	}
	Variant operator--(int)
	{
		Variant temp(*this);
		if( isValid() )
		{
			if( isArray() )
			{	
				size_t i;
				for(i=0; i<cValue->cSize; ++i)
					--cValue->cArray[i];
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
	Variant& operator++() 
	{
		if( isValid() )
		{
			if( isArray() )
			{	
				size_t i;
				for(i=0; i<cValue->cSize; ++i)
					++cValue->cArray[i];
			}
			else 
			{	
				convert2number();
				*this+=1;
			}
		}
		return *this;
	}
	Variant& operator--() 
	{
		if( isValid() )
		{
			if( isArray() )
			{	
				size_t i;
				for(i=0; i<cValue->cSize; ++i)
					--cValue->cArray[i];
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
	const Variant& operator&=(const Variant& v)
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
			size_t i, val = min(cValue->cSize, v.cValue->cSize);
			for(i=0; i<val; ++i)
				cValue->cArray[i] &= v.cValue->cArray[i];
			// for the rest of the elements, use 0
			for(   ; i<cValue->cSize; ++i)
				cValue->cArray[i] = 0;
		}
		else if( isArray() && !v.isArray() )
		{	// add a given skalar recursively to a local array
			size_t i;
			for(i=0; i<cValue->cSize; ++i)
				cValue->cArray[i] &= v;
		}
		else if( !isArray() && v.isArray() )
		{	// add a local skalar to a given array
			size_t i;
			Variant temp(*this);
			this->setarray(v.cValue->cSize);
			for(i=0; i<v.cValue->cSize; ++i)
			{	
				this->cValue->cArray[i]  = temp;
				this->cValue->cArray[i] &= v.cValue->cArray[i];
			}
		}
		////////////////////////////
		// simple types at the end
		else if( isString() || v.isString() )
		{	// make it append two strings
			convert2string();
			string<>* pstr = (string<>*)(cValue->cString);
			*pstr += v.getString();
		}
		else
		{
			convert2int();
			cValue->cInteger &= v.getInt();
		}
		return *this;
	}
	friend Variant operator&(const Variant& va, const Variant& vb)
	{
		Variant temp(va);
		temp &= vb;
		return temp;
	}
	const Variant& operator|=(const Variant& v)
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
			size_t i, val = min(cValue->cSize, v.cValue->cSize);
			for(i=0; i<val; ++i)
				cValue->cArray[i] |= v.cValue->cArray[i];
			// for the rest of the elements, do nothing
		}
		else if( isArray() && !v.isArray() )
		{	// add a given skalar recursively to a local array
			size_t i;
			for(i=0; i<cValue->cSize; ++i)
				cValue->cArray[i] |= v;
		}
		else if( !isArray() && v.isArray() )
		{	// add a local skalar to a given array
			size_t i;
			Variant temp(*this);
			this->setarray(v.cValue->cSize);
			for(i=0; i<v.cValue->cSize; ++i)
			{	
				this->cValue->cArray[i]  = temp;
				this->cValue->cArray[i] |= v.cValue->cArray[i];
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
	friend Variant operator|(const Variant& va, const Variant& vb)
	{
		Variant temp(va);
		temp |= vb;
		return temp;
	}
	const Variant& operator^=(const Variant& v)
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
			size_t i, val = min(cValue->cSize, v.cValue->cSize);
			for(i=0; i<val; ++i)
				cValue->cArray[i] ^= v.cValue->cArray[i];
			// for the rest of the elements, do nothing
		}
		else if( isArray() && !v.isArray() )
		{	// add a given skalar recursively to a local array
			size_t i;
			for(i=0; i<cValue->cSize; ++i)
				cValue->cArray[i] ^= v;
		}
		else if( !isArray() && v.isArray() )
		{	// add a local skalar to a given array
			size_t i;
			Variant temp(*this);
			this->setarray(v.cValue->cSize);
			for(i=0; i<v.cValue->cSize; ++i)
			{	
				this->cValue->cArray[i]  = temp;
				this->cValue->cArray[i] ^= v.cValue->cArray[i];
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
	friend Variant operator^(const Variant& va, const Variant& vb)
	{
		Variant temp(va);
		temp ^= vb;
		return temp;
	}
	const Variant& operator>>=(const Variant& v)
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
			size_t i, val = min(cValue->cSize, v.cValue->cSize);
			for(i=0; i<val; ++i)
				cValue->cArray[i] >>= v.cValue->cArray[i];
			// for the rest of the elements, do nothing
		}
		else if( isArray() && !v.isArray() )
		{	// add a given skalar recursively to a local array
			size_t i;
			for(i=0; i<cValue->cSize; ++i)
				cValue->cArray[i] >>= v;
		}
		else if( !isArray() && v.isArray() )
		{	// add a local skalar to a given array
			size_t i;
			Variant temp(*this);
			this->setarray(v.cValue->cSize);
			for(i=0; i<v.cValue->cSize; ++i)
			{	
				this->cValue->cArray[i]  = temp;
				this->cValue->cArray[i] >>= v.cValue->cArray[i];
			}
		}
		else
		{
			convert2int();
			cValue->cInteger >>= v.getInt();
		}
		return *this;
	}
	friend Variant operator>>(const Variant& va, const Variant& vb)
	{
		Variant temp(va);
		temp >>= vb;
		return temp;
	}
	const Variant& operator<<=(const Variant& v)
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
			size_t i, val = min(cValue->cSize, v.cValue->cSize);
			for(i=0; i<val; ++i)
				cValue->cArray[i] <<= v.cValue->cArray[i];
			// for the rest of the elements, do nothing
		}
		else if( isArray() && !v.isArray() )
		{	// add a given skalar recursively to a local array
			size_t i;
			for(i=0; i<cValue->cSize; ++i)
				cValue->cArray[i] <<= v;
		}
		else if( !isArray() && v.isArray() )
		{	// add a local skalar to a given array
			size_t i;
			Variant temp(*this);
			this->setarray(v.cValue->cSize);
			for(i=0; i<v.cValue->cSize; ++i)
			{	
				this->cValue->cArray[i]  = temp;
				this->cValue->cArray[i] <<= v.cValue->cArray[i];
			}
		}
		else
		{
			convert2int();
			cValue->cInteger <<= v.getInt();
		}
		return *this;
	}
	friend Variant operator<<(const Variant& va, const Variant& vb)
	{
		Variant temp(va);
		temp <<= vb;
		return temp;
	}
	friend Variant operator&&(const Variant& va, const Variant& vb)
	{
		return Variant( va.getInt() && vb.getInt() );
	}
	friend Variant operator||(const Variant& va, const Variant& vb)
	{
		return Variant( va.getInt() || vb.getInt() );
	}
	///////////////////////////////////////////////////////////////////////////
	// compare operations
	friend int compare(const Variant& va, const Variant& vb)
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
			if( va.cValue->cSize != vb.cValue->cSize )
				return va.cValue->cSize - vb.cValue->cSize;
			else
			{
				size_t i;
				int val;
				for(i=0; i<va.cValue->cSize; ++i)
				{
					val = compare(va.cValue->cArray[i], vb.cValue->cArray[i]);
					if( val!=0 )
						return val;
				}
				return 0;
			}
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
	bool operator==(const Variant& v) const
	{
		return 0==compare(*this,v);
	}
	bool operator!=(const Variant& v) const
	{
		return 0!=compare(*this,v);
	}
	bool operator< (const Variant& v) const
	{
		return 0< compare(*this,v);
	}
	bool operator<=(const Variant& v) const
	{
		return 0<=compare(*this,v);
	}
	bool operator> (const Variant& v) const
	{
		return 0> compare(*this,v);
	}
	bool operator>=(const Variant& v) const
	{
		return 0>=compare(*this,v);
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////




class Variable : public string<>
{
	Variable(const Variable&);					// no copy
	const Variable& operator=(const Variable&);	// no assign

public:
	Variant		cValue;
	///////////////////////////////////////////////////////////////////////////
	// Construct/Destruct
	Variable(const char* n) : string<>(n)		{  }
	Variable(const string<>& n) : string<>(n)	{  }
	~Variable()	{  }

	///////////////////////////////////////////////////////////////////////////
	// PreInitialize Array
	bool setsize(size_t cnt)
	{
	//	cValue.setarray(cnt);
		return true;
	}
	///////////////////////////////////////////////////////////////////////////
	// Set Values
	void set(int val)				{ cValue = val; }
	void set(double val)			{ cValue = val; }
	void set(const char* val)		{ cValue = val; }
	void set(const Variant & val)	{ cValue = val; }

	///////////////////////////////////////////////////////////////////////////
	// read/set Value
	operator Variant()				{ return cValue; }
};




///////////////////////////////////////////////////////////////////////////////
// the user stack
///////////////////////////////////////////////////////////////////////////////
class UserStack : private noncopyable
{
	///////////////////////////////////////////////////////////////////////////
	stack<Variant>			cStack;		// the stack
	size_t					cSC;		// stack counter
	size_t					cSB;		// initial stack start index
	size_t					cParaBase;	// function parameter start index
	size_t					cTempBase;	// TempVar start index

	size_t					cPC;		// Programm Counter
	CProgramm*				cProg;

	bool process()
	{
		bool run = true;
		CProgramm::CCommand ccmd;

		while(cProg && run && cProg->getCommand(cPC, ccmd) )
		{
			switch( ccmd.cCommand )
			{
			case OP_START:
			case OP_NOP:
			{
				break;
			}
			/////////////////////////////////////////////////////////////////
			// assignment operations
			// take two stack values and push up one
			case OP_ASSIGN:	// <Op If> '='   <Op>
			{
				cSC--;
				cStack[cSC-1] = cStack[cSC];
				cStack[cSC-1].makeValue();
				break;
			}
			case OP_ADD:		// <Op AddSub> '+' <Op MultDiv>
			case OP_ASSIGN_ADD:	// <Op If> '+='  <Op>
			{
				cSC--;
				cStack[cSC-1] += cStack[cSC];
				cStack[cSC-1].makeValue();
				break;
			}
			case OP_SUB:		// <Op AddSub> '-' <Op MultDiv>
			case OP_ASSIGN_SUB:	// <Op If> '-='  <Op>
			{
				cSC--;
				cStack[cSC-1] -= cStack[cSC];
				cStack[cSC-1].makeValue();
				break;
			}
			case OP_MUL:		// <Op MultDiv> '*' <Op Unary>
			case OP_ASSIGN_MUL:	// <Op If> '*='  <Op>
			{
				cSC--;
				cStack[cSC-1] *= cStack[cSC];
				cStack[cSC-1].makeValue();
				break;
			}
			case OP_DIV:		// <Op MultDiv> '/' <Op Unary>
			case OP_ASSIGN_DIV:	// <Op If> '/='  <Op>
			{
				cSC--;
				cStack[cSC-1] /= cStack[cSC];
				cStack[cSC-1].makeValue();
				break;
			}
			case OP_MOD:		// <Op MultDiv> '%' <Op Unary>
			{
				cSC--;
				cStack[cSC-1] /= cStack[cSC];
				cStack[cSC-1].makeValue();
				break;
			}
			case OP_BIN_XOR:	// <Op BinXOR> '^' <Op BinAND>
			case OP_ASSIGN_XOR:	// <Op If> '^='  <Op>
			{
				cSC--;
				cStack[cSC-1] ^= cStack[cSC];
				cStack[cSC-1].makeValue();
				break;
			}
			case OP_BIN_AND:		// <Op BinAND> '&' <Op Equate>
			case OP_ASSIGN_AND:	// <Op If> '&='  <Op>
			{
				cSC--;
				cStack[cSC-1] &= cStack[cSC];
				cStack[cSC-1].makeValue();
				break;
			}
			case OP_BIN_OR:		// <Op BinOr> '|' <Op BinXOR>
			case OP_ASSIGN_OR:	// <Op If> '|='  <Op>
			{
				cSC--;
				cStack[cSC-1] |= cStack[cSC];
				cStack[cSC-1].makeValue();
				break;
			}
			case OP_LOG_AND:	// <Op And> '&&' <Op BinOR>
			{
				cSC--;
				cStack[cSC-1] = cStack[cSC-1] && cStack[cSC];
				break;
			}
			case OP_LOG_OR:		// <Op Or> '||' <Op And>
			{
				cSC--;
				cStack[cSC-1] = cStack[cSC-1] || cStack[cSC];
				break;
			}
			case OP_RSHIFT:		// <Op Shift> '>>' <Op AddSub>
			case OP_ASSIGN_RSH:	// <Op If> '>>='  <Op>
			{
				cSC--;
				cStack[cSC-1] = cStack[cSC-1] >> cStack[cSC];
				break;
			}
			case OP_LSHIFT:		// <Op Shift> '<<' <Op AddSub>
			case OP_ASSIGN_LSH:	// <Op If> '<<='  <Op>
			{
				cSC--;
				cStack[cSC-1] = cStack[cSC-1] << cStack[cSC];
				break;
			}
			case OP_EQUATE:		// <Op Equate> '==' <Op Compare>
			{
				cSC--;
				cStack[cSC-1] = (cStack[cSC-1]==cStack[cSC]);
				break;
			}
			case OP_UNEQUATE:	// <Op Equate> '!=' <Op Compare>
			{
				cSC--;
				cStack[cSC-1] = (cStack[cSC-1]!=cStack[cSC]);
				break;
			}
			case OP_ISGT:		// <Op Compare> '>'  <Op Shift>
			{
				cSC--;
				cStack[cSC-1] = (cStack[cSC-1]>cStack[cSC]);
				break;
			}
			case OP_ISGTEQ:		// <Op Compare> '>=' <Op Shift>
			{
				cSC--;
				cStack[cSC-1] = (cStack[cSC-1]>=cStack[cSC]);
				break;
			}
			case OP_ISLT:		// <Op Compare> '<'  <Op Shift>
			{
				cSC--;
				cStack[cSC-1] = (cStack[cSC-1]<cStack[cSC]);
				break;
			}
			case OP_ISLTEQ:		// <Op Compare> '<=' <Op Shift>
			{
				cSC--;
				cStack[cSC-1] = (cStack[cSC-1]<=cStack[cSC]);
				break;
			}
			/////////////////////////////////////////////////////////////////
			// select operation
			// take three stack values and push the second or third depending on the first
			case OP_SELECT:	// <Op Or> '?' <Op If> ':' <Op If>
			{
				cSC -= 2;
				cStack[cSC-1] = (cStack[cSC-1]!=0) ? cStack[cSC] : cStack[cSC+1];
				break;
			}

			/////////////////////////////////////////////////////////////////
			// unary operations
			// take one stack values and push a value
			case OP_NOT:	// '!'    <Op Unary>
			{
				cStack[cSC-1] = !(cStack[cSC-1]);
				break;
			}
			case OP_INVERT:	// '~'    <Op Unary>
			{
				cStack[cSC-1] = ~(cStack[cSC-1]);
				break;
			}
			case OP_NEGATE:	// '-'    <Op Unary>
			{
				cStack[cSC-1] = -(cStack[cSC-1]);
				break;
			}

			/////////////////////////////////////////////////////////////////
			// sizeof operations
			// take one stack values and push the result
								// sizeof '(' <Type> ')' // replaces with OP_PUSH_INT on compile time
			case OP_SIZEOF:		// sizeof '(' Id ')'
			{
				cStack[cSC-1].makeValue();
				cStack[cSC-1] = (int)cStack[cSC-1].getSize();
				break;
			}

			/////////////////////////////////////////////////////////////////
			// cast operations
			// take one stack values and push the result
			case OP_CAST:	// '(' <Type> ')' <Op Unary>   !CAST
			{	// <Op Unary> is first on the stack, <Type> is second
				cStack[cSC-1].cast(ccmd.cParam1);
				break;
			}

			/////////////////////////////////////////////////////////////////
			// Pre operations
			// take one stack variable and push a value
			case OP_PREADD:		// '++'   <Op Unary>
			{	
				++cStack[cSC-1];
				cStack[cSC-1].makeValue();
				break;
			}
			case OP_PRESUB:		// '--'   <Op Unary>
			{	
				--cStack[cSC-1];
				cStack[cSC-1].makeValue();
				break;
			}
			/////////////////////////////////////////////////////////////////
			// Post operations
			// take one stack variable and push a value
			case OP_POSTADD:	// <Op Pointer> '++'
			{	
				Variant temp = cStack[cSC-1];
				temp.makeValue();
				cStack[cSC-1]++;
				cStack[cSC-1].makeValue();
				cStack[cSC-1] = temp;
				break;
			}
			case OP_POSTSUB:	// <Op Pointer> '--'
			{	
				Variant temp(cStack[cSC-1]);
				temp.makeValue();
				cStack[cSC-1]--;
				cStack[cSC-1].makeValue();
				cStack[cSC-1] = temp;
				break;
			}

			/////////////////////////////////////////////////////////////////
			// Member Access
			// take a variable and a value from stack and push a varible
			case OP_MEMBER:		// <Op Pointer> '.' <Value>     ! member
			{
				printf("not implemented yet\n");

				cSC--;
				break;
			}
			/////////////////////////////////////////////////////////////////
			// Array
			// take a variable and a value from stack and push a varible
			case OP_ARRAY:		// <Op Pointer> '[' <Expr> ']'  ! array
			{
				cSC--;
				cStack[cSC-1].assign( cStack[cSC-1][ cStack[cSC].getInt() ], true);
				break;
			}
			/////////////////////////////////////////////////////////////////
			// standard function calls
			// check the values on stack before or inside the call of function
			case OP_CALLSCRIPT1:
			case OP_CALLSCRIPT2:
			case OP_CALLSCRIPT3:
			case OP_CALLSCRIPT4:
							// Id '(' <Expr> ')'
							// Id '(' ')'
							// Id <Call List> ';'
							// Id ';'
			{

				printf("not implemented yet\n");
				cSC--;
				break;
			}
			/////////////////////////////////////////////////////////////////
			// standard function calls
			// check the values on stack before or inside the call of function
			case OP_CALLBUILDIN1:
			case OP_CALLBUILDIN2:
			case OP_CALLBUILDIN3:
			case OP_CALLBUILDIN4:
							// Id '(' <Expr> ')'
							// Id '(' ')'
							// Id <Call List> ';'
							// Id ';'
			{
				printf("not implemented yet\n");
				cSC--;
				break;
			}

			/////////////////////////////////////////////////////////////////
			// conditional branch
			// take one value from stack 
			// and add 1 or the branch offset to the programm counter
			case OP_NIF:
			{
				cSC--;
				if( cStack[cSC]==0 )
					cPC = ccmd.cParam1;

				break;
			}
			case OP_IF:		// if '(' <Expr> ')' <Normal Stm>
			{
				if( cStack[cSC]!=0 )
					cPC = ccmd.cParam1;

				break;
			}
			/////////////////////////////////////////////////////////////////
			// unconditional branch
			// add the branch offset to the programm counter
			case OP_GOTO:	// goto position
			{
				cPC = ccmd.cParam1;
				break;
			}

			case OP_CLEAR:
			{
				cStack[cSC-1].clear();
				break;
			}
			case OP_RESIZE:
			{	//this->logging("array resize (%i dimension(s))", ccmd.cParam1); break;
				// there are (ccmd.cParam1) elements on stack containing the dimemsions oth the multi-array
				size_t i, dim = ccmd.cParam1;
				cSC -= dim;
				cStack[cSC-1].clear();
				for(i=0; i<dim; ++i)
				{	// number of elements in this dimension
					cStack[cSC-1].addarray( cStack[cSC+i].getInt() );
				}
				break;
			}
			case OP_VECTORIZE1:
			case OP_VECTORIZE2:
			case OP_VECTORIZE3:
			case OP_VECTORIZE4:
			{	//this->logging("vectorize '%i' elements", ccmd.cParam1); break;
				size_t i, cnt = ccmd.cParam1;
				cSC -= cnt;
				cStack[cSC].setarray( cnt );
				for(i=1; i<cnt; ++i)
				{	// put the elements into the array
					cStack[cSC][i] = cStack[cSC+i];				
				}
				// and virtually push the vectorized element
				cSC ++;
				break;
			}

			/////////////////////////////////////////////////////////////////
			// explicit stack pushes
			// Values pushed on stack directly
			// HexLiteral
			// DecLiteral
			// StringLiteral
			// CharLiteral
			// FloatLiteral
			// Id
			// <Call Arg>  ::= '-'

			case OP_PUSH_ADDR:
			case OP_PUSH_INT1:	// followed by an integer
			case OP_PUSH_INT2:
			case OP_PUSH_INT3:
			case OP_PUSH_INT4:
			{
				cStack[cSC].makeValue();
				cStack[cSC] = ccmd.cParam1;
				cSC++;
				break;
			}
			case OP_PUSH_STRING:	// followed by a string (pointer)
			{
				cStack[cSC].makeValue();
				cStack[cSC] = ccmd.cString;
				cSC++;
				break;
			}
			case OP_PUSH_FLOAT:	// followed by a float
			{
				cStack[cSC].makeValue();
				cStack[cSC] = (double)CProgramm::int2float(ccmd.cParam1);
				cSC++;
				break;
			}
			case OP_PUSH_VAR:	// followed by a string containing a variable name
			{
				cStack[cSC].makeValue();
//				cStack[cSC] = findvariable(ccmd.cString);
				cStack[cSC].makeVariable();
				cSC++;
				break;
			}
			case OP_PUSH_VALUE:
			{
				cStack[cSC].makeValue();
//				cStack[cSC] = findvariable(ccmd.cString);
				cStack[cSC].makeVariable();
				cSC++;
				break;
			}
			case OP_PUSH_PARAM:
			{
				cStack[cSC].makeValue();
				cStack[cSC] = cStack[cParaBase+ccmd.cParam1];
				cStack[cSC].makeVariable();
				cSC++;
				break;
			}
			case OP_PUSH_TEMPVAR1:
			case OP_PUSH_TEMPVAR2:
			case OP_PUSH_TEMPVAR3:
			case OP_PUSH_TEMPVAR4:
			{
				cStack[cSC].makeValue();
				cStack[cSC] = cStack[cSB+ccmd.cParam1];
				cStack[cSC].makeVariable();
				cSC++;
				break;
			}
			case OP_PUSH_TEMPVALUE1:
			case OP_PUSH_TEMPVALUE2:
			case OP_PUSH_TEMPVALUE3:
			case OP_PUSH_TEMPVALUE4:
			{
				cStack[cSC].makeValue();
				cStack[cSC] = cStack[cSB+ccmd.cParam1];
				cSC++;
				break;
			}

			case OP_POP:	// decrements the stack by one
			{	// maybe better reset the stack
				cSC--;
				break;
			}


			case VX_LABEL:
			case VX_BREAK:
			case VX_CONT:
			case VX_GOTO:
				printf("non-converted temporal opcodes\n");

			case OP_RETURN:
			case OP_END:
			default:
			{
				run=false;
				break;
			}
			}// end switch
		}// end while
		return true;
	}

public:
	///////////////////////////////////////////////////////////////////////////
	// construct/destruct
	UserStack() : cSC(0), cSB(0), cParaBase(0), cPC(0), cProg(NULL)
	{}
	~UserStack()
	{}

	///////////////////////////////////////////////////////////////////////////
	// start a programm
	bool Call(size_t programm_id)
	{		

	
		return true;
	}
	///////////////////////////////////////////////////////////////////////////
	// comming back from a callback
	void Continue(Variant retvalue)
	{


	}
};



struct aaa
{
	char str[24];
	int  val;
};

void vartest()
{

	aaa xxa = {"hallo",1};
	aaa xxb = {"xx",2};

	aaa xxc;

	xxc = xxa;
	xxc = xxb;


	/////////////////////////
	Variant a,b,c;
	a = 8.6;

	Variant ref(a,true);
	ref += 2;

	printf("%s\n", (const char*)a.getString());
	printf("%s\n", (const char*)ref.getString());

	b = "hallo";

	ref = a+b;

	printf("%s\n", (const char*)a.getString());
	printf("%s\n", (const char*)ref.getString());

	ref++;
	printf("%s\n", (const char*)a.getString());
	printf("%s\n", (const char*)ref.getString());

	b.setarray(3);
	b[1] = 1;
	b[2] = 2.2;

	c = a;
	c = b;
	printf("%s\n", (const char*)c[0].getString());
	printf("%s\n", (const char*)c[1].getString());
	printf("%s\n\n", (const char*)c[2].getString());
	c = a+b;
	printf("%s\n", (const char*)c[0].getString());
	printf("%s\n", (const char*)c[1].getString());
	printf("%s\n\n", (const char*)c[2].getString());
	b = c++;
	printf("%s\n", (const char*)c[0].getString());
	printf("%s\n", (const char*)c[1].getString());
	printf("%s\n\n", (const char*)c[2].getString());
	printf("%s\n", (const char*)b[0].getString());
	printf("%s\n", (const char*)b[1].getString());
	printf("%s\n\n", (const char*)b[2].getString());

	c[1] += c[0];
	c[2] =  4;

	printf("%s\n", (const char*)c[0].getString());
	printf("%s\n", (const char*)c[1].getString());
	printf("%s\n\n", (const char*)c[2].getString());
	c += a;
	printf("%s\n", (const char*)c[0].getString());
	printf("%s\n", (const char*)c[1].getString());
	printf("%s\n\n", (const char*)c[2].getString());
	
	c[2] /= a;			printf("%s\n", (const char*)c[2].getString());
	c[2] &= a;			printf("%s\n", (const char*)c[2].getString());
	c[2] |= a;			printf("%s\n", (const char*)c[2].getString());
	c[2] ^= a;			printf("%s\n", (const char*)c[2].getString());
	c[2] = a && c[2];	printf("%s\n", (const char*)c[2].getString());
	c[2] = a || c[2];	printf("%s\n\n", (const char*)c[2].getString());

	printf("%s\n", (const char*)c[0].getString());
	printf("%s\n", (const char*)c[1].getString());
	printf("%s\n\n", (const char*)c[2].getString());




}
















///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


const unsigned char engine[] = 
{
#include "eascript.engine.txt"
};


//!! move to zlib wrapper
#include <zlib.h>

int zlib_decode(unsigned char *dest, unsigned long& destLen, const unsigned char* source, unsigned long sourceLen)
{
	z_stream stream;
	int err;

	stream.next_in = (Bytef*)source;
	stream.avail_in = (uInt)sourceLen;
	// Check for source > 64K on 16-bit machine:
	if ((uLong)stream.avail_in != sourceLen) return Z_BUF_ERROR;

	stream.next_out = (Bytef*) dest;
	stream.avail_out = (uInt)destLen;
	if ((unsigned long)stream.avail_out != destLen) return Z_BUF_ERROR;

	stream.zalloc = (alloc_func)0;
	stream.zfree = (free_func)0;

	err = inflateInit(&stream);
	if (err != Z_OK) return err;

	err = inflate(&stream, Z_FINISH);
	if (err != Z_STREAM_END) {
		inflateEnd(&stream);
		return err == Z_OK ? Z_BUF_ERROR : err;
	}
	destLen = stream.total_out;

	err = inflateEnd(&stream);
	return err;
}

int zlib_encode(unsigned char *dest, unsigned long& destLen, const unsigned char* source, unsigned long sourceLen)
{
	z_stream stream;
	int err;

	stream.next_in = (Bytef*)source;
	stream.avail_in = (uInt)sourceLen;
	/* Check for source > 64K on 16-bit machine: */
	if ((uLong)stream.avail_in != sourceLen) return Z_BUF_ERROR;

	stream.next_out = (Bytef*) dest;
	stream.avail_out = (uInt)destLen;
	if ((uLong)stream.avail_out != destLen) return Z_BUF_ERROR;

	stream.zalloc = (alloc_func)0;
	stream.zfree = (free_func)0;

	err = deflateInit(&stream,Z_BEST_COMPRESSION);
	if (err != Z_OK) return err;

	err = deflate(&stream, Z_FINISH);
	if (err != Z_STREAM_END) {
		deflateEnd(&stream);
		return err == Z_OK ? Z_BUF_ERROR : err;
	}
	destLen = stream.total_out;

	err = deflateEnd(&stream);
	return err;
}

const uchar* getEngine(ulong &sz)
{
	static uchar buffer[128*1024];

	sz = sizeof(buffer);
	if( 0==zlib_decode(buffer, sz, engine, sizeof(engine)) )
	{
		printf("loading engine...(%lu)\n", sz);
		return buffer;
	}
	return NULL;
}
void buildEngine()
{
	static uchar buffer[128*1024];
	static uchar buffer2[128*1024];
	FILE *fp = fopen("eascript.cgt", "rb");
	ulong sz=fread(buffer, 1, sizeof(buffer), fp);

	ulong i, sz2=sizeof(buffer2);
	zlib_encode(buffer2, sz2, buffer, sz);

	for(i=0; i<sz2; ++i)
	{
		if(i%8==0) printf("\n");
		printf("0x%02X, ", buffer2[i]);
	}
	exit(0);
}

