PROGRAM MAIN
	INTEGER:: m, n, result
	WRITE "Enter two numbers: "
	READ m
	READ n
	result = CALL GCD(m,n)
	WRITE "GCD of", m, "and", n, "is", result
END PROGRAM MAIN

FUNCTION GCD(m,n)
	INTEGER:: t
	DO
		IF (m < n) THEN
			t = m
			m = n
			n = t
		END IF
		t = m - n
		m = n
		n = t
	WHILE (n > 0)
	GCD = m
END FUNCTION GCD

