; Authored by Surya Mantha, CS 611, Fall 1989

; From mantha@cs Sat Dec  9 18:56:23 1989
; Date: Sat, 9 Dec 89 12:21:52 -0700
; From: mantha@cs (Surya M Mantha)
; To: ganesh@cs
; 
;                            Common Lisp
; 
;                     Part No. 98678   Rev. 2.01
; (c) Copyright 1986, Hewlett-Packard Company.  All rights reserved.
; 
; HP Common Lisp, 19-Oct-89
; 
; (load "/n/cs/u/mantha/thm-prover/p1.l")
; "/n/cs/u/mantha/thm-prover/p1.l"
; (load "/n/cs/u/mantha/thm-prover/v1.l")
; "/n/cs/u/mantha/thm-prover/v1.l"
; 
; Proofs for the partial correctness specifications at the end of file 
; "v1.l".
; 
; Checking syntax of annotated program ....... 
; OK.
; Assignment Axiom
;   T1:  |- {((X = A) AND (Y = B))} (ASSIGN R X) {((Y = B) AND (R = A))} 
; 
; By Sequencing  rule 
;   T2:  T1 |- {((X = A) AND (Y = B))} (SEQ (ASSIGN R X) (ASSIGN X Y) (ASSIGN Y R)) {((X = B) AND (Y = A))} 
; 
; all proved.
; T
; 
; 
; 
; Checking syntax of annotated program ....... 
; OK.
; Assignment Axiom
;   T3:  |- {T} (ASSIGN R X) {((R = X) AND (0 = 0))} 
; 
; Assignment Axiom
;   T4:  |- {((X = (R + (Y * Q))) AND (Y <= R))} (ASSIGN R (R - Y)) {(X = (R + (Y * (Q + 1))))} 
; 
; By Block  rule 
;   T5:  T4 |- {((X = (R + (Y * Q))) AND (Y <= R))} (BLOCK (ASSIGN R (R - Y)) (ASSIGN Q (Q + 1))) {(X = (R + (Y * Q)))} 
; 
; By While  rule 
;   T6:  T5 |- {((R = X) AND (Q = 0))} (WHILE (Y <= R) (ASSERT (X = (R + (Y * Q)))) (BLOCK (ASSIGN R (R - Y)) (ASSIGN Q (Q + 1)))) {((R < Y) AND (X = (R + (Y * Q))))} 
; 
; By Block  rule 
;   T7:  T3 T6 |- {T} (BLOCK (ASSIGN R X) (ASSIGN Q 0) (ASSERT ((R = X) AND (Q = 0))) (WHILE (Y <= R) (ASSERT (X = (R + (Y * Q)))) (BLOCK (ASSIGN R (R - Y)) (ASSIGN Q (Q + 1))))) {((R < Y) AND (X = (R + (Y * Q))))} 
; 
; all proved.
; T
; 
; 
; 
; Checking syntax of annotated program ....... 
; OK.
; Assignment Axiom
;   T8:  |- {T} (ASSIGN X 1) {(X = 1)} 
; 
; Assignment Axiom
;   T9:  |- {((X = 1) AND (X = 1))} (ASSIGN M 0) {(M = 0)} 
; 
; Assignment Axiom
;   T10:  |- {((X = 1) AND (NOT (X = 1)))} (ASSIGN M 1) {(M = 0)} 
; 
; By If-then-else rule
;   T11:  T9 T10 |- {(X = 1)} (IF2 (X = 1) (ASSIGN M 0) (ASSIGN M 1)) {(M = 0)} 
; 
; By Block  rule 
;   T12:  T8 T11 |- {T} (BLOCK (ASSIGN X 1) (ASSERT (X = 1)) (IF2 (X = 1) (ASSIGN M 0) (ASSIGN M 1))) {(M = 0)} 
; 
; all proved.
; T
; 
; 
; "The program containing the "for rule" needs more arithmetical facts
; to be proved correct"
; 
; 
; Checking syntax of annotated program ....... 
; OK.
; Assignment Axiom
;   T13:  |- {(M >= 1)} (ASSIGN X 0) {(X = 0)} 
; 
; Assignment Axiom
;   T14:  |- {((X = (((N - 1) * N) DIV 2)) AND ((1 <= N) AND (N <= M)))} (ASSIGN X (X + N)) {(X = ((((N + 1) - 1) * (N + 1)) DIV 2))} 
; 
; can't prove: 
; 
; ERR
; 
; (((X = 0) IMPLIES ((M < 1) IMPLIES (0 = (((M * M) + M) DIV 2)))))
; 
; NIL
; 
; 
; From mantha@cs Sat Dec  9 18:57:55 1989
; Date: Sat, 9 Dec 89 12:19:19 -0700
; From: mantha@cs (Surya M Mantha)
; To: ganesh@cs

