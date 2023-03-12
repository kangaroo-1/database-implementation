/*
 * src/tutorial/complex.c
 *
 ******************************************************************************
  This file contains routines that can be bound to a Postgres backend and
  called by the backend in the process of processing queries.  The calling
  format for these routines is dictated by Postgres architecture.
******************************************************************************/

#include "postgres.h"
#include <string.h>
#include "fmgr.h"
#include "libpq/pqformat.h"		/* needed for send/recv functions */

PG_MODULE_MAGIC;

typedef struct GeoCoord
{
	int32 length;
	char data[10];
	char location[256];
	char latitude[50];
	char longtitude[50];

}			GeoCoord;


/*****************************************************************************
 * Input/Output functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(geocoord_in);

Datum
geocoord_in(PG_FUNCTION_ARGS)
{
	char *str = PG_GETARG_CSTRING(0);
	


	// char *location;
	// char *geoStr;
	char locationBuffer[256];
	char buffer1[50];
	char buffer2[20];
	
	// char longtitudeBuffer[20];
	// char latitudeBuffer[20];
	// char geoStrBuffer[50];


	//for storing latitude and longitude



	int i = 0;
	// int j = 0;
	int location_index = 0;      // end index of location

	int index2 = 0;      // end index of latitude

	GeoCoord *result = (GeoCoord *) palloc(VARHDRSZ + 500);
	memset(locationBuffer, 0, sizeof(locationBuffer));
	memset(buffer1, 0, sizeof(buffer1));
	memset(buffer2, 0, sizeof(buffer2));

	//find the location location_index
	while (str[i] != '\0') {
		if (str[i] == ',') {
			location_index = i;
			break;
		}
		i++;
	}


	//find latitude index
	i++;
	while (str[i] != '\0') {
		if (str[i] == ',') {
			index2 = i;
			break;
		}
		i++;
	}

	//case: Melbourne,37.84°S 144.95°E, no comma
	if (index2 == 0) {
		i = location_index + 1;
		while (str[i] != '\0')  {
			if (str[i] == ' ')  {
				index2 = i;
				break;
			}
			i++;
		}

	}



	//copy strings into buffers
	strncpy(locationBuffer, str + 0, location_index - 0);
	strncpy(buffer1, str + location_index + 1, index2 - location_index);
	strncpy(buffer2, str + index2 + 1, strlen(str) - index2);

	locationBuffer[strlen(locationBuffer)] = '\0';
	buffer1[strlen(buffer1)] = '\0';
	buffer2[strlen(buffer2)] = '\0';

	// ereport(ERROR,
	// 			(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
	// 			 errmsg("%s", buffer1)));
	
	
	SET_VARSIZE(result, VARHDRSZ + 500);
	memset(result->location, 0, 256);
	memset(result->latitude, 0, 50);
	memset(result->longtitude, 0, 50);
	memcpy(result->location, locationBuffer, 256);
	memcpy(result->latitude,  buffer1, 50);
	memcpy(result->longtitude, buffer2, strlen(buffer2));
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(geocoord_out);

Datum
geocoord_out(PG_FUNCTION_ARGS)
{
	GeoCoord    *geocoord= (GeoCoord *) PG_GETARG_POINTER(0);
	char	   *result;
	result = psprintf("location: %s | latitude: %s | longtitude: %s ", geocoord->location, geocoord->latitude, geocoord->longtitude);
	PG_RETURN_CSTRING(result);
}

/*****************************************************************************
 * Binary Input/Output functions
 *
 * These are optional.
 *****************************************************************************/

// PG_FUNCTION_INFO_V1(complex_recv);

// Datum
// complex_recv(PG_FUNCTION_ARGS)
// {
// 	StringInfo	buf = (StringInfo) PG_GETARG_POINTER(0);
// 	Complex    *result;

// 	result = (Complex *) palloc(sizeof(Complex));
// 	result->x = pq_getmsgfloat8(buf);
// 	result->y = pq_getmsgfloat8(buf);
// 	PG_RETURN_POINTER(result);
// }

// PG_FUNCTION_INFO_V1(complex_send);

// Datum
// complex_send(PG_FUNCTION_ARGS)
// {
// 	Complex    *complex = (Complex *) PG_GETARG_POINTER(0);
// 	StringInfoData buf;

// 	pq_begintypsend(&buf);
// 	pq_sendfloat8(&buf, complex->x);
// 	pq_sendfloat8(&buf, complex->y);
// 	PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
// }

