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
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <math.h>
#include "fmgr.h"
#include <regex.h>
#include "libpq/pqformat.h"		/* needed for send/recv functions */

PG_MODULE_MAGIC;

typedef struct GeoCoord
{
	int32 length;
	char data[FLEXIBLE_ARRAY_MEMBER];

}			GeoCoord;


/*****************************************************************************
 * Input/Output functions
 *****************************************************************************/

//helper function to check if the inputs are valid
char* inputValid(char *str);
char* inputValid(char *str) {
	char location[256];
	char rest[200];
	char str1[100];
	char str2[100];
	double latitude,longtitude;
	int i;
	int index1;
	int index2;
	int flag;
    char *regex_pattern;
	regex_t regex;
	char *ptr;
	char *result;
	bool valid = true;
	char new_str[500];
	regex_pattern = "(-?)(0|([1-9][0-9]*))(\\.[0-9]+)?.(-?)(0|([1-9][0-9]*))(\\.[0-9]+)?°[S|N|W|E][,|" "](-?)(0|([1-9][0-9]*))(\\.[0-9]+)?.(-?)(0|([1-9][0-9]*))(\\.[0-9]+)?°[S|N|W|E]";
	i = 0;
	index1 = 0;
	index2 = 0;
	result = malloc(sizeof(char) * 500);
	memset(result, 0, sizeof(char) * 500);
	memset(new_str, 0, sizeof(char) * 500);
	memset(location, 0, sizeof(char) * 256);
	memset(rest, 0, sizeof(char) * 200);
	memset(str1, 0, sizeof(char) * 100);
	memset(str2, 0, sizeof(char) * 100);
	//extract location string
	while (str[i] != '\0') {
        if (str[i] == ',') {
			index1 = i;
            break;
        }
        i++;
    }
    
    if (i == 0) {
        valid = false;
    }

	strncpy(location, str + 0, index1- 0);
	//check if location string is valid
	 if (location[0] == ' ' || location[strlen(location) - 1] == ' ') {
        valid = false;
    }

	i = 0;
    while (location[i] != '\0') {
        if (location[i] != ' ' && isalpha(location[i]) == 0) {
             valid = false;
        }
        i++;
    }

	//extract langtitude and longtitude string
	strncpy(rest, str + index1 + 1 , strlen(str) - index1);
	flag = regcomp(&regex, regex_pattern, REG_EXTENDED | REG_NOSUB);
	if (flag == 0) {
		if (!regexec(&regex, rest, 0, NULL, 0)){
			valid = true;
		}
		else {
			 valid = false;
		}
	}
	regfree(&regex);
	
	i = 0;
	while (rest[i] != '\0') {
		if (rest[i] == ' ' || rest[i] == ',') {
			index2 = i;
			break;
		}
		i++;
	}

	strncpy(str1, rest + 0, index2 - 0);
	strncpy(str2, rest + index2 + 1, strlen(rest) - index2);
	if(strchr(str1, 'S') != NULL || strchr(str1, 'N') != NULL) {
        //str1 contains latitude
		latitude = strtod(str1, &ptr);
		longtitude = strtod(str2, &ptr);
		if (latitude > 90 || latitude < 0 || longtitude > 180 || longtitude < 0) {
				valid = false;
		}
		else {
			strcat(new_str, location);
			strcat(new_str, ",");
			strcat(new_str, str1);
			strcat(new_str, ",");
			strcat(new_str, str2);
			
			
		}
    }
	else {
		//str1 contains longtitudes
		latitude = strtod(str2, &ptr);
		longtitude = strtod(str1, &ptr);
		if (latitude > 90 || latitude < 0 || longtitude > 180 || longtitude < 0) {
			valid = false;		
		}
		else {
			strcat(new_str, location);
			strcat(new_str, ",");
			strcat(new_str, str2);
			strcat(new_str, ",");
			strcat(new_str, str1);
			elog(NOTICE, "new_str: %s", new_str);
			
		}

	}

	if (valid == false) {
		strcpy(result, "false");
	}
	else {
		strcpy(result, new_str);
	}
	return result;
	
	
}











PG_FUNCTION_INFO_V1(geocoord_in);