;; This is a Common Lisp implementation of the theorem prover in Gordon's book
;; the following code will generate the pattern matcher

(mapcar
 #'(lambda(x) (setf (get x 'constant ) 't)) 
 '(+ - =  < <= >= * T F DIV not and or implies))

;; testing whether something is a variable 
(defmacro is-variable (x)
  `(not (or (null ,x) (numberp ,x) (get ,x 'constant))))

;;matchfn is a function that matches a pattern against an
;; an expression in the context of a substitution.
   
(defun matchfn (pat exp sub)
  (if (atom pat)
    (if (is-variable pat)
      (if (assoc pat sub)
	(if (equal (cdr (assoc pat sub)) exp)
	  sub
	  (throw 'fail 'FAIL))
	(cons (cons pat exp) sub))
      (if (eq pat exp) sub (throw 'fail 'FAIL)))
    (if (atom exp)
      (throw 'fail 'FAIL)
      (matchfn
       (rest pat)
       (rest exp)
       (matchfn (first pat) (first exp) sub)))))


;; match can simply be defined to call matchfn with an
;; empty initial substituition;
;; it must catch any throws from matchfn

(defun match (pat exp) (catch  'fail
			 (matchfn pat exp nil)))

;; function rewrite
(defvar rewrite-flag)
(setq rewrite-flag nil)

(defun rewrite1 (eqn exp)
  (let ((l (first eqn))
	(r (third eqn)))
    (let ((sub (match l exp)))
      (if (eq sub 'fail)
	exp
       (prog2
	(cond (rewrite-flag
               (write-string "the current expression and equation are ")
	       (terpri)
               (write exp)
	       (terpri)
	       (write (sublis sub eqn))
	       (terpri)(terpri))
	      (T nil))
	(car (sublis sub (list r)))))))) 

;;; Takes a list of equations and tries each equation sequentially

(defun rewrite (eqns exp)
  (if (null eqns)
    exp
    (rewrite (rest eqns) (rewrite1 (first eqns) exp))))

;;; Macro that checks if its argument is an "implication"

(defmacro is-imp (x)
  `(and (listp ,x)
	(eq (length ,x) 3)
	(eq (second ,x) 'implies)))

(defmacro mk-imp (p q) `(list ,p (quote implies) ,q))

(defmacro antecedent (x) `(first ,x))

(defmacro consequent (x) `(third ,x))

;; the function imp-subst-simp defined below transforms any S-expression
;; of the form (P implies Q) to one of (P implies Q[T/P])

(defun imp-subst-simp(exp)
  (if (is-imp exp)
    (let ((a (antecedent exp))
	  (c (consequent exp))
	  )
      (mk-imp a (subst 'T a c)))
    exp))


(defmacro is-eqn (x)
  `(and (listp ,x) (eq (length ,x) 3) (eq (second ,x) '=)))

(defun imp-and-simp(exp)
  (if (and (is-imp exp) (is-eqn (antecedent exp)))
    (let ((a (antecedent exp))
	  (c (consequent exp))
	  )
      (mk-imp a (subst (third a) (first a) c )))
    exp))


;; The function imp-simp first applies imp-subst-simp and then applies
;; imp-and-simp to the result

(defun imp-simp (exp)
  (imp-and-simp (imp-subst-simp exp)))

;; Higher Order Rewriting functions

;; Higher Order functions repeat, depth-conv, top-depth-conv and 
;; re-depth-conv are based on Paulson's paper
;; These operators specify the order in which subexpressions are rewritten

(defun repeat (f exp)
  (let ((exp1 (funcall f exp)))
    (if (equal exp exp1)
      exp
      (repeat f exp1))))

;; function depth-conv repeatedly applies a function f to all 
;; subexpressions of an expression exp in a bottomup order.

(defun depth-conv (f exp)
  (if (atom exp)
    (repeat f exp)
    (repeat
     f
     (cons (depth-conv f (first exp))
	   (depth-conv f (rest exp))))))


;;Now use depth-conv to define a fucntion depth-imp-simp that applies
;; imp-simp to all subexpressions of an expression

(defun depth-imp-simp (exp)
  (depth-conv (function (lambda (x) (imp-simp x)))
	      exp))

;; top-depth-conv that is like depth-conv except it rewrites
;; top down before rewriting bottom up;

(defun top-depth-conv (f exp)
  (let ((exp1 (repeat f exp)))
    (if (atom exp1)
      exp1
      (repeat
       f
	(cons (top-depth-conv f (first exp1))
	      (top-depth-conv f (rest exp1)))))))


;; Using top-depth-conv define a function for repeatedly rewriting
;; all subexpressions of an expression in top-down order using a
;; supplied list of equations.
;; The topdown order is important

(defun top-depth-rewrite (eqns exp)
  (top-depth-conv
   (function (lambda (x) (rewrite eqns x)))
   exp))


;; depth-rewrite is defined that uses depth-conv instead of top-depth-conv i.e.

(defun depth-rewrite (eqns exp)
  (depth-conv
   (function (lambda (x) (rewrite eqns x)))
  exp))

;; Yet another depth conversion routine is "re-depth-conv"

(defun re-depth-conv (f exp)
  (if (atom exp)
    (repeat f exp)
    (let ((exp1 (cons (re-depth-conv f (first exp))
		      (re-depth-conv f (rest exp)))))
      (let ((exp2 (funcall f exp1)))
		  (if (equal exp1 exp2) exp1 (re-depth-conv f exp2))))))

;;The corresponding rewriting function is

(defun re-depth-rewrite (eqns exp)
  (re-depth-conv 
   (function (lambda (x) (rewrite eqns x)))
   exp))


;;; The simple theorm prover can now be assembled by defining a function
;;; prove that repeatedly applies depth-imp-simp followed by rewriting
;;; using top-depth-rewrite

(defvar *debug*)

(setq *debug* T)

(defun prove (eqns exp)
  (if *debug*
       (format T "~% Trying to prove ~S ~% .......... ~%" exp))
       
  (repeat
   (function 
    (lambda (x) (top-depth-rewrite eqns (depth-imp-simp x))))
   exp))

;; The equations used for rewriting will be structured into two list:
;; A list called LOGIC containing various properties of IMPLIES and AND
;; A list called ARITHMETIC containing various arithmetical facts

(defvar logic)

(setq 
 logic
 `(
   ((T implies X) = X)
   ((F implies X) = T)
   ((X implies T) = T)
   ((X implies X) = T)
   ((T and X) = X)
   ((X and T) = X)
   ((F and X) = F)
   ((X and F) = F)
   ((X = X) = T)
   (((X and (not X)) implies Z) = T)
   (((X and Y) implies Z) = (X implies (Y implies Z)))
   ))

;; these are the arithmetic facts. The order is important

(defvar arithmetic)
(setq arithmetic
  '(
    ((x >= x) = T)  ;;; added to take care of "if"
    ((x + 0) = x)
    ((0 + x) = x)
    ((x * 0) = 0)
    ((0 * x) = 0)
    ((x * 1) = x)
    ((1 * x) = x)
    ((not(x <= y)) = (y < x))
    ((not(x >= y)) = (y > x))
    ((not(x < y)) = (x >= y))
    (((- x) >= (- y)) = (x <= y))
    (((- x) >= y) = (x <= (- y)))
    ((- 0) = 0)
    (((x < y) implies (x <= y)) = t)
    ((x - x) = 0)
    (((x + y) - z) = (x + (y - z)))
    (((x - y) * z) = ((x * z) - (y * z)))
    ((x * (y + z)) = ((x * y) + (x * z)))
    (((x + y) * z) = ((x * z) + (y * z)))
    (((x >= y) implies ((x < y) implies z)) = t)
    (((x <= y) implies ((y < x) implies z)) = t)
    ((0 div x) = 0)
    (((x div y) + z) = ((x + (y * z)) div y))
    (((x - y) + z) = (x + (z - y)))
    ((2 * x) = (x + x))
    ))


;;The list of facts is the defined by 
(defvar facts)

(setq facts (append logic arithmetic))
;; an example of somthing that cannot be proved is


;;(prove facts '((( t and (x >= y)) implies ( x = (max x y)))
;;	       and
;;	       (( t and (not (x >= y))) implies (y = (max x y)))))

;; it needs axioms about "max"

;;; The following can be proved in the current implementation

;;(time (prove facts '((( x = ((( n - 1) * n) div 2)) and (( 1 <= n) and ( n <= m)))
;;       implies
;;       (( x + n) = (((( n + 1) - 1) * (n + 1)) div 2 )))))

 




;From mantha@cs Sat Dec  9 18:58:14 1989
;Date: Sat, 9 Dec 89 12:19:23 -0700
;From: mantha@cs (Surya M Mantha)
;To: ganesh@cs

;; This program does the following
;; It checks the syntactic well formedness of the annotated specs
;; Then it generates the Verification Conditions using the technqiues
;; described in chapter 3 of gordon's book
;; Then it invokes the theorem prover and tries to prove the VCs
;; it either terminates successfully or complains which VC's it could
;; not prove.


(defvar *theorem-count* 1)

;;SELECTOR MACROS
;; The following macros extract the componets of a spec (SPEC P C Q)

(defmacro precondition  (x) `(second ,x))
(defmacro command  (x) `(third ,x))
(defmacro postcondition  (x) `(fourth ,x))

;;The macro command-type gets the construtor from a command'
;; used in case-switches in the definition of chk-and-cmd
;; assigned-vars adn vc-gen

(defmacro command-type (c) `(first ,c))

;; Next two macros extract the lhs and rhs of an assignment statement

(defmacro lhs (c) `(second ,c))
(defmacro rhs (c) `(third ,c))

;;The next three macros get the list of commands in a sequence, the
;; components (i.e. local variable declarations and commands) of a block
;; and the name declared in a local variable declaration

(defmacro seq-commands (c) `(rest ,c))
(defmacro block-body (c) `(rest ,c))
(defmacro var-name (v) `(second ,v))

;;The following three macros get the components of conditionals

(defmacro if-test (c) `(second ,c))
(defmacro then-part (c) `(third ,c))
(defmacro else-part (c) `(fourth ,c))

;;the next three macros get the components of an annotated WHILE command

(defmacro while-test (c) `(second ,c))
(defmacro while-annotation (c) `(third ,c))
(defmacro while-body (c) `(fourth ,c))

;;These five macros get the components of an annotated FOR command

(defmacro for-var (c) `(second ,c))
(defmacro lower (c) `(third ,c))
(defmacro upper (c) `(fourth ,c))
(defmacro for-annotation (c) `(fifth ,c))
(defmacro for-body (c) `(sixth ,c))

;;Finally a macro to get the statement contained in an annotation (ASSERT S)

(defmacro statement (a) `(second ,a))

;;CONSTRUCTOR MACROS

;;Now we will define macros for constructing partial correctness specs
;; negated statements,implications and conjunctions

(defmacro mk-spec (p c q) `(list (quote SPEC) ,p ,c ,q))
(defmacro mk-not (s) `(list (quote not) ,s))
(defmacro mk-imp (s1 s2) `(list ,s1 (quote implies) ,s2))
(defmacro mk-and (s1 s2) `(list ,s1 (quote and) ,s2))

;;Three macros are defined to construct arithmetic expressions of the form
;; m+n, m<n, and m<= n;

(defmacro mk-add (m n) `( list ,m (quote +) , n))
(defmacro mk-less (m n) `( list ,m (quote <) , n))
(defmacro mk-less-eq (m n) `( list ,m (quote <=) , n))

;;TEST MACROS

;; is-var is a predicate that tests whether something represents a local
;; variable declaration i.e. has the form (VAR x) where x is a symbol

(defmacro is-var (v)
  `(and ,v
	(eq (first ,v) 'VAR)
	(rest ,v)
	(symbolp (second ,v)))) ; secondp?



;; the function is-assign is a predicate to test whether a command is an
;; assignment i.e. has the form (ASSIGN S1 S2)

(defmacro is-assign (a)
  `(and ,a
	(eq (first ,a) 'ASSIGN)
	(rest ,a)
	(rest (rest ,a))))

;;ERROR CHECKING FUNCTIONS

(defvar culprit)

(defun error_fun (message thing)
  (progn
    (princ message)
    (terpri)
    (setq culprit thing)
    ))

(defmacro add1 (x) 
  `(+ ,x 1))

;;Checking wellformedness

;;The function chk-type checks whether the first element of something
;; has a given name; t is returned if it does and an error if not

(defun chk-typ (name thing msg)
  (if (or (null thing) (not (eq (first thing) name)))
    (error_fun msg thing)
    t))

;;Evaluating chk-parts checks that m=n if not raises the error
;;with the culprit constructor

(defun chk-parts (thing size)
  (or (eq (length thing) (add1 size))
	  (error_fun
	   "Syntax error: Vanish"
	   thing)))

;; The macro chk-sym checks whether something is a LISP symbol:

(defmacro chk-sym (v msg) `(or (symbolp ,v) (error_fun ,msg ,v)))

;;The function chk-rev-ann-seq checks whether a sequence is correctly
;; annotated.  It is given its argument in reverse order by chk-ann-cmd
;; this makes recursion easier.

(defun chk-rev-ann-seq (c-list)
  (cond ((null c-list) t)
	((null (rest c-list)) (chk-ann-cmd (first c-list)))
	((is-assign (first c-list))
	 (chk-rev-ann-seq (rest c-list)))
	(t
	 (chk-ann-cmd (first c-list))
	 (chk-assert (second c-list) "Bad annotation in SEQ")
	 (chk-rev-ann-seq (rest (rest c-list))))))

;;The fucntion chk-ann-nlock checks whether the sequence of commands in a
;; block is properly annotated.  It uses the function block-commands that
;; gets the sequence of commands in a block;

(defun chk-ann-block (c)
  (chk-rev-ann-seq (reverse (block-commands c))))

(defun block-commands (c)
  (strip-locals (block-body c)))

(defun strip-locals (c-list)
  (if (is-var (first c-list))
    (strip-locals (rest c-list))
    c-list))

;;The next two macros are intended to check whether statements and expressions
;; are well-formed. These checks have not beenimplemented in this version of
;; the verifier, so the macros just expand to t

(defun chk-stat (s msg) t)
(defun chk-exp (e msg) t)

;; The function chk-assert checks whether something is an annotation
;; and reaises an error otherwise;

(defun chk-assert (s msg)
  (and (chk-typ `ASSERT s msg)
       (chk-parts s 1)))


;; Here is the defnition of the function chk-ann-cmd that checks whether 
;; a command is properly annotated.  This function is just a cases-switch
;;on th command type

(defun chk-ann-cmd (c)
  (case (command-type c)
    (ASSIGN (and (chk-parts c 2)
		 (chk-sym (lhs c) "Bad lhs of ASSIGN")
		 (chk-exp (rhs c) "Bad rhs of ASSIGN")
		 ))
    (IF1  (and (chk-parts c 2)
	       (chk-stat (if-test c) "BAD test in IF1")
	       (chk-ann-cmd (then-part c))))
    (IF2  (and (chk-parts c 3)
	       (chk-stat (if-test c) "BAD test in IF2")
	       (chk-ann-cmd (then-part c))
	       (chk-ann-cmd (else-part c))))
    (WHILE (and (chk-parts c 3)
		(chk-stat
		 (while-test c)
		 "BAD test in  WHILE ")
		(chk-assert
		 (while-annotation c)
		 "BAD annotation in WHILE")
		 (chk-ann-cmd (while-body c))))
    (FOR (and (chk-parts c 5)
	      (chk-sym (for-var c) "BAD FOR variable")
	      (chk-exp (lower c) "BAD lower BOUND")
	      (chk-exp (upper c) "BAD upper BOUND")
	      (chk-assert
	       (for-annotation c)
	       "BAD annotation in FOR")
	      (chk-ann-cmd (for-body c))))
    (SEQ (chk-rev-ann-seq (reverse (seq-commands c))))
    (BLOCK (chk-ann-block c))
    (t   (error_fun
	  "UNKNOWN command type"
	  (command-type c)))))
     
;;The function chk-ann-spec checks whether something is a correctly
;; annotated partial correctness spec and raises an error if it isn't

(defun chk-ann-spec (a)
  (and (chk-typ 'SPEC a "Bad Specification constructor")
       (chk-parts a 3)
       (chk-stat (precondition a) "Bad precondition")
       (chk-ann-cmd (command a))
       (chk-stat (postcondition a) "Bad postcondition")))

;;Checking Side conditions

(defun get-locals (c-list)
  (if (is-var (first c-list))
    (cons (var-name (first c-list))
	  (get-locals (rest c-list)))
    nil))

;;the function vars-in computes the variables in a term

(defun vars-in (e)
  (cond ((null e) nil)
	((atom e) (list e))
	(t (append (vars-in (first e)) (vars-in (rest e))))))

;;the function append list appends togetheers the members of a
;;alist of lists

(defun  append-lists (l)
  (if (null l)
    (l
     (append (first l) (append-lists (rest l))))))

;; the function assigned-vars computes a list of the variables
;; in a command.
;; It is used by chk-for-side-condition

(defun assigned-vars (c)
  (case (command-type c)
    (ASSIGN (list (lhs c)))
    (IF1 (assigned-vars (then-part c)))
    (IF2 (append (assigned-vars (then-part c))
		 (assigned-vars (else-part c))))
    (WHILE (assigned-vars (while-body c)))
    (FOR (assigned-vars (for-body c)))
    (SEQ
     (append-lists
      (mapcar (function assigned-vars)
	      (seq-commands c))))
    (BLOCK
     (append-lists
      (mapcar (function assigned-vars)
	      (block-commands c))))
    (t nil)))

;; The defintion of the function disjoint below depends on
;; the arguments of "and" and "or" being evaluated in left to right
;; order

(defun disjoint (x y)
  (or (null x)
      (and (not(member (first x) y))
	   (disjoint (rest x) y))))

;;evaluating (chk-block-side-condition P C Q) checks that if
;; C represents a block i.e. has the form (BLOCK (VAR V1) .. C1 .. Cn)

(defun chk-block-side-condition (p c q)
  (let ((p-vars (vars-in p))
	(c-vars (get-locals (block-body c)))
	(q-vars (vars-in q)))
    (or (disjoint c-vars (append p-vars q-vars))
	(error_fun "Side Condition of BLOCK violated" c-vars)
	)))

;;evaluating (chk-for-side-condtion (FOR V E1 E2 R C)) checks that
;;neither V, nor any variable occuring in E1 or E2 is assigned to 
;;inside C

(defun chk-for-side-condition (c)
  (let ((v (for-var c))
	(e1 (lower c))
	(e2 (upper c))
	(c1 (for-body c)))
    (or (disjoint 
	 (cons v (append (vars-in e1) (vars-in e2)))
	 (assigned-vars c1))
	(error_fun "Side condition in FOR violated" c1))))

;;THE VERIFICATION CONDITION GENERATOR

;;Evaluating (vc-gen P C Q) returns the verification conditions from
;;an annotated spec (SPEC P C Q )
;; It calls assign-vc-gen, if1-vc-gen, if2-vc-gen, while-vc-gen
;; for-vc-gen, seq-vc-gen and block-vc-gen

(defun vc-gen (p c q)
  (case (command-type c)
    (ASSIGN (assign-vc-gen p c q))
    (IF1 (if1-vc-gen p c q))
    (IF2 (if2-vc-gen p c q))
    (WHILE (while-vc-gen p c q))
    (FOR (for-vc-gen p c q))
    (SEQ (seq-vc-gen p c q))
    (BLOCK (block-vc-gen p c q))))

;;the function assign-vc-gen generates a list of verification 
;;conditions in the standard way


(defun assign-vc-gen (p c q)
  (let* (( v (lhs c))
	 (e (rhs c))
	 (vcs (list (mk-imp p (subst e v q))))
	 (unproved (unproved-vcs (prove facts  vcs))))
    (cond ((null unproved)
	   (let ((num (concatenate 'string (string 'T) (princ-to-string *theorem-count*))))
             (setq *theorem-count*  (1+ *theorem-count*))
	     (print-thm "Assignment Axiom" num  nil (list p c q))
	     (list num)))
	  (T (throw 'unproved (list 'error-proof-aborted unproved))))))


;; the function IF1-VC-GEN generates a list of verification conditions for
;; the if rule
(defun if1-vc-gen ( p c q)
  (let* (( s (if-test c))
	 (c1 (then-part c))
	 (vcs  (mk-imp (mk-and p (mk-not s)) q))
	 (vcs1 (vc-gen (mk-and p s) c1 q))
	 (unproved (unproved-vcs (prove facts vcs))))
    (cond ((null unproved)
	   (let ((num (concatenate 'string (string 'T) (princ-to-string *theorem-count*))))
             (setq *theorem-count*  (1+ *theorem-count*))
	     (print-thm "By If-then rule " num vcs1 (list p c q))
	     (list num)))
	  (T (throw 'unproved (list 'error-proof-aborted unproved))))))


;; the function IF2-VC-GEN generates a list of verification conditions for
;; the if-then-else rule

(defun if2-vc-gen (p c q)
  (let* (( s (if-test c))
	 (c1 (then-part c))
	 (c2 (else-part c))
	 (vcs1 (vc-gen (mk-and p s) c1 q))
	 (vcs2 (vc-gen (mk-and p (mk-not s)) c2 q)))
    (let ((num (concatenate 'string (string 'T) (princ-to-string *theorem-count*))))
      (setq *theorem-count*  (1+ *theorem-count*))
      (print-thm "By If-then-else rule" num (append vcs1 vcs2) (list p c q))
      (list num))))
	 


;; the function while-VC-GEN generates a list of verification conditions for
;; the while rule

(defun while-vc-gen (p c q)
  (let* ((s (while-test c))
	(r (statement (while-annotation c)))
	(c1 (while-body c))
	(vcs (cons
	      (mk-imp p r)
	      (list (mk-imp (mk-and r (mk-not s)) q))))
	(vcs1  (vc-gen (mk-and r s ) c1 r))
	(unproved (unproved-vcs (prove facts vcs))))
    (cond ((null unproved)
	   (let ((num (concatenate 'string (string 'T) (princ-to-string *theorem-count*))))
	     (setq *theorem-count*  (1+ *theorem-count*))
	     (print-thm "By While  rule " num vcs1 (list p c q))
	     (list num)))
	  (T (throw 'unproved (list 'error-proof-aborted unproved))))))

