
#include <math.h>
#include <float.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "liblwgeom.h"

/*
 * Lower this to reduce integrity checks
 */
#define PARANOIA_LEVEL 1

//#define PGIS_DEBUG 1

// This is an implementation of the functions defined in lwgeom.h

//forward decs
extern uchar *parse_lwg(const char* geometry, lwallocator allocfunc, lwreporter errfunc);

//*********************************************************************
// BOX routines

// returns the float thats very close to the input, but <=
//  handles the funny differences in float4 and float8 reps.


//these are taken from glibc
// some machines do *not* have these functions defined, so we give
//  an implementation of them here.

typedef int int32_tt;
typedef unsigned int u_int32_tt;


float nextafterf_custom(float x, float y);

typedef union
{
  float value;
  u_int32_tt word;
} ieee_float_shape_type;



#define GET_FLOAT_WORD(i,d)                                     \
do {                                                            \
  ieee_float_shape_type gf_u;                                   \
  gf_u.value = (d);                                             \
  (i) = gf_u.word;                                              \
} while (0)


#define SET_FLOAT_WORD(d,i)                                     \
do {                                                            \
  ieee_float_shape_type sf_u;                                   \
  sf_u.word = (i);                                              \
  (d) = sf_u.value;                                             \
} while (0)


// returns the next smaller or next larger float
// from x (in direction of y).
float nextafterf_custom(float x, float y)
{
        int32_tt hx,hy,ix,iy;

        GET_FLOAT_WORD(hx,x);
        GET_FLOAT_WORD(hy,y);
        ix = hx&0x7fffffff;             /* |x| */
        iy = hy&0x7fffffff;             /* |y| */

        if((ix>0x7f800000) ||   /* x is nan */
           (iy>0x7f800000))     /* y is nan */
           return x+y;
        if(x==y) return y;              /* x=y, return y */
        if(ix==0) {                             /* x == 0 */
            SET_FLOAT_WORD(x,(hy&0x80000000)|1);/* return +-minsubnormal */
            y = x*x;
            if(y==x) return y; else return x;   /* raise underflow flag */
        }
        if(hx>=0) {                             /* x > 0 */
            if(hx>hy) {                         /* x > y, x -= ulp */
                hx -= 1;
            } else {                            /* x < y, x += ulp */
                hx += 1;
            }
        } else {                                /* x < 0 */
            if(hy>=0||hx>hy){                   /* x < y, x -= ulp */
                hx -= 1;
            } else {                            /* x > y, x += ulp */
                hx += 1;
            }
        }
        hy = hx&0x7f800000;
        if(hy>=0x7f800000) return x+x;  /* overflow  */
        if(hy<0x00800000) {             /* underflow */
            y = x*x;
            if(y!=x) {          /* raise underflow flag */
                SET_FLOAT_WORD(y,hx);
                return y;
            }
        }
        SET_FLOAT_WORD(x,hx);
        return x;
}




float nextDown_f(double d)
{
	float result  = d;

	if ( ((double) result) <=d)
		return result;

	return nextafterf_custom(result, result - 1000000);

}

// returns the float thats very close to the input, but >=
//  handles the funny differences in float4 and float8 reps.
float nextUp_f(double d)
{
		float result  = d;

		if ( ((double) result) >=d)
			return result;

		return nextafterf_custom(result, result + 1000000);
}


// returns the double thats very close to the input, but <
//  handles the funny differences in float4 and float8 reps.
double nextDown_d(float d)
{
		double result  = d;

		if ( result < d)
			return result;

		return nextafterf_custom(result, result - 1000000);
}

// returns the double thats very close to the input, but >
//  handles the funny differences in float4 and float8 reps.
double nextUp_d(float d)
{
			double result  = d;

			if ( result > d)
				return result;

			return nextafterf_custom(result, result + 1000000);
}



// Convert BOX3D to BOX2D
// returned box2d is allocated with 'lwalloc'
BOX2DFLOAT4 *
box3d_to_box2df(BOX3D *box)
{
	BOX2DFLOAT4 *result = (BOX2DFLOAT4*) lwalloc(sizeof(BOX2DFLOAT4));

	if (box == NULL)
	{
		lwerror("box3d_to_box2df got NUL box");
		return result;
	}

	result->xmin = nextDown_f(box->xmin);
	result->ymin = nextDown_f(box->ymin);

	result->xmax = nextUp_f(box->xmax);
	result->ymax = nextUp_f(box->ymax);

	return result;
}

// Convert BOX3D to BOX2D using pre-allocated BOX2D
// returned box2d is allocated with 'lwalloc'
// return 0 on error (NULL input box)
int
box3d_to_box2df_p(BOX3D *box, BOX2DFLOAT4 *result)
{
	if (box == NULL)
	{
		lwnotice("box3d_to_box2df got NUL box");
		return 0;
	}

	result->xmin = nextDown_f(box->xmin);
	result->ymin = nextDown_f(box->ymin);

	result->xmax = nextUp_f(box->xmax);
	result->ymax = nextUp_f(box->ymax);

	return 1;
}



// convert BOX2D to BOX3D
// zmin and zmax are set to 0.0
BOX3D
box2df_to_box3d(BOX2DFLOAT4 *box)
{
	BOX3D result;

	if (box == NULL)
		return result;

	result.xmin = box->xmin;
	result.ymin = box->ymin;

	result.xmax = box->xmax;
	result.ymax = box->ymax;

	result.zmin = result.zmax = 0.0;

	return result;
}

// convert BOX2D to BOX3D, using pre-allocated BOX3D as output
// Z values are set to 0.0.
void
box2df_to_box3d_p(BOX2DFLOAT4 *box, BOX3D *out)
{
	if (box == NULL) return;

	out->xmin = box->xmin;
	out->ymin = box->ymin;

	out->xmax = box->xmax;
	out->ymax = box->ymax;

	out->zmin = out->zmax = 0.0;
}



// returns a BOX3D that encloses b1 and b2
// box3d_union(NULL,A) --> A
// box3d_union(A,NULL) --> A
// box3d_union(A,B) --> A union B
BOX3D *
box3d_union(BOX3D *b1, BOX3D *b2)
{
	BOX3D *result;

	result = lwalloc(sizeof(BOX3D));

	if ( (b1 == NULL) && (b2 == NULL) )
	{
		return NULL;
	}

	if  (b1 == NULL)
	{
		//return b2
		memcpy(result, b2, sizeof(BOX3D));
		return result;
	}
	if (b2 == NULL)
	{
		//return b1
		memcpy(result, b1, sizeof(BOX3D));
		return result;
	}

	if (b1->xmin < b2->xmin)
		result->xmin = b1->xmin;
	else
		result->xmin = b2->xmin;

	if (b1->ymin < b2->ymin)
			result->ymin = b1->ymin;
	else
		result->ymin = b2->ymin;


	if (b1->xmax > b2->xmax)
		result->xmax = b1->xmax;
	else
		result->xmax = b2->xmax;

	if (b1->ymax > b2->ymax)
		result->ymax = b1->ymax;
	else
		result->ymax = b2->ymax;

	if (b1->zmax > b2->zmax)
			result->zmax = b1->zmax;
	else
			result->zmax = b2->zmax;

	if (b1->zmin > b2->zmin)
		result->zmin = b1->zmin;
	else
		result->zmin = b2->zmin;

	return result;
}

