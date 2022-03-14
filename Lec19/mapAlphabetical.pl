/* Maps arranged West to East as a,b,c,d,e,f */

/* Rules - often put later
 */
east(X,Y) :- e(X,Y).
east(X,Y) :- e(X,Z), east(Z,Y).
west(Y,X) :- east(X,Y).

/* Facts - often put earlier
 */
e(a,b).
e(b,c).
e(c,d).
e(d,e).
e(e,f).
