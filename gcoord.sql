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

INSERT INTO test_complex VALUES (1, 'Melbourne,37.84°S 144.95°E');

SELECT * FROM test_complex;


-- clean up the example
DROP TABLE test_complex;
DROP TYPE geocoord CASCADE;