// Make given ubox a union of b1 and b2
int
box3d_union_p(BOX3D *b1, BOX3D *b2, BOX3D *ubox)
{
	if ( (b1 == NULL) && (b2 == NULL) )
	{
		return 0;
	}

	if  (b1 == NULL)
	{
		memcpy(ubox, b2, sizeof(BOX3D));
		return 1;
	}
	if (b2 == NULL)
	{
		memcpy(ubox, b1, sizeof(BOX3D));
		return 1;
	}

	if (b1->xmin < b2->xmin)
		ubox->xmin = b1->xmin;
	else
		ubox->xmin = b2->xmin;

	if (b1->ymin < b2->ymin)
		ubox->ymin = b1->ymin;
	else
		ubox->ymin = b2->ymin;


	if (b1->xmax > b2->xmax)
		ubox->xmax = b1->xmax;
	else
		ubox->xmax = b2->xmax;

	if (b1->ymax > b2->ymax)
		ubox->ymax = b1->ymax;
	else
		ubox->ymax = b2->ymax;

	if (b1->zmax > b2->zmax)
		ubox->zmax = b1->zmax;
	else
		ubox->zmax = b2->zmax;

	if (b1->zmin > b2->zmin)
		ubox->zmin = b1->zmin;
	else
		ubox->zmin = b2->zmin;

	return 1;
}

// returns a pointer to internal storage, or NULL
// if the serialized form does not have a BBOX.
//BOX2DFLOAT4 *
//getbox2d_internal(uchar *srl)
//{
//	if (TYPE_HASBBOX(srl[0])) return (BOX2DFLOAT4 *)(srl+1);
//	else return NULL;
//}

// same as getbox2d, but modifies box instead of returning result on the stack
int
getbox2d_p(uchar *srl, BOX2DFLOAT4 *box)
{
	uchar type = srl[0];
	uchar *loc;
	BOX3D box3d;

#ifdef PGIS_DEBUG
	lwnotice("getbox2d_p call");
#endif

	loc = srl+1;

	if (lwgeom_hasBBOX(type))
	{
		//woot - this is easy
#ifdef PGIS_DEBUG
		lwnotice("getbox2d_p: has box");
#endif
		memcpy(box, loc, sizeof(BOX2DFLOAT4));
		return 1;
	}

#ifdef PGIS_DEBUG
	lwnotice("getbox2d_p: has no box - computing");
#endif

	//we have to actually compute it!
	//lwnotice("getbox2d_p:: computing box");
	if ( ! compute_serialized_box3d_p(srl, &box3d) ) return 0;

#ifdef PGIS_DEBUG
	lwnotice("getbox2d_p: compute_serialized_box3d returned %p", box3d);
#endif

	if ( ! box3d_to_box2df_p(&box3d, box) ) return 0;

#ifdef PGIS_DEBUG
	lwnotice("getbox2d_p: box3d converted to box2d");
#endif

	return 1;
}

//************************************************************************
// POINTARRAY support functions


// copies a point from the point array into the parameter point
// will set point's z=0 (or NaN) if pa is 2d
// will set point's m=0 (or NaN( if pa is 3d or 2d
// NOTE: point is a real POINT3D *not* a pointer
POINT4D
getPoint4d(const POINTARRAY *pa, int n)
{
	POINT4D result;
	getPoint4d_p(pa, n, &result);
	return result;
}

// copies a point from the point array into the parameter point
// will set point's z=NO_Z_VALUE  if pa is 2d
// will set point's m=NO_M_VALUE  if pa is 3d or 2d
// NOTE: this will modify the point4d pointed to by 'point'.
int
getPoint4d_p(const POINTARRAY *pa, int n, POINT4D *op)
{
	uchar *ptr;
	int zmflag;

#if PARANOIA_LEVEL > 0
	if ( ! pa ) return 0;

	if ( (n<0) || (n>=pa->npoints))
	{
		lwerror("getPoint4d_p: point offset out of range");
		return 0; //error
	}
#endif

	/* Get a pointer to nth point offset and zmflag */
	ptr=getPoint_internal(pa, n);
	zmflag=TYPE_GETZM(pa->dims);

	switch (zmflag)
	{
		case 0: // 2d 
			memcpy(op, ptr, sizeof(POINT2D));
			op->m=NO_M_VALUE;
			op->z=NO_Z_VALUE;
			break;

		case 3: // ZM
			memcpy(op, ptr, sizeof(POINT4D));
			break;

		case 2: // Z
			memcpy(op, ptr, sizeof(POINT3DZ));
			op->m=NO_M_VALUE;
			break;

		case 1: // M
			memcpy(op, ptr, sizeof(POINT3DM));
			op->m=op->z; // we use Z as temporary storage
			op->z=NO_Z_VALUE;
			break;

		default:
			lwerror("Unknown ZM flag ??");
	}

	return 1;

}



// copies a point from the point array into the parameter point
// will set point's z=0 (or NaN) if pa is 2d
// NOTE: point is a real POINT3DZ *not* a pointer
POINT3DZ
getPoint3dz(const POINTARRAY *pa, int n)
{
	POINT3DZ result;
	getPoint3dz_p(pa, n, &result);
	return result;
}

// copies a point from the point array into the parameter point
// will set point's z=0 (or NaN) if pa is 2d
// NOTE: point is a real POINT3DZ *not* a pointer
POINT3DM
getPoint3dm(const POINTARRAY *pa, int n)
{
	POINT3DM result;
	getPoint3dm_p(pa, n, &result);
	return result;
}

// copies a point from the point array into the parameter point
// will set point's z=NO_Z_VALUE if pa is 2d
// NOTE: this will modify the point3dz pointed to by 'point'.
int
getPoint3dz_p(const POINTARRAY *pa, int n, POINT3DZ *op)
{
	uchar *ptr;

#if PARANOIA_LEVEL > 0
	if ( ! pa ) return 0;

	if ( (n<0) || (n>=pa->npoints))
	{
		lwnotice("%d out of numpoint range (%d)", n, pa->npoints);
		return 0; //error
	}
#endif

#ifdef PGIS_DEBUG
	lwnotice("getPoint3dz_p called on array of %d-dimensions / %u pts",
		TYPE_NDIMS(pa->dims), pa->npoints);
#endif

	/* Get a pointer to nth point offset */
	ptr=getPoint_internal(pa, n);

	/*
	 * if input POINTARRAY has the Z, it is always
	 * at third position so make a single copy
	 */
	if ( TYPE_HASZ(pa->dims) )
	{
		memcpy(op, ptr, sizeof(POINT3DZ));
	}

	/*
	 * Otherwise copy the 2d part and initialize
	 * Z to NO_Z_VALUE
	 */
	else
	{
		memcpy(op, ptr, sizeof(POINT2D));
		op->z=NO_Z_VALUE;
	}

	return 1;

}

