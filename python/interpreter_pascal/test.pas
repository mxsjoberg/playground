{ run this test program on https://www.onlinegdb.com/online_pascal_compiler }
PROGRAM test;
VAR
    number     : INTEGER;
    a, b       : INTEGER;
    y          : REAL;

BEGIN { comment }
    BEGIN
        number := 2;
        a := number;
        b := 10 * a + 10 * number DIV 4;
    END;
    y := 20 / 7 + 3.14;
END. { comment }