#ifndef _GCLGRID_H_
#define _GCLGRID_H_
#include <stdlib.h>
#include <string>
#include <vector>
#include "stock.h" 
#include "db.h"
#include "perf.h"
#include "coords.h"
#undef min
#undef max
#include "dmatrix.h"

#ifdef	__cplusplus

typedef struct geo_{
	double lat;
	double lon;
	double r;
} Geographic_point;
typedef struct cart_ {
	double x1;
	double x2;
	double x3;
} Cartesian_point;

// Disabled for now:
// verbosity level.  0 terse, 1 verbose, 2 very verbose
// extern int GCLverbose;  
class GCLgrid_error
{
public:
	string message;
	GCLgrid_error(){message="GCLgrid library error\n";};
	GCLgrid_error(const string mess){message=mess;};
	virtual void log_error(){cerr<<"GCLgrid_error: "<<message<<endl;};
};
		

class GCLgrid
{
	public:
		char name[10];  // name assigned to this coordinate grid
		double lat0, lon0, r0;  // geographical location of origin 
		double azimuth_y;  // Azimuth of positive y axis 
		double dx1_nom, dx2_nom;  // nominal grid spacing 
		int n1,n2;  // grid size in each component direction
		int i0, j0;  // origin location in grid 
		double x1low, x1high, x2low, x2high, x3low, x3high;// bounding box 
		double **x1, **x2, **x3; //cartesian coordinates of nodes
		// These were protected in an earlier version
		double gtoc_rmatrix[3][3];
		double translation_vector[3];

		GCLgrid(){
			n1=0;n2=0;x1=NULL;x2=NULL;x3=NULL;
		};
		GCLgrid(int n1size, int n2size);
		GCLgrid(int n1size, int n2size, char *n, double la0, double lo0,
			double radius0, double az, double dx1n, double dx2n, 
			int iorigin, int jorigin);
		GCLgrid(Dbptr db, char *nm);  // acquire from Antelope database 
		GCLgrid(const GCLgrid&);  //standard copy constructor
		GCLgrid& operator=(const GCLgrid& );
		bool operator==(const GCLgrid&);
		bool operator!=(const GCLgrid&);
		void dbsave(Dbptr, char *) throw(int);
		int lookup(double, double);
		void reset_index() {ix1=i0; ix2=j0;};
		void get_index(int *ind) {ind[0]=ix1; ind[1]=ix2;};
		Geographic_point ctog(double, double, double);
		Geographic_point ctog(Cartesian_point);
		Cartesian_point gtoc(double, double, double);
		Cartesian_point gtoc(Geographic_point);
		void set_transformation_matrix();
		dmatrix fetch_transformation_matrix();
		double *fetch_translation_vector();
		void compute_extents();
		Geographic_point geo_coordinates(int,int);
		double lat(int,int);
		double lon(int,int);
		double r(int,int);
		double depth(int,int);
		~GCLgrid();
		//
		//These derived classes need private access so are declared
		//friends
		//
		friend class GCLgrid3d;
		friend class GCLscalarfield;
		friend class GCLvectorfield;
		friend class GCLscalarfield3d;
		friend class GCLvectorfield3d;
	private:
		int ix1, ix2;
};
//3d version is identical except it requires 3 indexes instead of 2 for
//coordinates.  We use inheritance to simply this description.
class GCLgrid3d : public GCLgrid
{
	public:
		double dx3_nom;
		int n3;
		int k0;
		double ***x1, ***x2, ***x3;

		GCLgrid3d(){
			n1=0;n2=0;n3=0;
			x1=NULL;x2=NULL;x3=NULL;
		};
		GCLgrid3d(int n1size, int n2size, int n3size);
		GCLgrid3d(int n1size, int n2size, int n3size, 
			char *n, double la0, double lo0,
			double radius0, double az, 
			double dx1n, double dx2n, double dx3n,
			int iorigin, int jorigin);
		GCLgrid3d(Dbptr db, char *nm); 
		GCLgrid3d(const GCLgrid3d&); 
		GCLgrid3d& operator=(const GCLgrid3d& );
		void dbsave(Dbptr, char *) throw(int);
		int lookup(double, double, double);
		void reset_index() {ix1=i0; ix2=j0; ix3=k0;};
		void get_index(int *ind) {ind[0]=ix1; ind[1]=ix2; ind[2]=ix3;};
		void compute_extents();
		Geographic_point geo_coordinates(int,int,int);
		double lat(int,int,int);
		double lon(int,int,int);
		double r(int,int,int);
		double depth(int,int,int);
		~GCLgrid3d();
	private:
		int ix1, ix2, ix3;
};	  		
//
//These are generic scalar and vector fields defined on a GCLgrid object
//of 2 or 3d
//
class GCLscalarfield :  public GCLgrid
{
	public:
		double **val;