// copies a point from the point array into the parameter point
// will set point's m=NO_M_VALUE if pa has no M
// NOTE: this will modify the point3dm pointed to by 'point'.
int
getPoint3dm_p(const POINTARRAY *pa, int n, POINT3DM *op)
{
	uchar *ptr;
	int zmflag;

#if PARANOIA_LEVEL > 0
	if ( ! pa ) return 0;

	if ( (n<0) || (n>=pa->npoints))
	{
		lwerror("%d out of numpoint range (%d)", n, pa->npoints);
		return 0; //error
	}
#endif 

#ifdef PGIS_DEBUG
	lwnotice("getPoint3dm_p(%d) called on array of %d-dimensions / %u pts",
		n, TYPE_NDIMS(pa->dims), pa->npoints);
#endif


	/* Get a pointer to nth point offset and zmflag */
	ptr=getPoint_internal(pa, n);
	zmflag=TYPE_GETZM(pa->dims);

	/*
	 * if input POINTARRAY has the M and NO Z,
	 * we can issue a single memcpy
	 */
	if ( zmflag == 1 )
	{
		memcpy(op, ptr, sizeof(POINT3DM));
		return 1;
	}

	/*
	 * Otherwise copy the 2d part and 
	 * initialize M to NO_M_VALUE
	 */
	memcpy(op, ptr, sizeof(POINT2D));

	/*
	 * Then, if input has Z skip it and
	 * copy next double, otherwise initialize
	 * M to NO_M_VALUE
	 */
	if ( zmflag == 3 )
	{
		ptr+=sizeof(POINT3DZ);
		memcpy(&(op->m), ptr, sizeof(double));
	}
	else
	{
		op->m=NO_M_VALUE;
	}

	return 1;
}


// copies a point from the point array into the parameter point
// z value (if present is not returned)
// NOTE: point is a real POINT2D *not* a pointer
POINT2D
getPoint2d(const POINTARRAY *pa, int n)
{
	POINT2D result;
	getPoint2d_p(pa, n, &result);
	return result;
}

// copies a point from the point array into the parameter point
// z value (if present is not returned)
// NOTE: this will modify the point2d pointed to by 'point'.
int
getPoint2d_p(const POINTARRAY *pa, int n, POINT2D *point)
{
#if PARANOIA_LEVEL > 0
	if ( ! pa ) return 0;

	if ( (n<0) || (n>=pa->npoints))
	{
		lwerror("getPoint2d_p: point offset out of range");
		return 0; //error
	}
#endif

	// this does x,y
	memcpy(point, getPoint_internal(pa, n), sizeof(POINT2D));
	return 1;
}

// get a pointer to nth point of a POINTARRAY
// You cannot safely cast this to a real POINT, due to memory alignment
// constraints. Use getPoint*_p for that.
uchar *
getPoint_internal(const POINTARRAY *pa, int n)
{
	int size;

#if PARANOIA_LEVEL > 0
	if ( pa == NULL ) {
		lwerror("getPoint got NULL pointarray");
		return NULL;
	}

	if ( (n<0) || (n>=pa->npoints))
	{
		return NULL; //error
	}
#endif

	size = pointArray_ptsize(pa);

	return &(pa->serialized_pointlist[size*n]);
}



// constructs a POINTARRAY.
// NOTE: points is *not* copied, so be careful about modification (can be aligned/missaligned)
// NOTE: ndims is descriptive - it describes what type of data 'points'
//       points to.  No data conversion is done.
POINTARRAY *
pointArray_construct(uchar *points, char hasz, char hasm,
	uint32 npoints)
{
	POINTARRAY  *pa;
	pa = (POINTARRAY*)lwalloc(sizeof(POINTARRAY));

	pa->dims = 0;
	TYPE_SETZM(pa->dims, hasz?1:0, hasm?1:0);
	pa->npoints = npoints;

	pa->serialized_pointlist = points;

	return pa;
}


//size of point represeneted in the POINTARRAY
// 16 for 2d, 24 for 3d, 32 for 4d
int
pointArray_ptsize(const POINTARRAY *pa)
{
	//fprintf(stderr, "pointArray_ptsize: TYPE_NDIMS(pa->dims)=%x\n", TYPE_NDIMS(pa->dims));
	return sizeof(double)*TYPE_NDIMS(pa->dims);
}


//***************************************************************************
// basic type handling


// returns true if this type says it has an SRID (S bit set)
char
lwgeom_hasSRID(uchar type)
{
	return TYPE_HASSRID(type);
}

// returns either 2,3, or 4 -- 2=2D, 3=3D, 4=4D
int
lwgeom_ndims(uchar type)
{
	return TYPE_NDIMS(type);
}

// has M ?
int lwgeom_hasM(uchar type)
{
	return  ( (type & 0x10) >>4);
}

// has Z ?
int lwgeom_hasZ(uchar type)
{
	return  ( (type & 0x20) >>5);
}


// get base type (ie. POLYGONTYPE)
int
lwgeom_getType(uchar type)
{
	return (type & 0x0F);
}


//construct a type (hasBOX=false)
uchar
lwgeom_makeType(char hasz, char hasm, char hasSRID, int type)
{
	return lwgeom_makeType_full(hasz, hasm, hasSRID, type, 0);
}

/*
 * Construct a type
 * TODO: needs to be expanded to accept explicit MZ type
 */
uchar
lwgeom_makeType_full(char hasz, char hasm, char hasSRID, int type, char hasBBOX)
{
	uchar result = (char)type;

	TYPE_SETZM(result, hasz, hasm);
	TYPE_SETHASSRID(result, hasSRID);
	TYPE_SETHASBBOX(result, hasBBOX);

	return result;
}

//returns true if there's a bbox in this LWGEOM (B bit set)
char
lwgeom_hasBBOX(uchar type)
{
	return TYPE_HASBBOX(type);
}

//*****************************************************************************
// basic sub-geometry types

// handle missaligned unsigned int32 data
uint32
get_uint32(const uchar *loc)
{
	uint32 result;

	memcpy(&result, loc, sizeof(uint32));
	return result;
}

// handle missaligned signed int32 data
int32
get_int32(const uchar *loc)
{
	int32 result;

	memcpy(&result,loc, sizeof(int32));
	return result;
}

//******************************************************************************





//*************************************************************************
// multi-geometry support
// note - for a simple type (ie. point), this will have sub_geom[0] = serialized_form.
// for multi-geomtries sub_geom[0] will be a few bytes into the serialized form
// This function just computes the length of each sub-object and pre-caches this info.
// For a geometry collection of multi* geometries, you can inspect the sub-components
// as well.
LWGEOM_INSPECTED *
lwgeom_inspect(const uchar *serialized_form)
{
	LWGEOM_INSPECTED *result = lwalloc(sizeof(LWGEOM_INSPECTED));
	uchar typefl = (uchar)serialized_form[0];
	uchar type;
	uchar **sub_geoms;
	const uchar *loc;
	int 	t;

#ifdef PGIS_DEBUG
	lwnotice("lwgeom_inspect: serialized@%p", serialized_form);
#endif

	if (serialized_form == NULL)
		return NULL;

	result->serialized_form = serialized_form; 
	result->type = (uchar) serialized_form[0];
	result->SRID = -1; // assume

	type = lwgeom_getType(typefl);

	loc = serialized_form+1;

	if ( lwgeom_hasBBOX(typefl) )
	{
		loc += sizeof(BOX2DFLOAT4);
	}

	if ( lwgeom_hasSRID(typefl) )
	{
		result->SRID = get_int32(loc);
		loc += 4;
	}

	if ( (type==POINTTYPE) || (type==LINETYPE) || (type==POLYGONTYPE) )
	{
		//simple geometry (point/line/polygon)-- not multi!
		result->ngeometries = 1;
		sub_geoms = (uchar**) lwalloc(sizeof(char*));
		sub_geoms[0] = (uchar *)serialized_form;
		result->sub_geoms = (uchar **)sub_geoms;
		return result;
	}

	// its a GeometryCollection or multi* geometry

	result->ngeometries = get_uint32(loc);
	loc +=4;
#ifdef PGIS_DEBUG
	lwnotice("lwgeom_inspect: geometry is a collection of %d elements",
		result->ngeometries);
#endif

	if ( ! result->ngeometries ) return result;

	sub_geoms = lwalloc(sizeof(uchar*) * result->ngeometries );
	result->sub_geoms = sub_geoms;
	sub_geoms[0] = (uchar *)loc;
#ifdef PGIS_DEBUG
	lwnotice("subgeom[0] @ %p (+%d)", sub_geoms[0], sub_geoms[0]-serialized_form);
#endif
	for (t=1;t<result->ngeometries; t++)
	{
		int sub_length = lwgeom_size_subgeom(sub_geoms[t-1], -1);//-1 = entire object
		sub_geoms[t] = sub_geoms[t-1] + sub_length;
#ifdef PGIS_DEBUG
		lwnotice("subgeom[%d] @ %p (+%d)",
			t, sub_geoms[t], sub_geoms[0]-serialized_form);
#endif
	}

	return result;

}


