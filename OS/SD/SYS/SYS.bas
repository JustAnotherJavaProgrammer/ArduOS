REM draw logo
DRAW IMAGE 20, 78, "ARDUOS.BMP"
REM Testing commands
PRINT "Hello BASIC!"
PRINT "3-4-2 should be -3 and is ", 3-4-2
PRINT "16/8/2 = 1 BUT ", 16/8/2
PRINT "32 times two plus six equals ", 32  *   2 + 6
PRINT "Value of A is: ", A
LET A = A+40
PRINT "Added 40; A=", A
LET A = 40 + 2
PRINT "A is now 40+2=A=", A
IF A=42 THEN PRINT "A is 42"
IF A=0 THEN PRINT "A is 0?!"
GOSUB (A)
PRINT "A is now ", A
GOSUB (A)
PRINT "RETURNed from second GOSUB, should draw single pixel now"
DRAW FRECT 100, 10, 69, 128, 31712
END
42 PRINT "GOSUB (A) was successfully evaluated!"
(A) PRINT "Hit me Baby one more time!"
LET A = A+1
RETURN