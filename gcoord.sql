---------------------------------------------------------------------------
--
-- complex.sql-
--    This file shows how to create a new user-defined type and how to
--    use this new type.
--
--
-- Portions Copyright (c) 1996-2022, PostgreSQL Global Development Group
-- Portions Copyright (c) 1994, Regents of the University of California
--
-- src/tutorial/gcoord.source
--
---------------------------------------------------------------------------

-----------------------------
-- Creating a new type:
--	We are going to create a new type called 'complex' which represents
--	complex numbers.
--	A user-defined type must have an input and an output function, and
--	optionally can have binary input and output functions.  All of these
--	are usually user-defined C functions.
-----------------------------

-- Assume the user defined functions are in /localstorage/z5290566/postgresql-15.1/src/tutorial/complex$DLSUFFIX
-- (we do not want to assume this is in the dynamic loader search path).
-- Look at $PWD/complex.c for the source.  Note that we declare all of
-- them as STRICT, so we do not need to cope with NULL inputs in the
-- C code.  We also mark them IMMUTABLE, since they always return the
-- same outputs given the same inputs.

-- the input function 'geocoord_in' takes a null-terminated string (the
-- textual representation of the type) and turns it into the internal
-- (in memory) representation. You will get a message telling you 'complex'
-- does not exist yet but that's okay.


CREATE FUNCTION geocoord_in(cstring)
   RETURNS geocoord
   AS '/localstorage/z5290566/postgresql-15.1/src/tutorial/gcoord'
   LANGUAGE C IMMUTABLE STRICT;

-- the output function 'geocoord_out' takes the internal representation and
-- converts it into the textual representation.

CREATE FUNCTION geocoord_out(geocoord)
   RETURNS cstring
   AS '/localstorage/z5290566/postgresql-15.1/src/tutorial/gcoord'
   LANGUAGE C IMMUTABLE STRICT;




-- now, we can create the type. The internallength specifies the size of the
-- memory block required to hold the type (we need two 8-byte doubles).

CREATE TYPE geocoord (
   internallength = variable,
   input = geocoord_in,
   output = geocoord_out,
   alignment = double
);


-----------------------------
-- Using the new type:
--	user-defined types can be used like ordinary built-in types.
-----------------------------

-- eg. we can use it in a table

CREATE TABLE test_complex (
	id integer primary key,
	a geocoord
);

-- data for user-defined types are just strings in the proper textual
-- representation.


SELECT * FROM test_complex;

-----------------------------
-- Interfacing New Types with Indexes:
--	We cannot define a secondary index (eg. a B-tree) over the new type
--	yet. We need to create all the required operators and support
--      functions, then we can make the operator class.
-----------------------------

-- first, define the required operators

CREATE FUNCTION geocoord_eq(geocoord, geocoord) RETURNS bool
   AS '/localstorage/z5290566/postgresql-15.1/src/tutorial/gcoord' LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION geocoord_gt(geocoord, geocoord) RETURNS bool
   AS '/localstorage/z5290566/postgresql-15.1/src/tutorial/gcoord' LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION geocoord_lt(geocoord, geocoord) RETURNS bool
   AS '/localstorage/z5290566/postgresql-15.1/src/tutorial/gcoord' LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION geocoord_ge(geocoord, geocoord) RETURNS bool
   AS '/localstorage/z5290566/postgresql-15.1/src/tutorial/gcoord' LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION geocoord_le(geocoord, geocoord) RETURNS bool
   AS '/localstorage/z5290566/postgresql-15.1/src/tutorial/gcoord' LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION geocoord_tz(geocoord, geocoord) RETURNS bool
   AS '/localstorage/z5290566/postgresql-15.1/src/tutorial/gcoord' LANGUAGE C IMMUTABLE STRICT;


CREATE OPERATOR = (
   leftarg = geocoord, rightarg = geocoord, procedure = geocoord_eq,
   commutator = = ,
   -- leave out negator since we didn't create <> operator
   -- negator = <> ,
   restrict = eqsel, join = eqjoinsel
);

CREATE OPERATOR > (
   leftarg = geocoord, rightarg = geocoord, procedure = geocoord_gt,
   commutator = < , negator = <= ,
   restrict = scalargtsel, join = scalargtjoinsel
);

CREATE OPERATOR < (
   leftarg = geocoord, rightarg = geocoord, procedure = geocoord_lt,
   commutator = > , negator = >= ,
   restrict = scalargtsel, join = scalargtjoinsel
);

CREATE OPERATOR <= (
   leftarg = geocoord, rightarg = geocoord, procedure = geocoord_le,
   commutator = >= , negator = > ,
   restrict = scalargtsel, join = scalargtjoinsel
);

CREATE OPERATOR >= (
   leftarg = geocoord, rightarg = geocoord, procedure = geocoord_ge,
   commutator = <= , negator = <,
   restrict = scalargtsel, join = scalargtjoinsel
);

CREATE OPERATOR ~ (
   leftarg = geocoord, rightarg = geocoord, procedure = geocoord_tz,
   commutator = ~,
   restrict = eqsel, join = eqjoinsel
);


-- create the support function too
CREATE FUNCTION geocoord_cmp(GeoCoord, GeoCoord) RETURNS int4
   AS '/localstorage/z5290566/postgresql-15.1/src/tutorial/gcoord' LANGUAGE C IMMUTABLE STRICT;


CREATE FUNCTION convert2dms(GeoCoord) RETURNS text
   AS '/localstorage/z5290566/postgresql-15.1/src/tutorial/gcoord' LANGUAGE C IMMUTABLE STRICT;

SELECT convert2dms('sydney,33.86°S,151.21°E');

-- now we can make the operator class
CREATE OPERATOR CLASS geocoord_op
    DEFAULT FOR TYPE geocoord USING btree AS
        OPERATOR        1       < ,
        OPERATOR        2       <= ,
        OPERATOR        3       = ,
        OPERATOR        4       >= ,
        OPERATOR        5       > ,
        OPERATOR        5       !~ ,
        FUNCTION        1       geocoord_cmp(GeoCoord, GeoCoord);


CREATE INDEX test_cplx_ind ON test_complex
   USING btree(a geocoord_op);








-- clean up the example