// 1st geometry has geom_number = 0
// if the actual sub-geometry isnt a POINT, null is returned (see _gettype()).
// if there arent enough geometries, return null.
// this is fine to call on a point (with geom_num=0),
// multipoint or geometrycollection
LWPOINT *
lwgeom_getpoint(uchar *serialized_form, int geom_number)
{
	int type = lwgeom_getType((uchar)serialized_form[0]);
	uchar *sub_geom;

	if ((type == POINTTYPE)  && (geom_number == 0))
	{
		//be nice and do as they want instead of what they say
		return lwpoint_deserialize(serialized_form);
	}

	if ((type != MULTIPOINTTYPE) && (type != COLLECTIONTYPE) )
		return NULL;

	sub_geom = lwgeom_getsubgeometry(serialized_form, geom_number);
	if (sub_geom == NULL)
		return NULL;

	type = lwgeom_getType(sub_geom[0]);
	if (type != POINTTYPE)
		return NULL;

	return lwpoint_deserialize(sub_geom);
}

// 1st geometry has geom_number = 0
// if the actual sub-geometry isnt a POINT, null is returned (see _gettype()).
// if there arent enough geometries, return null.
// this is fine to call on a point (with geom_num=0), multipoint or geometrycollection
LWPOINT *lwgeom_getpoint_inspected(LWGEOM_INSPECTED *inspected, int geom_number)
{
	char *sub_geom;
	uchar type;

	sub_geom = lwgeom_getsubgeometry_inspected(inspected, geom_number);

	if (sub_geom == NULL) return NULL;

	type = lwgeom_getType( (uchar) sub_geom[0]);
	if (type != POINTTYPE) return NULL;

	return lwpoint_deserialize(sub_geom);
}


// 1st geometry has geom_number = 0
// if the actual geometry isnt a LINE, null is returned (see _gettype()).
// if there arent enough geometries, return null.
// this is fine to call on a line, multiline or geometrycollection
LWLINE *
lwgeom_getline(uchar *serialized_form, int geom_number)
{
	uchar type = lwgeom_getType( (uchar) serialized_form[0]);
	uchar *sub_geom;

	if ((type == LINETYPE)  && (geom_number == 0))
	{
		//be nice and do as they want instead of what they say
		return lwline_deserialize(serialized_form);
	}

	if ((type != MULTILINETYPE) && (type != COLLECTIONTYPE) )
		return NULL;

	sub_geom = lwgeom_getsubgeometry(serialized_form, geom_number);
	if (sub_geom == NULL) return NULL;

	type = lwgeom_getType((uchar) sub_geom[0]);
	if (type != LINETYPE) return NULL;

	return lwline_deserialize(sub_geom);
}

// 1st geometry has geom_number = 0
// if the actual geometry isnt a LINE, null is returned (see _gettype()).
// if there arent enough geometries, return null.
// this is fine to call on a line, multiline or geometrycollection
LWLINE *lwgeom_getline_inspected(LWGEOM_INSPECTED *inspected, int geom_number)
{
	uchar *sub_geom;
	uchar type;

	sub_geom = lwgeom_getsubgeometry_inspected(inspected, geom_number);

	if (sub_geom == NULL) return NULL;

	type = lwgeom_getType((uchar) sub_geom[0]);
	if (type != LINETYPE) return NULL;

	return lwline_deserialize(sub_geom);
}

// 1st geometry has geom_number = 0
// if the actual geometry isnt a POLYGON, null is returned (see _gettype()).
// if there arent enough geometries, return null.
// this is fine to call on a polygon, multipolygon or geometrycollection
LWPOLY *
lwgeom_getpoly(uchar *serialized_form, int geom_number)
{
	uchar type = lwgeom_getType((uchar)serialized_form[0]);
	char *sub_geom;

	if ((type == POLYGONTYPE)  && (geom_number == 0))
	{
		//be nice and do as they want instead of what they say
		return lwpoly_deserialize(serialized_form);
	}

	if ((type != MULTIPOLYGONTYPE) && (type != COLLECTIONTYPE) )
		return NULL;

	sub_geom = lwgeom_getsubgeometry(serialized_form, geom_number);
	if (sub_geom == NULL) return NULL;

	type = lwgeom_getType((uchar) sub_geom[0]);
	if (type != POLYGONTYPE) return NULL;

	return lwpoly_deserialize(sub_geom);
}

// 1st geometry has geom_number = 0
// if the actual geometry isnt a POLYGON, null is returned (see _gettype()).
// if there arent enough geometries, return null.
// this is fine to call on a polygon, multipolygon or geometrycollection
LWPOLY *lwgeom_getpoly_inspected(LWGEOM_INSPECTED *inspected, int geom_number)
{
	char *sub_geom;
	uchar type;

	sub_geom = lwgeom_getsubgeometry_inspected(inspected, geom_number);

	if (sub_geom == NULL) return NULL;

	type = lwgeom_getType((uchar) sub_geom[0]);
	if (type != POLYGONTYPE) return NULL;

	return lwpoly_deserialize(sub_geom);
}

// this gets the serialized form of a sub-geometry
// 1st geometry has geom_number = 0
// if this isnt a multi* geometry, and geom_number ==0 then it returns
// itself
// returns null on problems.
// in the future this is how you would access a muli* portion of a
// geometry collection.
//    GEOMETRYCOLLECTION(MULTIPOINT(0 0, 1 1), LINESTRING(0 0, 1 1))
//   ie. lwgeom_getpoint( lwgeom_getsubgeometry( serialized, 0), 1)
//           --> POINT(1 1)
// you can inspect the sub-geometry as well if you wish.
uchar *
lwgeom_getsubgeometry(const uchar *serialized_form, int geom_number)
{
	//major cheat!!
	char * result;
	LWGEOM_INSPECTED *inspected = lwgeom_inspect(serialized_form);

	result = lwgeom_getsubgeometry_inspected(inspected, geom_number);
	pfree_inspected(inspected);
	return result;
}