;; the function for-vc-gen lists vc for the "for rule"
;; the syntactic side condition is checked by chk-for-side-condition

(defun for-vc-gen(p c q)
  (let* ((v (for-var c))
	(e1 (lower c))
	(e2 (upper c))
	(r (statement (for-annotation c)))
	(c1 (for-body c))
	(syntactic-check (chk-for-side-condition c)))
    (if syntactic-check 
      (let* 
	  ((vcs (list
		  (mk-imp p (subst e1 v r))
		  (mk-imp (subst (mk-add e2 1) v r) q)
		  (mk-imp (mk-and p (mk-less e2 e1)) q)))
	   (vcs1 (vc-gen
		  (mk-and
		   r
		   (mk-and
		    (mk-less-eq e1 v)
		    (mk-less-eq v e2)))
		  c1
		  (subst (mk-add v 1) v r)))
	   (unproved (unproved-vcs (prove facts vcs))))
	(cond ((null unproved)
	       (let ((num (concatenate 'string (string 'T) (princ-to-string *theorem-count*))))
		 (setq *theorem-count*  (1+ *theorem-count*))
		 (print-thm "By For  rule " num vcs1 (list p c q))
		 (list num)))
	  (T (throw 'unproved (list 'error-proof-aborted unproved)))))
      (error_fun "syntactic error" (list p c q)))))


