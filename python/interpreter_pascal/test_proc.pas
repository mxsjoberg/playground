{ run this test program on https://www.onlinegdb.com/online_pascal_compiler }
PROGRAM testproc;
VAR
    a : INTEGER;

{ P1 }
PROCEDURE P1;
VAR
	a : REAL;
	k : INTEGER;

	{ P2 }
	PROCEDURE P2;
	VAR
		a, z : INTEGER;
	BEGIN
		z := 42;
	END;
	{ ./P2}

BEGIN
END;
{ ./P1 }

BEGIN
	a := 10;
END.