uchar *
lwgeom_getsubgeometry_inspected(LWGEOM_INSPECTED *inspected, int geom_number)
{
	if ((geom_number <0) || (geom_number >= inspected->ngeometries) )
	{
		lwerror("lwgeom_getsubgeometry_inspected: geom_number out of range");
		return NULL;
	}

	return inspected->sub_geoms[geom_number];
}


// 1st geometry has geom_number = 0
//  use geom_number = -1 to find the actual type of the serialized form.
//    ie lwgeom_gettype( <'MULTIPOINT(0 0, 1 1)'>, -1)
//                 --> multipoint
//   ie lwgeom_gettype( <'MULTIPOINT(0 0, 1 1)'>, 0)
//                 --> point
// gets the 8bit type of the geometry at location geom_number
uchar
lwgeom_getsubtype(uchar *serialized_form, int geom_number)
{
	//major cheat!!
	char  result;
	LWGEOM_INSPECTED *inspected = lwgeom_inspect(serialized_form);

	result = lwgeom_getsubtype_inspected(inspected, geom_number);
	pfree_inspected(inspected);
	return result;
}

uchar
lwgeom_getsubtype_inspected(LWGEOM_INSPECTED *inspected, int geom_number)
{
	if ((geom_number <0) || (geom_number >= inspected->ngeometries) )
		return 99;

	return inspected->sub_geoms[geom_number][0]; // 1st byte is type
}


// how many sub-geometries are there?
//  for point,line,polygon will return 1.
int
lwgeom_getnumgeometries(uchar *serialized_form)
{
	uchar type = lwgeom_getType((uchar)serialized_form[0]);
	uchar *loc;

	if ( (type==POINTTYPE) || (type==LINETYPE) || (type==POLYGONTYPE) )
	{
		return 1;
	}

	loc = serialized_form+1;

	if (lwgeom_hasBBOX((uchar) serialized_form[0]))
	{
		loc += sizeof(BOX2DFLOAT4);
	}

	if (lwgeom_hasSRID((uchar) serialized_form[0]) )
	{
		loc += 4;
	}
	//its a GeometryCollection or multi* geometry
	return get_uint32(loc);
}

// how many sub-geometries are there?
//  for point,line,polygon will return 1.
int lwgeom_getnumgeometries_inspected(LWGEOM_INSPECTED *inspected)
{
	return inspected->ngeometries;
}


// set finalType to COLLECTIONTYPE or 0 (0 means choose a best type)
//   (ie. give it 2 points and ask it to be a multipoint)
//  use SRID=-1 for unknown SRID  (will have 8bit type's S = 0)
// all subgeometries must have the same SRID
// if you want to construct an inspected, call this then inspect the result...
uchar *
lwgeom_serialized_construct(int SRID, int finalType, char hasz, char hasm, int nsubgeometries, uchar **serialized_subs)
{
	uint32 *lengths;
	int t;
	int total_length = 0;
	char type = -1;
	char this_type = -1;
	uchar *result;
	uchar *loc;

	if (nsubgeometries == 0)
		return lwgeom_constructempty(SRID, hasz, hasm);

	lengths = lwalloc(sizeof(int32) * nsubgeometries);

	for (t=0;t<nsubgeometries;t++)
	{
		lengths[t] = lwgeom_size_subgeom(serialized_subs[t],-1);
		total_length += lengths[t];
		this_type = lwgeom_getType((uchar) (serialized_subs[t][0]));
		if (type == -1)
		{
			type = this_type;
		}
		else if (type == COLLECTIONTYPE)
		{
				//still a collection type...
		}
		else
		{
			if ( (this_type == MULTIPOINTTYPE) || (this_type == MULTILINETYPE)  || (this_type == MULTIPOLYGONTYPE) || (this_type == COLLECTIONTYPE) )
			{
				type = COLLECTIONTYPE;
			}
			else
			{
				if ( (this_type == POINTTYPE)  && (type==POINTTYPE) )
					type=MULTIPOINTTYPE;
				else if ( (this_type == LINETYPE)  && (type==LINETYPE) )
					type=MULTILINETYPE;
				else if ( (this_type == POLYGONTYPE)  && (type==POLYGONTYPE) )
					type=MULTIPOLYGONTYPE;
				else if ( (this_type == POLYGONTYPE)  && (type==MULTIPOLYGONTYPE) )
					;//nop
				else if ( (this_type == LINETYPE)  && (type==MULTILINETYPE) )
					;//nop
				else if ( (this_type == POINTTYPE)  && (type==MULTIPOINTTYPE) )
					;//nop
				else
					type = COLLECTIONTYPE;
			}
		}
	}

	if (type == POINTTYPE)
		type = MULTIPOINTTYPE;
	if (type == LINETYPE)
		type = MULTILINETYPE;
	if (type == POINTTYPE)
		type = MULTIPOINTTYPE;

	if (finalType == COLLECTIONTYPE)
		type = COLLECTIONTYPE;

	// now we have a mutli* or GEOMETRYCOLLECTION, lets serialize it

	if (SRID != -1)
		total_length +=4; // space for SRID

	total_length +=1 ;   // main type;
	total_length +=4 ;   // nsubgeometries

	result = lwalloc(total_length);
	result[0] = (uchar) lwgeom_makeType(hasz, hasm, SRID != -1,  type);
	if (SRID != -1)
	{
		memcpy(&result[1],&SRID,4);
		loc = result+5;
	}
	else
		loc = result+1;

	memcpy(loc,&nsubgeometries,4);
	loc +=4;

	for (t=0;t<nsubgeometries;t++)
	{
		memcpy(loc, serialized_subs[t], lengths[t] );
		loc += lengths[t] ;
	}

	lwfree(lengths);
	return result;
}


// construct the empty geometry (GEOMETRYCOLLECTION(EMPTY))
//returns serialized form
uchar *
lwgeom_constructempty(int SRID, char hasz, char hasm)
{
	int size = 0;
	uchar *result;
	int ngeoms = 0;
	uchar *loc;

	if (SRID != -1)
		size +=4;

	size += 5;

	result = lwalloc(size);

	result[0] = lwgeom_makeType(hasz, hasm, SRID != -1,  COLLECTIONTYPE);
	if (SRID != -1)
	{
		memcpy(&result[1],&SRID,4);
		loc = result+5;
	}
	else
		loc = result+1;

	memcpy(loc,&ngeoms,4);
	return result;
}

size_t
lwgeom_empty_length(int SRID)
{
	int size = 5;
	if ( SRID != 1 ) size += 4;
	return size;
}

// construct the empty geometry (GEOMETRYCOLLECTION(EMPTY))
// writing it into the provided buffer.
void
lwgeom_constructempty_buf(int SRID, char hasz, char hasm,
	uchar *buf, size_t *retsize)
{
	int ngeoms = 0;

	buf[0] =(uchar) lwgeom_makeType( hasz, hasm, SRID != -1,  COLLECTIONTYPE);
	if (SRID != -1)
	{
		memcpy(&buf[1],&SRID,4);
		buf += 5;
	}
	else
		buf += 1;

	memcpy(buf, &ngeoms, 4);

	if (retsize) *retsize = lwgeom_empty_length(SRID);
}

// helper function (not for general use)
// find the size a geometry (or a sub-geometry)
// 1st geometry has geom_number = 0
//  use geom_number = -1 to find the actual type of the serialized form.
//    ie lwgeom_gettype( <'MULTIPOINT(0 0, 1 1)'>, -1)
//                 --> size of the multipoint
//   ie lwgeom_gettype( <'MULTIPOINT(0 0, 1 1)'>, 0)
//         --> size of the point

