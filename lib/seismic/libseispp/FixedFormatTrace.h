#ifndef _FIXEDFORMATTRACE_H_
#define _FIXEDFORMATTRACE_H_
#include <stdio.h>
#include <string>
#include "HeaderMap.h"
namespace SEISPP{
using namespace std;
using namespace SEISPP;

/*! \brief Generic object that can be used to easily access seismic data
in a number of common external formats.

Most seismic data is stored externally in files with trace header
having a fixed set of attributes in specific spots.  Classic examples
are SEGY and SAC.  The object was designd to provide a common interface
into any seismic data format that is structured as a header in one
distinct binary block and data in another.  The concept used here is
to simply read the data in as a raw binary object and provide 
an series of indices into the the binary blob through the API.  

The construction of one of these beasts is a bit unusual and 
worth noting up front.  Because the mapping operation of data
and trace attributes in formats like this is fixed, we don't 
want to rebuild the structure over and over again.  Hence the
normal usage is to call a simple constuctor that builds a skeleton
for the particular external format.  One then uses this skeleton
in secondary calls that have a FILE pointer as an argument.  
Reading then involves cloning the skeleton and filling in the
actual data with raw binary read (putting flesh on the bones). 
*/
class FixedFormatTrace : public BasicTimeSeries
{
public:
	/*! Construct an empty trace object defined by name type.

	This constructor is used in two very different contexts.
	First, if one want to read a set of data in a specific
	format one should first call this constructor defaulting
	the nsamp parameter (zero value).  When nsamp is zero
	this contructor just builds the skeleton of the object
	with no data.  Later reads with a FILE parameter can be 
	used to load many traces without the overhead of building
	the complicated indices required when this particular
	constructor is called.

	The second use of this constructor is to build an empty 
	object of this type with a specified number of samples
	and all the attributes set to default values.  

	Note the both uses of this constructor hit a special parameter
	file that is assumed to have been installed in 
	the standard Antelope location for parameter files
	($ANTELOPE/data/pf).  The indexing for this particular
	format defined by the parameter passed as the argument type
	is defined in this parameter file.  This is a complicated
	parameter file with nested parameters.  Full description of
	it will wait until this code is in production form.  At 
	this writing it is experimental. 

	\param type data format name.  This name is used to search
	for a tag name in the parameter file that defines the
	layout of this particular data format.

	\param nsamp number of samples in data are to create for this 
	object.  If 0 (default) no data are is created and the 
	result can be used as a skeleton to efficiently clone objects
	of this type. 

	\exception throws a SeisppError for a variety of conditions
	possible in parsing the complicated parameter file used to
	describe a data format.
	*/
	FixedFormatTrace(string type,int nsnew=0);
	/*! \brief standard copy constructor. 

	Not trivial because of need to manage internal pointers.
	New block of memory is allocated and copied by this 
	constructor so be aware. */
	FixedFormatTrace(const FixedFormatTrace& parent);
	/*! Main constructor for this object.  

	This data object is designed to allow raw binary reads of 
	trace data from a file with fixed length binary header that
	can be loaded with fread.   It is assumed the interface 
	takes care of all details of mapping the actual bits to 
	something that can be extracted.  This constructor clones
	the skeleton that defines the indexing from parent and 
	reads ns samples from the file pointer fp.  This is possible
	because the skeleton allows the number of bytes to be 
	read by fread to be computed.
	\param parent whose skeleton is used to build this object
	\param fp plain C i/o FILE handle to read binary data
	\param nsamp number of samples to read from fp
	\param key keyword used to extract dt sample interval from 
		header after the data are read.
	\param key_is_dt some formats stored dt and other store
		samprate=1/dt.  When true (default) attribute extracted
		by key is assumed to be dt.  Otherwise the attribute
		will be converted to dt by 1/value.
	*/
	FixedFormatTrace(const FixedFormatTrace& parent,FILE *fp,int nsamp,
		string key, bool key_is_dt=true);
	/*! Similar to above, but ns is extracted from the header using
	the keyword defined by nsamp_keyword */
	FixedFormatTrace(const FixedFormatTrace& parent,
		FILE *fp,string nsamp_keyword,string dt_keyword,
		bool key_is_dt=true);
	/*! Standard destructor.  Not trivial here because this
	object has hidden raw pointers. */
	~FixedFormatTrace(); 
	template <class T> T get(string name);
	template <class T> void put(string name, T value);
	template <class T> T get(const char *name);
	template <class T> void put(const char *name, T value);
	/*! \brief Return sample data as an STL vector.

	Usually one wants the entire vector of sample data to manipulate.
	For efficiency it is usually much better to call this method to
	retrieve all the than manipulating the contents one sample at
	a time with the operator() method.  In fact this method just
	calls operator() ns times to retrieve and convert all the contents
	to doubles.
	*/
	vector<double> data();
	/*! Return one sample value.  

	This is comparable in function to the data method, but retrieves
	only one sample at a time.  Sample retrieved is by sample number
	isap.*/
	double operator()(int isap);
	/*! Load a vector of data into the object.

	This method is the main one used to load data into this generic
	object.   For a variety of reasons my design choice or this was
        made intentionally less general than it could have been.  That is,
        one might want to use operator [] to access samples, but this proved
        both awkward and inefficient.  This led to this interface method that
        simply requires one supply a vector of some rational type. */
	template <class T>void load(vector<T> d);
	void zero_gaps();
private:
	HeaderMap header;
	AttributeType stype;
	unsigned char *h;  /* points to start of header data */
	unsigned char *d;  /* points to start of data samples */
	bool data_loaded;
	size_t size_of_this;
};
template <class T> T FixedFormatTrace::get(string name)
{
    try {
	const string base_error("FixedFormatTrace::get method: ");
	if(h==NULL)
		throw SeisppError(base_error
		 + string("data pointer for this object is NULL.\n")
		 + string("Probable coding error.  Check documentation.") );
	AttributeType rdtype=this->header.dtype(name);
	T result;
	switch (rdtype)
	{
	case INT32:
		int ival;
		ival=this->header.get<int>(name,h);
		result=static_cast<T>(ival);
		break;
	case INT16:
		short int sival;
		ival=this->header.get<short int>(name,h);
		result=static_cast<T>(sival);
		break;
	case BOOL:
		bool bval;
		bval=this->header.get_bool(name,h);
		/* this does nothing but seems necessary to 
		handle the general typing */
		result=static_cast<T>(bval);
		break;
	case BYTE:
		unsigned char ucval;
		ucval=this->header.get<unsigned char>(name,h);
		result=static_cast<T>(ucval);
		break;
	case REAL32:
		float rval;
		rval=this->header.get<float>(name,h);
		result=static_cast<T>(rval);
		break;
	case REAL64:
		double dval;
		dval=this->header.get<double>(name,h);
		result=static_cast<T>(dval);
		break;
	case STRING:
		if(typeid(T)==typeid(string))
		{
			string sval;
			sval=this->header.get_string(name,h);
			/* This oddity seems necessary to fool the compiler
			to make this template work on both string and 
			numeric values.  Basically the opaque pointer 
			intermediary and the reiterpret cast is an 
			I'm sure of this trick */
			T *stemp;
			stemp=reinterpret_cast<T *>(&sval);
			result = *stemp;
		}
		else
		{
			throw SeisppError(base_error
			 + string("cannot convert attribute=")
			 + name 
			 + string("\nType mismatch does not allow conversion. ")
			 + string("Check header attribute definitions.") );
		}
		break;
	case HDRINVALID:
	default:
		throw SeisppError(base_error
		 + string("Attribute tagged with name=")
		 + name
		 + string(" is not defined for data format requested") );
	}
	return(result);
    } catch (...) {throw;}

}
template <class T> void FixedFormatTrace::put(string name,T value)
{
    try {
	const string base_error("FixedFormatTrace::put method: ");
	if(h==NULL)
		throw SeisppError(base_error
		 + string("data pointer for this object is NULL.\n")
		 + string("Probable coding error.  Check documentation.") );
	AttributeType rdtype=this->header.dtype(name);
	switch (rdtype)
	{
	case INT32:
		/* this assumes int is 32 bits */
		int *ival;
		ival=reinterpret_cast<int *>(&value);
		this->header.put<int>(name,*ival,h);
		break;
	case INT16:
		short int *sival;
		sival=reinterpret_cast<short int *>(&value);
		this->header.put<short int>(name,*sival,h);
		break;
	case BYTE:
		unsigned char *ucval;
		ucval=reinterpret_cast<unsigned char *>(&value);
		this->header.put<unsigned char>(name,*ucval,h);
		break;
	case BOOL:
		bool *bval;
		bval=reinterpret_cast<bool *>(&value);
		this->header.put_bool(name,*bval,h);
	case REAL32:
		float *rval;
		rval=reinterpret_cast<float *>(&value);
		this->header.put<float>(name,*rval,h);
		break;
	case REAL64:
		double *dval;
		dval=reinterpret_cast<double *>(&value);
		this->header.put<double>(name,*dval,h);
		break;
	case STRING:
		/* Some sources say this is evil, but see no
		alternative.  Works for both string and 
		char * because HeaderMap overloads the 
		put_string method.*/
		if(typeid(T)==typeid(string))
		{
			string *stmp=reinterpret_cast<string *>(&value);
			this->header.put_string(name,*stmp,h);
		}
		else if(typeid(T)==typeid(char *))
		{
			char **chtmp=reinterpret_cast<char **>(&value);
			this->header.put_string(name,*chtmp,h);
		}
		else
		{
			throw SeisppError(base_error
			 + string("cannot convert attribute=")
			 + name 
			 + string(" There is a type mistmach.  Check header definition.") );
		}
		break;
	case HDRINVALID:
	default:
		throw SeisppError(base_error
		 + string("Attribute tagged with name=")
		 + name
		 + string(" is not defined for data format requested") );
	}
    } catch (...) {throw;}
}
template<class T> T FixedFormatTrace::get(const char *name)
{
	try {
		T result=this->get<T>(string(name));
		return(result);
	} catch(...){throw;};
}
template<class T> void FixedFormatTrace::put(const char *name, T value)
{
	try {
		this->put<T>(string(name),value);
	} catch(...){throw;};
}
template <class T>void FixedFormatTrace::load(vector<T> dvec)
{
        int bytes_per_sample;
        switch(stype)
        {
        case INT16:
                bytes_per_sample=2;
                break;
        case REAL64:
                bytes_per_sample=8;
                break;
        case INT32:
        case REAL32:
        default:
                bytes_per_sample=4;
        }
	int dsize=dvec.size();
	size_t datasize;
	if(dsize!=ns)
	{
		datasize=bytes_per_sample*dsize;
		size_of_this=datasize+header.size();
		h=(unsigned char *)realloc(static_cast<void *>(h),size_of_this);
		if(h==NULL) 
		  throw SeisppError(string("FixedFormatTrace::load:  ")
			+string("realloc failed."));
		d=h+header.size();
		ns=dsize;
	}
	else
	{
		datasize=bytes_per_sample*ns;
	}
	int i;
	short int *siptr;
	int *iptr;
	float *fptr;
	double *dptr;
	switch (stype)
	{
	case INT16:
		siptr=new short int[ns];
		for(i=0;i<ns;++i) 
			siptr[i]=static_cast<short int>(dvec[i]);
		memcpy(static_cast<void *>(d),static_cast<const void *>(siptr),datasize);
		delete [] siptr;
		break;
	case INT32:
		iptr=new int[ns];
		for(i=0;i<ns;++i) 
			siptr[i]=static_cast<int>(dvec[i]);
		memcpy(static_cast<void *>(d),static_cast<const void *>(iptr),datasize);
		delete [] iptr;
		break;
	case REAL32:
		fptr=new float[ns];
		for(i=0;i<ns;++i) 
			fptr[i]=static_cast<float>(dvec[i]);
		memcpy(static_cast<void *>(d),static_cast<const void *>(fptr),datasize);
		delete [] fptr;
		break;
	case REAL64:
		dptr=new double[ns];
		for(i=0;i<ns;++i) 
			dptr[i]=static_cast<double>(dvec[i]);
		memcpy(static_cast<void *>(d),static_cast<const void *>(dptr),datasize);
		delete [] dptr;
		break;
	default:
		throw SeisppError(string("FixedFormatTrace::load:  ")
			+ string("Unsupported sample type format.  Cannot convert"));
		
	}
}
} // End SEISPP namespace encapsulation
#endif