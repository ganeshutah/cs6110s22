
/*
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
We are going to encode all of this in Prolog
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
foo(p,x){
 var f,q;
 if (*p==0) {f=1;}
 else {
   q = alloc 0;
   *q = (*p)-1;
   f=(*p)*(x(q,x));
 }
 return f;
}

main() {
var n;
n = input;
return foo(&n,foo);
}
------------------------------------------------------------------
*/

/* 0            :       [0] = int */
 T_0=int,
/* 1            :       [1] = int */
 T_1=int,
/* *p           :       [p] = ^[*p] = ^int */
 T_p=ptr(T_starp),
/* *p=0         :       [*p] = [0] = int */
 T_starp=T_0,
/* f=1          :       [f] = [1] = int */
 T_f=T_1,
/* alloc 0              :       [alloc 0] = ^[0] = ^int */
 T_aloc0=ptr(T_0),
/* q=alloc 0    :       [q]=[alloc 0]=^int */
  T_q=T_aloc0,
/* *q           :       [q] = ^[*q] = ^int */
 T_q=ptr(T_starq),
/* (*p) - 1     :       [1] = [*p] = int = [(*p) - 1] */
 T_1=T_starp,  T_starpm1=T_starp,                                            
/* *q = (*p)-1  :       [*q] = [(*p) - 1] = int */
 T_starq=T_starpm1,
/* (*p)*(x(q,x))        :       [*p] = [x(q,x)] = int */
 T_starp= T_xqx,    
/* return f     :       [foo] = ([p],[x]) -> [f] = ([p],[x]) -> int */
    T_foo=ar3(T_p,T_x,T_f),
/*--- MAIN NOW ---*/    
/* n_m = input  :       [n_m]=[input]=int */
    T_nm = T_input, T_input=int,
/* &n_m                 :       [&n_m] = ^[n_m] = ^int */
    T_andnm = ptr(T_nm),
/* foo(&n_m,foo)        :       [foo] = ([&n_m],[foo]) -> [foo(&n_m,foo)] */
    T_foo=ar3(T_andnm, T_foo, T_fooandnmfoo).

T_0=int, T_1=int,T_p=ptr(T_starp), T_starp=T_0,T_f=T_1,T_aloc0=ptr(T_0),T_q=T_aloc0,T_q=ptr(T_starq),T_1=T_starp,T_starpm1=T_starp,T_starq=T_starpm1,T_starp=T_xqx,T_foo=ar3(T_p,T_x,T_f),T_nm=T_input, T_input=int, T_andnm=ptr(T_nm), T_foo=ar3(T_andnm,T_foo,T_fooandnmfoo).
    
 