Datum
geocoord_in(PG_FUNCTION_ARGS)
{
	char *str = PG_GETARG_CSTRING(0);
	
	//check the inputs and error handling
	GeoCoord *result;
	char *str_valid;
	int len;

	str_valid = inputValid(str);
	if (strcmp(str_valid, "false") == 0) {
		ereport(ERROR,
				(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
				 errmsg("invalid input syntax for type %s: \"%s\"",
						"geocoord", str)));
	}



	len = strlen(str) + 1;
	result = (GeoCoord *) palloc(VARHDRSZ + len);


	memset(result->data, 0, VARHDRSZ + len);
	SET_VARSIZE(result, VARHDRSZ + len);
	memcpy(result->data, str_valid, strlen(str_valid));
	// elog(NOTICE, "%s", result->data);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(geocoord_out);

Datum
geocoord_out(PG_FUNCTION_ARGS)
{
	GeoCoord    *geocoord= (GeoCoord *) PG_GETARG_POINTER(0);
	
	char	   *result;
	result = psprintf("%s", geocoord->data);
	

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

static int
geocoord_cmp_internal(GeoCoord * a, GeoCoord * b)
{
	char *a_buffer = a->data;
	char *b_buffer = b->data;

	//buffers
	char a_location[256];
	char b_location[256];
	//for latitude
	char a_latitude[20];
	char b_latitude[20];
	double a_latitude_value;
	double b_latitude_value;
	int a_latitude_dir;    // N:1,  S:0
	int b_latitude_dir;

	//for longtitude
	char a_longtitude[20];
	char b_longtitude[20];
	double a_longtitude_value;
	double b_longtitude_value;
	int a_longtitude_dir;    // E:1 W:0
	int b_longtitude_dir;

	int i = 0;
	int index1 = 0;
	int index2 = 0;
	char *ptr;
	int location_cmp;


	memset(a_location, 0, 256);
	memset(b_location, 0, 256);
	memset(a_latitude, 0, 20);
	memset(b_latitude, 0, 20);
	memset(a_longtitude, 0, 20);
	memset(b_longtitude, 0, 20);



	//extract a 
	//find the first index
	while (a_buffer[i] != '\0') {
		if (a_buffer[i] == ',') {
			index1 = i;
			break;
		}
		i++;
	}

	strncpy(a_location, a_buffer + 0, index1 - 0);

	//find the second index
	i++;
	while (a_buffer[i] != '\0') {
		if (a_buffer[i] == ',') {
			index2 = i;
			break;
		}
		i++;
	}

	if (index2 == 0) {
		i = index1 + 1;
		while (a_buffer[i] != '\0') {
			if (a_buffer[i] == ' ') {
				index2 = i;
				break;
			}
			i++;
		}
	}

	//copy string into the buffer

	strncpy(a_latitude, a_buffer + index1+ 1, index2 - index1 - 1);
	strncpy(a_longtitude, a_buffer + index2+ 1, strlen(a_buffer) - index2);
	a_latitude_value = strtod(a_latitude, &ptr);
	a_longtitude_value = strtod(a_longtitude, &ptr);
	if (a_latitude[strlen(a_latitude) - 1] == 'N') {
		a_latitude_dir = 1;
	}
	else {
		a_latitude_dir = 0;
	}

	
	if (a_longtitude[strlen(a_longtitude) - 1] == 'E') {
		a_longtitude_dir = 1;
	}
	else {
		a_longtitude_dir = 0;
	}

	

	
	//extract b values
	//find the first index
	i = 0;
	index1 = 0;
	index2 = 0;
	while (b_buffer[i] != '\0') {
		if (b_buffer[i] == ',') {
			index1 = i;
			break;
		}
		i++;
	}

	strncpy(b_location, b_buffer + 0, index1 - 0);

	//find the second index
	i++;
	while (b_buffer[i] != '\0') {
		if (b_buffer[i] == ',') {
			index2 = i;
			break;
		}
		i++;
	}

	if (index2 == 0) {
		i = index1 + 1;
		while (b_buffer[i] != '\0') {
			if (b_buffer[i] == ' ') {
				index2 = i;
				break;
			}
			i++;
		}
	}

	//copy string into the buffer
	strncpy(b_location, b_buffer + 0, index1  - 0);
	strncpy(b_latitude, b_buffer + index1+ 1, index2 - index1 - 1);
	strncpy(b_longtitude, b_buffer + index2+ 1, strlen(b_buffer) - index2);

	if (b_latitude[strlen(b_latitude) - 1] == 'N') {
		b_latitude_dir = 1;
	}
	else {
		b_latitude_dir = 0;
	}


	if (b_longtitude[strlen(b_longtitude) - 1] == 'E') {
		b_longtitude_dir = 1;
	}
	else {
		b_longtitude_dir = 0;
	}

	b_latitude_value = strtod(b_latitude, &ptr);
	b_longtitude_value = strtod(b_longtitude, &ptr);



	//compare location
	location_cmp = strcasecmp(a_location, b_location);
	// ereport(ERROR,
	// 			(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
	// 			 errmsg("error: %s | %s | %d", a_location, b_location, location_cmp)));

	if (location_cmp == 0 && a_latitude_value == b_latitude_value && a_longtitude_value == b_longtitude_value) {
		return 0;
	}
	else {
		
		//compare latitude first
		if (a_latitude_value < b_latitude_value) {
			return 1;
		}
		else if (a_latitude_value > b_latitude_value) {
			return -1;
		}
		else if (a_latitude_dir > b_latitude_dir) {
			return 1;
		}
		else if (a_latitude_dir < b_latitude_dir) {
			return -1;
		}
		else if (a_longtitude_value < b_longtitude_value) {
			return 1;
		}
		else if (a_longtitude_value > b_longtitude_value) {
			return -1;
		}
		else if (a_longtitude_dir > b_longtitude_dir) {
			return 1;
		}
		else if (a_longtitude_dir < b_longtitude_dir) {
			return -1;
		}
		else {
			return location_cmp;
		}
 



	}
	
}

PG_FUNCTION_INFO_V1(geocoord_cmp);
Datum
geocoord_cmp(PG_FUNCTION_ARGS)
{
	GeoCoord    *a = (GeoCoord *) PG_GETARG_POINTER(0);
	GeoCoord    *b = (GeoCoord *) PG_GETARG_POINTER(1);

	PG_RETURN_INT32(geocoord_cmp_internal(a, b));
}



PG_FUNCTION_INFO_V1(geocoord_eq);
Datum
geocoord_eq(PG_FUNCTION_ARGS)
{
	GeoCoord    *a = (GeoCoord *) PG_GETARG_POINTER(0);
	GeoCoord    *b = (GeoCoord *) PG_GETARG_POINTER(1);


	PG_RETURN_BOOL(geocoord_cmp_internal(a, b) == 0);
}


PG_FUNCTION_INFO_V1(geocoord_gt);
Datum
geocoord_gt(PG_FUNCTION_ARGS)
{
	GeoCoord    *a = (GeoCoord *) PG_GETARG_POINTER(0);
	GeoCoord    *b = (GeoCoord *) PG_GETARG_POINTER(1);


	PG_RETURN_BOOL(geocoord_cmp_internal(a, b) > 0);
}


PG_FUNCTION_INFO_V1(geocoord_lt);
Datum
geocoord_lt(PG_FUNCTION_ARGS)
{
	GeoCoord    *a = (GeoCoord *) PG_GETARG_POINTER(0);
	GeoCoord    *b = (GeoCoord *) PG_GETARG_POINTER(1);


	PG_RETURN_BOOL(geocoord_cmp_internal(a, b) < 0);
}



PG_FUNCTION_INFO_V1(geocoord_ge);
Datum
geocoord_ge(PG_FUNCTION_ARGS)
{
	GeoCoord    *a = (GeoCoord *) PG_GETARG_POINTER(0);
	GeoCoord    *b = (GeoCoord *) PG_GETARG_POINTER(1);


	PG_RETURN_BOOL(geocoord_cmp_internal(a, b) >= 0);
}



PG_FUNCTION_INFO_V1(geocoord_le);
Datum
geocoord_le(PG_FUNCTION_ARGS)
{
	GeoCoord    *a = (GeoCoord *) PG_GETARG_POINTER(0);
	GeoCoord    *b = (GeoCoord *) PG_GETARG_POINTER(1);


	PG_RETURN_BOOL(geocoord_cmp_internal(a, b) <= 0);
}



//time zone ~
PG_FUNCTION_INFO_V1(geocoord_tz);
Datum
geocoord_tz(PG_FUNCTION_ARGS)
{
	GeoCoord    *a = (GeoCoord *) PG_GETARG_POINTER(0);
	GeoCoord    *b = (GeoCoord *) PG_GETARG_POINTER(1);

	char *a_buffer = a->data;
	char *b_buffer = b->data;
	char a_longtitude[20];
	char b_longtitude[20];
	double a_longtitude_value;
	double b_longtitude_value;
	double a_floor;
	double b_floor;
	
	int i;
	int index = 0;
	char *ptr;


	i = strlen(a_buffer) - 1;
	while (i >= 0) {
		if (a_buffer[i] == ',' || a_buffer[i] == ' ') {
			index = i;
			break;
		}
		i--;
	}
	strncpy(a_longtitude, a_buffer + index + 1, strlen(a_buffer) - index);
	a_longtitude_value = strtod(a_longtitude, &ptr);
	

	i = strlen(b_buffer) - 1;
	index = 0;
	while (i >= 0) {
		if (b_buffer[i] == ',' || b_buffer[i] == ' ') {
			index = i;
			break;
		}
		i--;
	}
	strncpy(b_longtitude, b_buffer + index + 1, strlen(b_buffer) - index);
	b_longtitude_value = strtod(b_longtitude, &ptr);

	a_longtitude_value = a_longtitude_value / 15;
	b_longtitude_value = b_longtitude_value / 15;
	a_floor = floor(a_longtitude_value);
	b_floor = floor(b_longtitude_value);

	PG_RETURN_BOOL(a_floor == b_floor);
}



PG_FUNCTION_INFO_V1(geocoord_tz_ne);
Datum
geocoord_tz_ne(PG_FUNCTION_ARGS)
{
	GeoCoord    *a = (GeoCoord *) PG_GETARG_POINTER(0);
	GeoCoord    *b = (GeoCoord *) PG_GETARG_POINTER(1);

	char *a_buffer = a->data;
	char *b_buffer = b->data;
	char a_longtitude[20];
	char b_longtitude[20];
	double a_longtitude_value;
	double b_longtitude_value;
	double a_floor;
	double b_floor;
	
	int i;
	int index = 0;
	char *ptr;


	i = strlen(a_buffer) - 1;
	while (i >= 0) {
		if (a_buffer[i] == ',' || a_buffer[i] == ' ') {
			index = i;
			break;
		}
		i--;
	}
	strncpy(a_longtitude, a_buffer + index + 1, strlen(a_buffer) - index);
	a_longtitude_value = strtod(a_longtitude, &ptr);
	

	i = strlen(b_buffer) - 1;
	index = 0;
	while (i >= 0) {
		if (b_buffer[i] == ',' || b_buffer[i] == ' ') {
			index = i;
			break;
		}
		i--;
	}
	strncpy(b_longtitude, b_buffer + index + 1, strlen(b_buffer) - index);
	b_longtitude_value = strtod(b_longtitude, &ptr);

	a_longtitude_value = a_longtitude_value / 15;
	b_longtitude_value = b_longtitude_value / 15;
	a_floor = floor(a_longtitude_value);
	b_floor = floor(b_longtitude_value);

	PG_RETURN_BOOL(a_floor != b_floor);
}

PG_FUNCTION_INFO_V1(convert2dms);

Datum
convert2dms(PG_FUNCTION_ARGS)
{
	GeoCoord    *a = (GeoCoord *) PG_GETARG_POINTER(0);
	GeoCoord    *result;

	char *a_buffer = a->data;
	//buffers
	char a_location[256];
	//for latitude
	char a_latitude[20];
	double a_latitude_value;
	int a_latitude_dir;    // N:1,  S:0

	//for longtitude
	char a_longtitude[20];
	double a_longtitude_value;
	int a_longtitude_dir;    // E:1 W:0

	int i = 0;
	int index1 = 0;
	int index2 = 0;
	char *ptr;

	//for DMS calculation
	int a_latitude_d;
	float temp_a_latitude_m;
	int a_latitude_m;
	float temp_a_latitude_s;
	int a_latitude_s;

	int a_longtitude_d;
	float temp_a_longtitude_m;
	int a_longtitude_m;
	float temp_a_longtitude_s;
	int a_longtitude_s;

	char a_latitude_d_buffer[10];
	char a_latitude_m_buffer[10];
	char a_latitude_s_buffer[10];
	char a_longtitude_d_buffer[10];
	char a_longtitude_m_buffer[10];
	char a_longtitude_s_buffer[10];

	char a_new_latitude[30];
	char a_new_longtitude[30];
	char new_str[350];
	int len;




	memset(a_location, 0, 256);
	memset(a_latitude, 0, 20);
	memset(a_longtitude, 0, 20);
	memset(a_new_latitude, 0, 30);
	memset(a_new_longtitude, 0, 30);
	memset(new_str, 0, 350);

	memset(a_latitude_d_buffer, 0, 10);
	memset(a_latitude_m_buffer, 0, 10);
	memset(a_latitude_s_buffer, 0, 10);
	memset(a_longtitude_d_buffer, 0, 10);
	memset(a_longtitude_m_buffer, 0, 10);
	memset( a_longtitude_s_buffer, 0, 10);


	//extract a 
	//find the first index
	while (a_buffer[i] != '\0') {
		if (a_buffer[i] == ',') {
			index1 = i;
			break;
		}
		i++;
	}
	

	strncpy(a_location, a_buffer + 0, index1 - 0);
	// ereport(ERROR,
	// (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
 	// errmsg("test1: %s", a_location)));

	//find the second index
	i++;
	while (a_buffer[i] != '\0') {
		if (a_buffer[i] == ',' || a_buffer[i] == ' ') {
			index2 = i;
			break;
		}
		i++;
	}


	//copy string into the buffer
	strncpy(a_latitude, a_buffer + index1+ 1, index2 - index1 - 1);
	strncpy(a_longtitude, a_buffer + index2+ 1, strlen(a_buffer) - index2);
	a_latitude_value = strtod(a_latitude, &ptr);
	a_longtitude_value = strtod(a_longtitude, &ptr);
	if (a_latitude[strlen(a_latitude) - 1] == 'N') {
		a_latitude_dir = 1;
	}
	else {
		a_latitude_dir = 0;
	}

	
	if (a_longtitude[strlen(a_longtitude) - 1] == 'E') {
		a_longtitude_dir = 1;
	}
	else {
		a_longtitude_dir = 0;
	}


	//DMS calculation
	a_latitude_d = floor(a_latitude_value);
	temp_a_latitude_m = 60 * fabs(a_latitude_value - a_latitude_d);
	a_latitude_m = floor(temp_a_latitude_m);
	temp_a_latitude_s = (3600 * fabs(a_latitude_value - a_latitude_d) - 60 * a_latitude_m);
	a_latitude_s = floor(temp_a_latitude_s);
	if (a_latitude_s == -1) {
		a_latitude_s = 0;
	}


	a_longtitude_d = floor(a_longtitude_value);
	temp_a_longtitude_m = (60 * fabs(a_longtitude_value - a_longtitude_d));
	a_longtitude_m = floor(temp_a_longtitude_m);
	temp_a_longtitude_s = (3600 * fabs(a_longtitude_value - a_longtitude_d) - 60 * a_longtitude_m);
	a_longtitude_s = floor(temp_a_longtitude_s);
	if (a_longtitude_s == -1) {
		a_longtitude_s = 0;
	}
	
	//strcat latitude string
	strcat(new_str, a_location);
	strcat(new_str, ",");
	snprintf(a_latitude_d_buffer, 10, "%d", a_latitude_d);
	strcat(new_str, a_latitude_d_buffer);
	strcat(new_str, "°");

	if (a_latitude_m != 0) {
		snprintf(a_latitude_m_buffer, 10, "%d", a_latitude_m);
		strcat(new_str, a_latitude_m_buffer);
		strcat(new_str, "\'");
	}

	if (a_latitude_s != 0) {
		snprintf(a_latitude_s_buffer, 10, "%d", a_latitude_s);
		strcat(new_str, a_latitude_s_buffer);
		strcat(new_str, "\"");
	}


	if (a_latitude_dir == 0) {
		strcat(new_str, "S");
	}
	else {
		strcat(new_str, "N");
	}
	strcat(new_str, ",");

	//strcat longtitude string
	snprintf(a_longtitude_d_buffer, 10, "%d", a_longtitude_d);
	strcat(new_str, a_longtitude_d_buffer);
	strcat(new_str, "°");
	if (a_longtitude_m != 0) {
		snprintf(a_longtitude_m_buffer, 10, "%d", a_longtitude_m);
		strcat(new_str, a_longtitude_m_buffer);
		strcat(new_str, "\'");
	}

	if (a_longtitude_s != 0) {
		snprintf(a_longtitude_s_buffer, 10, "%d", a_longtitude_s);
		strcat(new_str, a_longtitude_s_buffer);
		strcat(new_str, "\"");
	}


	if (a_longtitude_dir == 0) {
		strcat(new_str, "W");
	}
	else {
		strcat(new_str, "E");
	}


	
	len = strlen(new_str) + 1;
	result = (GeoCoord *) palloc(VARHDRSZ + len);
	memset(result->data, 0, VARHDRSZ + len);
	SET_VARSIZE(result, VARHDRSZ + len);
	memcpy(result->data, new_str, strlen(new_str));
	// PG_RETURN_POINTER(result);
	PG_RETURN_TEXT_P(result);
}