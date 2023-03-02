; Authored by Surya Mantha, CS 611, Fall 1989

(verify '(SPEC
	  ((X = a) and (Y = b))
	  (SEQ (ASSIGN R X) (ASSIGN X Y) (ASSIGN Y R))
	  ((X = b) and (Y = a))))

(verify '(SPEC
	  T
	  (BLOCK
	   (ASSIGN R X)
	   (ASSIGN Q 0)
	   (ASSERT (( R = X) and (Q = 0)))
	   (WHILE (Y <= R)
		  (ASSERT (X = (R + (Y * Q))))
		  (BLOCK (ASSIGN R (R - Y))
			 (ASSIGN Q (Q + 1)))))
	  ((R < Y) and (X = (R + (Y * Q))))))

(verify '(SPEC 
	  T 
	  (BLOCK
	   (ASSIGN X 1)
	   (ASSERT (X = 1))
	   (IF2 (X = 1)
		(ASSIGN M 0)
		(ASSIGN M 1)))
	  (M = 0)))

; buggy
(verify '(SPEC 
	  T 
	  (BLOCK
	   (ASSIGN X 1)
	   (ASSERT (X = 1))
	   (IF2 (X = 1)
		(ASSIGN M 0)
		(ASSIGN M 1)))
	  (M = 1)))


(verify '(SPEC 
	  (M >= 1)
	   (SEQ (ASSIGN X 0)
		(ASSERT ( X = 0))
		(FOR N 1 M (ASSERT (X = (((N - 1) * N) DIV 2)))
		     (ASSIGN X (X + N))))
	   (X = ((M * (M + 1)) DIV 2))))


		      