// take a geometry, and find its length
size_t
lwgeom_size(const uchar *serialized_form)
{
	uchar type = lwgeom_getType((uchar) serialized_form[0]);
	int t;
	const uchar *loc;
	uint32 ngeoms;
	int sub_size;
	int result = 1; //"type"

#ifdef PGIS_DEBUG
	lwnotice("lwgeom_size called");
#endif

	if (type == POINTTYPE)
	{
#ifdef PGIS_DEBUG
		lwnotice("lwgeom_size: is a point");
#endif
		return lwgeom_size_point(serialized_form);
	}
	else if (type == LINETYPE)
	{
#ifdef PGIS_DEBUG
		lwnotice("lwgeom_size: is a line");
#endif
		return lwgeom_size_line(serialized_form);
	}
	else if (type == POLYGONTYPE)
	{
#ifdef PGIS_DEBUG
		lwnotice("lwgeom_size: is a polygon");
#endif
		return lwgeom_size_poly(serialized_form);
	}

	if ( type == 0 )
	{
		lwerror("lwgeom_size called with unknown-typed serialized geometry");
		return 0;
	}

	//handle all the multi* and geometrycollections the same
	//NOTE: for a geometry collection of GC of GC of GC we will be recursing...

#ifdef PGIS_DEBUG
	lwnotice("lwgeom_size called on a geoemtry with type %d", type);
#endif

	loc = serialized_form+1;

	if (lwgeom_hasBBOX((uchar) serialized_form[0]))
	{
#ifdef PGIS_DEBUG
		lwnotice("lwgeom_size: has bbox");
#endif

		loc += sizeof(BOX2DFLOAT4);
		result +=sizeof(BOX2DFLOAT4);
	}

	if (lwgeom_hasSRID( (uchar) serialized_form[0]) )
	{
#ifdef PGIS_DEBUG
		lwnotice("lwgeom_size: has srid");
#endif
		result +=4;
		loc +=4;
	}


	ngeoms = get_uint32(loc);
	loc +=4;
	result += 4; // numgeoms

#ifdef PGIS_DEBUG
	lwnotice("lwgeom_size called on a geoemtry with %d elems (result so far: %d)", ngeoms, result);
#endif

	for (t=0;t<ngeoms;t++)
	{
		sub_size = lwgeom_size(loc);
#ifdef PGIS_DEBUG
		lwnotice(" subsize %d", sub_size);
#endif
		loc += sub_size;
		result += sub_size;
	}

#ifdef PGIS_DEBUG
	lwnotice("lwgeom_size returning %d", result);
#endif
	return result;
}

size_t
lwgeom_size_subgeom(const uchar *serialized_form, int geom_number)
{
	if (geom_number == -1)
	{
		return lwgeom_size(serialized_form);
	}
	return lwgeom_size( lwgeom_getsubgeometry(serialized_form,geom_number));
}



int
lwgeom_size_inspected(const LWGEOM_INSPECTED *inspected, int geom_number)
{
	return lwgeom_size(inspected->sub_geoms[geom_number]);
}

int
compute_serialized_box3d_p(uchar *srl, BOX3D *out)
{
	BOX3D *box = compute_serialized_box3d(srl);
	if ( ! box ) return 0;
	out->xmin = box->xmin;
	out->ymin = box->ymin;
	out->zmin = box->zmin;
	out->xmax = box->xmax;
	out->ymax = box->ymax;
	out->zmax = box->zmax;
	lwfree(box);

	return 1;
}

// Compute bounding box of a serialized LWGEOM, even if it is
// already cached. The computed BOX2DFLOAT4 is stored at
// the given location, the function returns 0 is the geometry
// does not have a bounding box (EMPTY GEOM)
int
compute_serialized_box2d_p(uchar *srl, BOX2DFLOAT4 *out)
{
	  BOX3D *result = compute_serialized_box3d(srl);
	  if ( ! result ) return 0;
	  out->xmin = result->xmin;
	  out->xmax = result->xmax;
	  out->ymin = result->ymin;
	  out->ymax = result->ymax;
	  lwfree(result);

	  return 1;
}

//dont forget to lwfree() result
BOX3D *
compute_serialized_box3d(uchar *srl)
{
	int type = lwgeom_getType(srl[0]);
	int t;
	uchar *loc;
	uint32 ngeoms;
	BOX3D *result;
	BOX3D b1;
	int sub_size;
	char nboxes=0;

	//lwnotice("compute_serialized_box3d called");

#ifdef PGIS_DEBUG
lwnotice("compute_serialized_box3d called on type %d", type);
#endif

	if (type == POINTTYPE)
	{
		LWPOINT *pt = lwpoint_deserialize(srl);
#ifdef PGIS_DEBUG
lwnotice("compute_serialized_box3d: lwpoint deserialized");
#endif
		result = lwpoint_compute_box3d(pt);
#ifdef PGIS_DEBUG
lwnotice("compute_serialized_box3d: bbox found");
#endif
		pfree_point(pt);
		return result;
	}

	else if (type == LINETYPE)
	{
		LWLINE *line = lwline_deserialize(srl);
		result = lwline_compute_box3d(line);
		pfree_line(line);
		return result;

	}
	else if (type == POLYGONTYPE)
	{
		LWPOLY *poly = lwpoly_deserialize(srl);
		result = lwpoly_compute_box3d(poly);
		pfree_polygon(poly);
		return result;
	}

	if ( ! ( type == MULTIPOINTTYPE || type == MULTILINETYPE ||
		type == MULTIPOLYGONTYPE || type == COLLECTIONTYPE ) )
	{
		lwnotice("compute_serialized_box3d called on unknown type %d", type);
		return NULL;
	}

	loc = srl+1;

	if (lwgeom_hasBBOX(srl[0]))
	{
		loc += sizeof(BOX2DFLOAT4);
	}

	if (lwgeom_hasSRID(srl[0]) )
	{
		loc +=4;
	}

	ngeoms = get_uint32(loc);
	loc += 4;

	// each sub-type
	result = NULL;
	for (t=0; t<ngeoms; t++)
	{
		if ( compute_serialized_box3d_p(loc, &b1) ) 
		{
#ifdef PGIS_DEBUG
			lwnotice("Geom %d have box:"); printBOX3D(&b1);
#endif
			if (result)
			{
				nboxes += box3d_union_p(result, &b1, result);
			}
			else
			{ 
				result = lwalloc(sizeof(BOX3D));
				memcpy(result, &b1, sizeof(BOX3D));
			}
		}

		sub_size = lwgeom_size(loc);
		loc += sub_size;
	}

	return result;

}

//****************************************************************
// memory management -- these only delete the memory associated
//  directly with the structure - NOT the stuff pointing into
//  the original de-serialized info

void
pfree_inspected(LWGEOM_INSPECTED *inspected)
{
	if ( inspected->ngeometries )
		lwfree(inspected->sub_geoms);
	lwfree(inspected);
}


void pfree_POINTARRAY(POINTARRAY *pa)
{
	lwfree(pa);
}




//************************************************
//** debugging routines


void printBOX3D(BOX3D *box)
{
	lwnotice("BOX3D: %g %g, %g %g", box->xmin, box->ymin,
		box->xmax, box->ymax);
}