		GCLscalarfield();
		GCLscalarfield(int, int);
		GCLscalarfield(const GCLscalarfield&);
		GCLscalarfield(GCLgrid& );
		GCLscalarfield(Dbptr db, char *grdnm, char *fn);
		GCLscalarfield& operator=(const GCLscalarfield&);
		void dbsave(Dbptr,char *,char *,char *, char *) throw(int);
		void operator+=(GCLscalarfield&);
		void operator*=(double);
		double interpolate(double,double,double);
		~GCLscalarfield();
		friend ostream& operator << (ostream&,GCLscalarfield&);
};
class GCLvectorfield : public GCLgrid
{
	public:
		int nv;
		double ***val;

		GCLvectorfield();
		GCLvectorfield(int,int,int);
		GCLvectorfield(const GCLvectorfield&);
		GCLvectorfield(GCLgrid &,int);
		GCLvectorfield(Dbptr db, char *grdnm, char *fn,int nvsize); 
		GCLvectorfield& operator=(const GCLvectorfield&);
		void dbsave(Dbptr, char *,char *, char *, char *) throw(int);
		void operator+=(GCLvectorfield&);
		void operator*=(double);
		double *interpolate(double,double,double);
		friend ostream& operator << (ostream&,GCLvectorfield&);
		~GCLvectorfield();
};
class GCLscalarfield3d : public GCLgrid3d 
{
	public:
		double ***val;

		GCLscalarfield3d();
		GCLscalarfield3d(int,int,int);
		GCLscalarfield3d(GCLgrid3d &);
		GCLscalarfield3d(const GCLscalarfield3d&);
		GCLscalarfield3d(Dbptr db, char *grdnm, char *fn);
		GCLscalarfield3d& operator=(const GCLscalarfield3d&);
		void dbsave(Dbptr, char *,char *, char *, char *) throw(int);
		void operator+=(GCLscalarfield3d&);
		void operator*=(double);
		double interpolate(double,double,double);
		~GCLscalarfield3d();
		friend ostream& operator << (ostream&,GCLscalarfield3d&);
};
class GCLvectorfield3d : public GCLgrid3d
{
	public:
		int nv;
		double ****val;

		GCLvectorfield3d();
		GCLvectorfield3d(int,int,int,int);
		GCLvectorfield3d(const GCLvectorfield3d&);
		GCLvectorfield3d(GCLgrid3d &,int);
		GCLvectorfield3d(Dbptr db, char *grdnm, char *fn,int nvsize); 
		GCLvectorfield3d& operator=(const GCLvectorfield3d&);
		void dbsave(Dbptr, char *,char *, char *, char *) throw(int);
		void operator+=(GCLvectorfield3d&);
		void operator*=(double);
		double *interpolate(double,double,double);
		~GCLvectorfield3d();
		friend ostream& operator << (ostream&,GCLvectorfield3d&);
};
/*
//
//C++ helpers
//
*/
double r0_ellipse(double);
dmatrix *extract_gridline(GCLgrid3d& grid, int ix1, int ix2, int ix3, 
	int comp, bool reverse) throw(GCLgrid_error);
vector<double> pathintegral(GCLscalarfield3d& field,dmatrix& path)
                                throw(GCLgrid_error);
dmatrix ustrans(GCLgrid& g, double lat, double lon);
void initialize_1Dscalar(GCLscalarfield3d& field, 
	vector<double> val1d,vector<double> z1d,vector<double>grad);
void initialize_1Dscalar(GCLscalarfield3d& field, 
	vector<double> val1d,vector<double> z1d);
dmatrix& remap_path(GCLgrid3d& parentgrid, dmatrix& path, GCLgrid3d& newpathgrid);
#endif

#ifdef	__cplusplus
extern "C" {
#endif

	
/*
//
// plain C helper function prototypes 
//
*/
double ****create_4dgrid_contiguous(int, int, int, int);
double ***create_3dgrid_contiguous(int, int, int);
double **create_2dgrid_contiguous(int, int);
void free_4dgrid_contiguous(double ****,int,int,int);
void free_3dgrid_contiguous(double ***,int,int);
void free_2dgrid_contiguous(double **,int);
void fme_weights_ (double *, double *, double *);
#ifdef  __cplusplus
}
#endif

#endif
