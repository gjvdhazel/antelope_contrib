#include <vector>
#include "gclgrid.h"
#include "dmatrix.h"
/* General purpose utility to integrate a scalar field variable
along a path defined by the input matrix path.  The prototype
example of this for us seismologists is integration of slowness
along a ray path.  Only the cartesian system of the field is 
used. The components of path are assumed stored in a 3xnc 
matrix whose components use the same basis as the field object.
The algorithm works along the path.  If the lookup function
ever fails for any reason the path will be truncated at the 
point where the lookup failed.  The caller should test for 
this condition by verifying that the length of the output
vector of doubles is the same as the number of columns in
the path matrix.  

The function returns a vector of doubles containing integral
along path of the field variable.

The routine throws an exception when the input matrix is not
correctly dimension.  This would normally be a programming error
and the caller may choose to not attempt to catch the error 
except in debugging.  

Author:  Gary L. Pavlis
Written:  June 2003
*/
vector <double> pathintegral(GCLscalarfield3d& field,dmatrix& path)
				throw(GCLgrid_error)
{
	int *sz;
	int npts;
	double outval,outval_last;
	int index[3];
	double val1,val2;  // averaged to get field value for interval
	double dx;  // path increment
	vector<double> outvec;
	int i;

	sz=path.size();
	npts = sz[1];
	if(sz[0]!=3) 
	  throw(GCLgrid_error("pathintegral:  input matrix of path coordinates has incorrect dimensions"));
	delete [] sz;  // no longer needed
	for( i=1,outval=0.0,outval_last=0.0;i<npts;++i)
	{
		double dx1,dx2,dx3;
		if(field.lookup(path(0,i-1),path(1,i-1),path(2,i-1))) break;
		val1=field.interpolate(path(0,i),path(1,i),path(2,i));
		if(field.lookup(path(0,i),path(1,i),path(2,i))) break;
		val2=field.interpolate(path(0,i),path(1,i),path(2,i));
		// This could be functionized, but I'll make it inline
		dx1 = path(0,i)-path(0,i-1);
		dx2 = path(1,i)-path(1,i-1);
		dx3 = path(2,i)-path(2,i-1);
		dx = sqrt(dx1*dx1+dx2*dx2+dx3*dx3);
		outval=outval_last+(val1+val2)*dx/2.0;
		outval_last = outval;
		outvec.push_back(outval);
	}
	return(outvec);
}