(defun print-thm (str th-num dep-list thm)
  (format T "~A~%  ~A: " str (make-symbol th-num))
  (dolist (x dep-list) 
    (format T " ~A" x))
  (format T " |- ")
  (format T "{~S} ~S {~S} ~%~%" (first thm) (second thm) (third thm)))
  


(defun seq-vc-gen(p c q)
  (let ((vcs (rev-seq-vc-gen p (reverse (seq-commands c)) q))
	(num (concatenate 'string (string 'T) (princ-to-string *theorem-count*))))
    (setq *theorem-count*  (1+ *theorem-count*))
    (print-thm "By Sequencing  rule " num vcs (list p c q))
    (list num)))


(defun rev-seq-vc-gen (p c-list q)
  (cond ((null c-list)
	 (error_fun "Empty command list" (list p c-list q)))
	((null (rest c-list)) (vc-gen p (first c-list) q))
	((is-assign (first c-list))
	 (let ((v (lhs (first c-list)))
	       (e (rhs (first c-list))))
	   (rev-seq-vc-gen p (rest c-list) (subst e v q))))
	(t
	 (let ((cn (first c-list))
	       (r (statement(second c-list))))
	   (append (rev-seq-vc-gen p (rest(rest c-list)) r)
		   (vc-gen r cn q))))))

