PROGRAM NUMBER_COMPARATOR
	INTEGER:: n, m
	n = 10
	m = 100
	WRITE "The numbers to compare are: "
	WRITE "first =", n
	WRITE "second =", m
	IF (n < m) THEN
		WRITE "first < second"
	ELSE
		IF (n > m) THEN
			WRITE "first > second"
		ELSE
			WRITE "first = second"
		END IF
	END IF
END PROGRAM NUMBER_COMPARATOR
