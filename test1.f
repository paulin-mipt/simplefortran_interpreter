PROGRAM MAIN
	INTEGER :: N
	WRITE "Enter a number: "
	READ N
	WRITE "Fib(N) = ", CALL FIB(N)
	WRITE "Fact(N) = ", CALL FACT(N)
END PROGRAM MAIN

FUNCTION FACT(N)
	FACT = 1
	DO 
		FACT = N * FACT
		N = N - 1
	WHILE(N > 0)
END FUNCTION FACT

FUNCTION FIB(N)
	INTEGER :: PREV, RESULT, SUM
	PREV = -1
	RESULT = 1
	DO
		SUM = RESULT + PREV
		PREV = RESULT
		RESULT = SUM
		N = N - 1
	WHILE(N >= 0)
	FIB = RESULT
END FUNCTION FIB