void printPA(POINTARRAY *pa)
{
	int t;
	POINT4D pt;
	uchar *mflag;

	if ( TYPE_HASM(pa->dims) ) mflag = "M";
	else mflag = "";

	lwnotice("      POINTARRAY%s{", mflag);
	lwnotice("                 ndims=%i,   ptsize=%i",
		TYPE_NDIMS(pa->dims), pointArray_ptsize(pa));
	lwnotice("                 npoints = %i", pa->npoints);

	for (t =0; t<pa->npoints;t++)
	{
		getPoint4d_p(pa, t, &pt);
		if (TYPE_NDIMS(pa->dims) == 2)
		{
			lwnotice("                    %i : %lf,%lf",t,pt.x,pt.y);
		}
		if (TYPE_NDIMS(pa->dims) == 3)
		{
			lwnotice("                    %i : %lf,%lf,%lf",t,pt.x,pt.y,pt.z);
		}
		if (TYPE_NDIMS(pa->dims) == 4)
		{
			lwnotice("                    %i : %lf,%lf,%lf,%lf",t,pt.x,pt.y,pt.z,pt.m);
		}
	}

	lwnotice("      }");
}

void printBYTES(uchar *a, int n)
{
	int t;
	char buff[3];

	buff[2] = 0; //null terminate

	lwnotice(" BYTE ARRAY (n=%i) IN HEX: {", n);
	for (t=0;t<n;t++)
	{
		deparse_hex(a[t], buff);
		lwnotice("    %i : %s", t,buff );
	}
	lwnotice("  }");
}


void
printMULTI(uchar *serialized)
{
	LWGEOM_INSPECTED *inspected = lwgeom_inspect(serialized);
	LWLINE  *line;
	LWPOINT *point;
	LWPOLY  *poly;
	int t;

	lwnotice("MULTI* geometry (type = %i), with %i sub-geoms",lwgeom_getType((uchar)serialized[0]), inspected->ngeometries);

	for (t=0;t<inspected->ngeometries;t++)
	{
		lwnotice("      sub-geometry %i:", t);
		line = NULL; point = NULL; poly = NULL;

		line = lwgeom_getline_inspected(inspected,t);
		if (line !=NULL)
		{
			printLWLINE(line);
		}
		poly = lwgeom_getpoly_inspected(inspected,t);
		if (poly !=NULL)
		{
			printLWPOLY(poly);
		}
		point = lwgeom_getpoint_inspected(inspected,t);
		if (point !=NULL)
		{
			printPA(point->point);
		}
    }

    lwnotice("end multi*");

	pfree_inspected(inspected);
}

void printType(uchar type)
{
	lwnotice("type 0x%x ==> hasBBOX=%i, hasSRID=%i, ndims=%i, type=%i",(unsigned int) type, lwgeom_hasBBOX(type), lwgeom_hasSRID(type),lwgeom_ndims(type), lwgeom_getType(type));
}

// get the SRID from the LWGEOM
// none present => -1
int
lwgeom_getsrid(uchar *serialized)
{
	uchar type = serialized[0];
	uchar *loc = serialized+1;

	if ( ! lwgeom_hasSRID(type)) return -1;

	if (lwgeom_hasBBOX(type))
	{
		loc += sizeof(BOX2DFLOAT4);
	}

	return get_int32(loc);
}

// get the SRID from the LWGEOM
// none present => -1
int pglwgeom_getSRID(PG_LWGEOM *lwgeom)
{
	uchar type = lwgeom->type;
	uchar *loc = lwgeom->data;

	if ( ! lwgeom_hasSRID(type)) return -1;

	if (lwgeom_hasBBOX(type))
	{
		loc += sizeof(BOX2DFLOAT4);
	}

	return get_int32(loc);
}

// Set the SRID of a PG_LWGEOM
// Returns a newly allocated PG_LWGEOM object.
// Allocation will be done using the lwalloc.
PG_LWGEOM *
pglwgeom_setSRID(PG_LWGEOM *lwgeom, int32 newSRID)
{
	uchar type = lwgeom->type;
	int bbox_offset=0; //0=no bbox, otherwise sizeof(BOX2DFLOAT4)
	int len,len_new,len_left;
	PG_LWGEOM *result;
	uchar *loc_new, *loc_old;

	if (lwgeom_hasBBOX(type))
		bbox_offset = sizeof(BOX2DFLOAT4);

	len = lwgeom->size;

	if (lwgeom_hasSRID(type))
	{
		if ( newSRID != -1 ) {
			//we create a new one and copy the SRID in
			result = lwalloc(len);
			memcpy(result, lwgeom, len);
			memcpy(result->data+bbox_offset, &newSRID,4);
		} else {
			//we create a new one dropping the SRID
			result = lwalloc(len-4);
			result->size = len-4;
			result->type = lwgeom_makeType_full(
				TYPE_HASZ(type), TYPE_HASM(type),
				0, lwgeom_getType(type),
				lwgeom_hasBBOX(type));
			loc_new = result->data;
			loc_old = lwgeom->data;
			len_left = len-4-1;

			// handle bbox (if there)
			if (lwgeom_hasBBOX(type))
			{
				memcpy(loc_new, loc_old, sizeof(BOX2DFLOAT4));
				loc_new += sizeof(BOX2DFLOAT4);
				loc_old += sizeof(BOX2DFLOAT4);
				len_left -= sizeof(BOX2DFLOAT4);
			}

			// skip SRID, copy the remaining
			loc_old += 4;
			len_left -= 4;
			memcpy(loc_new, loc_old, len_left);

		}

	}
	else 
	{
		// just copy input, already w/out a SRID
		if ( newSRID == -1 ) {
			result = lwalloc(len);
			memcpy(result, lwgeom, len);
		}
		// need to add one
		else {
			len_new = len + 4;//+4 for SRID
			result = lwalloc(len_new);
			memcpy(result, &len_new, 4); // size copy in
			result->type = lwgeom_makeType_full(
				TYPE_HASZ(type), TYPE_HASM(type),
				1, lwgeom_getType(type),lwgeom_hasBBOX(type));

			loc_new = result->data;
			loc_old = lwgeom->data;

			len_left = len -4-1;// old length - size - type

			// handle bbox (if there)

			if (lwgeom_hasBBOX(type))
			{
				memcpy(loc_new, loc_old, sizeof(BOX2DFLOAT4)) ;//copy in bbox
				loc_new += sizeof(BOX2DFLOAT4);
				loc_old += sizeof(BOX2DFLOAT4);
				len_left -= sizeof(BOX2DFLOAT4);
			}

			//put in SRID

			memcpy(loc_new, &newSRID,4);
			loc_new +=4;
			memcpy(loc_new, loc_old, len_left);
		}
	}
	return result;
}


char
ptarray_isccw(const POINTARRAY *pa)
{
	int i;
	double area = 0;
	POINT2D p1, p2;

	for (i=0; i<pa->npoints-1; i++)
	{
		getPoint2d_p(pa, i, &p1);
		getPoint2d_p(pa, i+1, &p2);
		area += (p1.x * p2.y) - (p1.y * p2.x);
	}
	if ( area > 0 ) return 0;
	else return 1;
}

