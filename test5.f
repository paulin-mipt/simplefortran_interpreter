PROGRAM MINUS_PLUS
INTEGER:: a
WRITE "Enter a number: "
READ a
IF (a.LT.0) THEN
	WRITE "The number is negative"
END IF
IF (a.GT.0) THEN
	WRITE "The number is positive"
END IF
IF (a.EQ.0) THEN
	WRITE "The number is zero"
END IF
END PROGRAM MINUS_PLUS