;; The block vc gen does the same after checking for the
;; side conditions

(defun block-vc-gen(p c q)
  (if (chk-block-side-condition p c q)
    (let* ((vcs (rev-seq-vc-gen p (reverse (block-commands c)) q))
	   (num (concatenate 'string (string 'T) (princ-to-string *theorem-count*))))
      (setq *theorem-count*  (1+ *theorem-count*))
      (print-thm "By Block  rule " num vcs (list p c q))
      (list num))
    (error_fun "side condition failed" (list p c q))))


;; The complete verifier
;;
;; The function verify takes an annotated specification and
;; (i)  checks that is is annotated correctly using chk-ann-spec;
;; (ii) generates the verification conditions using vc-gen,
;; (iii)prints them out;
;; (iv)attempts to prove them with the theorem prover prove described
;;     in prover.l using the global facts
;;
;; (v) prints out ALL PROVED if it succeeds in proving all the verification
;;     condition, otherwise it prints out the things it cannot prove;
;; (vi) returns t if all the verifications are proved and nil otherwise"

;;Here are some auxillary functions

(defun print-list (list)
  (mapcar
   (function (lambda(x) (terpri) (pprint x)))
   list))

(defun unproved-vcs (l)
  (cond ((null l) nil)
	((eq (first l) 'T) (unproved-vcs (rest l)))
	(t (cons (first l) (unproved-vcs (rest l))))))


;;The top level FUNCTION which invokes others

(defun verify (a)
  (prog (vcs)
	(terpri)
	(princ "Checking syntax of annotated program ....... ")
	(chk-ann-spec a)
	(terpri)
	(princ "OK.")
	(terpri)
	(setq vcs (catch 'unproved (vc-gen (precondition a)
					   (command a)
					   (postcondition a))))

	(cond ((and (listp vcs) (eql 'error-proof-aborted (first vcs)))
		(format T "can't prove: ~%")
		(print-list vcs)
		(terpri) (return 'nil))
	      (T (princ "all proved.") (return 'T)))))

;---------------------- end of file ----------------------