// returns a BOX2DFLOAT4 that encloses b1 and b2
// box2d_union(NULL,A) --> A
// box2d_union(A,NULL) --> A
// box2d_union(A,B) --> A union B
BOX2DFLOAT4 *
box2d_union(BOX2DFLOAT4 *b1, BOX2DFLOAT4 *b2)
{
	BOX2DFLOAT4 *result;

	if ( (b1 == NULL) && (b2 == NULL) )
	{
		return NULL;
	}

	result = lwalloc(sizeof(BOX2DFLOAT4));

	if  (b1 == NULL)
	{
		memcpy(result, b2, sizeof(BOX2DFLOAT4));
		return result;
	}
	if (b2 == NULL)
	{
		memcpy(result, b1, sizeof(BOX2DFLOAT4));
		return result;
	}

	if (b1->xmin < b2->xmin) result->xmin = b1->xmin;
	else result->xmin = b2->xmin;

	if (b1->ymin < b2->ymin) result->ymin = b1->ymin;
	else result->ymin = b2->ymin;

	if (b1->xmax > b2->xmax) result->xmax = b1->xmax;
	else result->xmax = b2->xmax;

	if (b1->ymax > b2->ymax) result->ymax = b1->ymax;
	else result->ymax = b2->ymax;

	return result;
}

/*
 * ubox may be one of the two args...
 * return 1 if done something to ubox, 0 otherwise.
 */
int
box2d_union_p(BOX2DFLOAT4 *b1, BOX2DFLOAT4 *b2, BOX2DFLOAT4 *ubox)
{
	if ( (b1 == NULL) && (b2 == NULL) )
	{
		return 0;
	}

	if  (b1 == NULL)
	{
		memcpy(ubox, b2, sizeof(BOX2DFLOAT4));
		return 1;
	}
	if (b2 == NULL)
	{
		memcpy(ubox, b1, sizeof(BOX2DFLOAT4));
		return 1;
	}

	if (b1->xmin < b2->xmin) ubox->xmin = b1->xmin;
	else ubox->xmin = b2->xmin;

	if (b1->ymin < b2->ymin) ubox->ymin = b1->ymin;
	else ubox->ymin = b2->ymin;

	if (b1->xmax > b2->xmax) ubox->xmax = b1->xmax;
	else ubox->xmax = b2->xmax;

	if (b1->ymax > b2->ymax) ubox->ymax = b1->ymax;
	else ubox->ymax = b2->ymax;

	return 1;
}

const uchar *
lwgeom_typeflags(uchar type)
{
	static char flags[5];
	int flagno=0;
	if ( TYPE_HASZ(type) ) flags[flagno++] = 'Z';
	if ( TYPE_HASM(type) ) flags[flagno++] = 'M';
	if ( TYPE_HASBBOX(type) ) flags[flagno++] = 'B';
	if ( TYPE_HASSRID(type) ) flags[flagno++] = 'S';
	flags[flagno] = '\0';
	//lwnotice("Flags: %s - returning %p", flags, flags);
	return flags;
}

//given a string with at least 2 chars in it, convert them to
// a byte value.  No error checking done!
uchar
parse_hex(char *str)
{
	//do this a little brute force to make it faster

	uchar		result_high = 0;
	uchar		result_low = 0;

	switch (str[0])
	{
		case '0' :
			result_high = 0;
			break;
		case '1' :
			result_high = 1;
			break;
		case '2' :
			result_high = 2;
			break;
		case '3' :
			result_high = 3;
			break;
		case '4' :
			result_high = 4;
			break;
		case '5' :
			result_high = 5;
			break;
		case '6' :
			result_high = 6;
			break;
		case '7' :
			result_high = 7;
			break;
		case '8' :
			result_high = 8;
			break;
		case '9' :
			result_high = 9;
			break;
		case 'A' :
			result_high = 10;
			break;
		case 'B' :
			result_high = 11;
			break;
		case 'C' :
			result_high = 12;
			break;
		case 'D' :
			result_high = 13;
			break;
		case 'E' :
			result_high = 14;
			break;
		case 'F' :
			result_high = 15;
			break;
	}
	switch (str[1])
	{
		case '0' :
			result_low = 0;
			break;
		case '1' :
			result_low = 1;
			break;
		case '2' :
			result_low = 2;
			break;
		case '3' :
			result_low = 3;
			break;
		case '4' :
			result_low = 4;
			break;
		case '5' :
			result_low = 5;
			break;
		case '6' :
			result_low = 6;
			break;
		case '7' :
			result_low = 7;
			break;
		case '8' :
			result_low = 8;
			break;
		case '9' :
			result_low = 9;
			break;
		case 'A' :
			result_low = 10;
			break;
		case 'B' :
			result_low = 11;
			break;
		case 'C' :
			result_low = 12;
			break;
		case 'D' :
			result_low = 13;
			break;
		case 'E' :
			result_low = 14;
			break;
		case 'F' :
			result_low = 15;
			break;
	}
	return (uchar) ((result_high<<4) + result_low);
}


//given one byte, populate result with two byte representing
// the hex number
// ie deparse_hex( 255, mystr)
//		-> mystr[0] = 'F' and mystr[1] = 'F'
// no error checking done
void
deparse_hex(uchar str, uchar *result)
{
	int	input_high;
	int  input_low;

	input_high = (str>>4);
	input_low = (str & 0x0F);

	switch (input_high)
	{
		case 0:
			result[0] = '0';
			break;
		case 1:
			result[0] = '1';
			break;
		case 2:
			result[0] = '2';
			break;
		case 3:
			result[0] = '3';
			break;
		case 4:
			result[0] = '4';
			break;
		case 5:
			result[0] = '5';
			break;
		case 6:
			result[0] = '6';
			break;
		case 7:
			result[0] = '7';
			break;
		case 8:
			result[0] = '8';
			break;
		case 9:
			result[0] = '9';
			break;
		case 10:
			result[0] = 'A';
			break;
		case 11:
			result[0] = 'B';
			break;
		case 12:
			result[0] = 'C';
			break;
		case 13:
			result[0] = 'D';
			break;
		case 14:
			result[0] = 'E';
			break;
		case 15:
			result[0] = 'F';
			break;
	}

	switch (input_low)
	{
		case 0:
			result[1] = '0';
			break;
		case 1:
			result[1] = '1';
			break;
		case 2:
			result[1] = '2';
			break;
		case 3:
			result[1] = '3';
			break;
		case 4:
			result[1] = '4';
			break;
		case 5:
			result[1] = '5';
			break;
		case 6:
			result[1] = '6';
			break;
		case 7:
			result[1] = '7';
			break;
		case 8:
			result[1] = '8';
			break;
		case 9:
			result[1] = '9';
			break;
		case 10:
			result[1] = 'A';
			break;
		case 11:
			result[1] = 'B';
			break;
		case 12:
			result[1] = 'C';
			break;
		case 13:
			result[1] = 'D';
			break;
		case 14:
			result[1] = 'E';
			break;
		case 15:
			result[1] = 'F';
			break;
	}
}

uchar *
parse_lwgeom_wkt(char *wkt_input)
{
	uchar *serialized_form = parse_lwg(wkt_input,
		lwalloc, lwerror);


#ifdef PGIS_DEBUG
	lwnotice("parse_lwgeom_wkt with %s",wkt_input);
#endif

	if (serialized_form == NULL)
	{
		lwerror("parse_WKT:: couldnt parse!");
		return NULL;
	}

	return serialized_form;

}