// /*****************************************************************************
//  * New Operators
//  *
//  * A practical Complex datatype would provide much more than this, of course.
//  *****************************************************************************/

// PG_FUNCTION_INFO_V1(complex_add);

// Datum
// complex_add(PG_FUNCTION_ARGS)
// {
// 	Complex    *a = (Complex *) PG_GETARG_POINTER(0);
// 	Complex    *b = (Complex *) PG_GETARG_POINTER(1);
// 	Complex    *result;

// 	result = (Complex *) palloc(sizeof(Complex));
// 	result->x = a->x + b->x;
// 	result->y = a->y + b->y;
// 	PG_RETURN_POINTER(result);
// }


// /*****************************************************************************
//  * Operator class for defining B-tree index
//  *
//  * It's essential that the comparison operators and support function for a
//  * B-tree index opclass always agree on the relative ordering of any two
//  * data values.  Experience has shown that it's depressingly easy to write
//  * unintentionally inconsistent functions.  One way to reduce the odds of
//  * making a mistake is to make all the functions simple wrappers around
//  * an internal three-way-comparison function, as we do here.
//  *****************************************************************************/

// #define Mag(c)	((c)->x*(c)->x + (c)->y*(c)->y)

// static int
// complex_abs_cmp_internal(Complex * a, Complex * b)
// {
// 	double		amag = Mag(a),
// 				bmag = Mag(b);

// 	if (amag < bmag)
// 		return -1;
// 	if (amag > bmag)
// 		return 1;
// 	return 0;
// }


// PG_FUNCTION_INFO_V1(complex_abs_lt);

// Datum
// complex_abs_lt(PG_FUNCTION_ARGS)
// {
// 	Complex    *a = (Complex *) PG_GETARG_POINTER(0);
// 	Complex    *b = (Complex *) PG_GETARG_POINTER(1);

// 	PG_RETURN_BOOL(complex_abs_cmp_internal(a, b) < 0);
// }

// PG_FUNCTION_INFO_V1(complex_abs_le);

// Datum
// complex_abs_le(PG_FUNCTION_ARGS)
// {
// 	Complex    *a = (Complex *) PG_GETARG_POINTER(0);
// 	Complex    *b = (Complex *) PG_GETARG_POINTER(1);

// 	PG_RETURN_BOOL(complex_abs_cmp_internal(a, b) <= 0);
// }

// PG_FUNCTION_INFO_V1(complex_abs_eq);

// Datum
// complex_abs_eq(PG_FUNCTION_ARGS)
// {
// 	Complex    *a = (Complex *) PG_GETARG_POINTER(0);
// 	Complex    *b = (Complex *) PG_GETARG_POINTER(1);

// 	PG_RETURN_BOOL(complex_abs_cmp_internal(a, b) == 0);
// }

// PG_FUNCTION_INFO_V1(complex_abs_ge);

// Datum
// complex_abs_ge(PG_FUNCTION_ARGS)
// {
// 	Complex    *a = (Complex *) PG_GETARG_POINTER(0);
// 	Complex    *b = (Complex *) PG_GETARG_POINTER(1);

// 	PG_RETURN_BOOL(complex_abs_cmp_internal(a, b) >= 0);
// }

// PG_FUNCTION_INFO_V1(complex_abs_gt);

// Datum
// complex_abs_gt(PG_FUNCTION_ARGS)
// {
// 	Complex    *a = (Complex *) PG_GETARG_POINTER(0);
// 	Complex    *b = (Complex *) PG_GETARG_POINTER(1);

// 	PG_RETURN_BOOL(complex_abs_cmp_internal(a, b) > 0);
// }

// PG_FUNCTION_INFO_V1(complex_abs_cmp);

// Datum
// complex_abs_cmp(PG_FUNCTION_ARGS)
// {
// 	Complex    *a = (Complex *) PG_GETARG_POINTER(0);
// 	Complex    *b = (Complex *) PG_GETARG_POINTER(1);

// 	PG_RETURN_INT32(complex_abs_cmp_internal(a, b));
// }

PG_FUNCTION_INFO_V1(geocoord_eq);

Datum
geocoord_eq(PG_FUNCTION_ARGS)
{
	GeoCoord    *a = (GeoCoord *) PG_GETARG_POINTER(0);
	GeoCoord    *b = (GeoCoord *) PG_GETARG_POINTER(1);

	int cmp = strcmp(a->location, b->location);


	PG_RETURN_BOOL(cmp == 0);
}
