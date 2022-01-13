/* Five dining philosophers  */
/* See run script at the end */

mtype = {are_you_free, yes, no, release}

byte progress;

proctype phil(chan lf, rf; int philno)
{
  do
    :: do 
         :: lf!are_you_free ->
            if 
              :: lf?yes -> break
              :: lf?no;
            fi
       od;
  
       do
         :: rf!are_you_free ->
            if
              :: rf?yes;
                 progress = 1; progress = 0;
                 lf!release -> 
                 rf!release;
                 break;
              :: rf?no -> lf!release;
                 break;
            fi
       od;
  od
}

proctype fork(chan lp, rp)
{
  do

   :: rp?are_you_free -> rp!yes ->

        do
        :: lp?are_you_free ->
           lp!no
        :: rp?release -> break
        od

   :: lp?are_you_free -> lp!yes ->

      do
        :: rp?are_you_free ->
           rp!no
        :: lp?release -> break
      od;
  od
}

init {

   chan c0 = [0] of { mtype };
   chan c1 = [0] of { mtype };
   chan c2 = [0] of { mtype };
   chan c3 = [0] of { mtype };
   chan c4 = [0] of { mtype };
   chan c5 = [0] of { mtype };

   atomic {
     run phil(c5, c0, 0);
     run fork(c0, c1);
     run phil(c1, c2, 1);
     run fork(c2, c3);
     run phil(c3, c4, 2);
     run fork(c4, c5);
   };
}

never { /* Negation of []<> progress */
 do
 :: skip
 :: (!progress) -> goto accept;
 od;
 accept: goto accept;
}

/*---
> origin/main
Removing 2022/Asgs/Asg1/soln/pan.tmp
Removing 2022/Asgs/Asg1/soln/pan.err
error: Terminal is dumb, but EDITOR unset
Not committing merge; use 'git commit' to complete the merge.
[ganesh@vpn-155-94-98-155 soln]$ spin -a PhilLL.pml 
[ganesh@vpn-155-94-98-155 soln]$ gcc -DMEMLIM=1024 -O2 -w -o pan pan.c
[ganesh@vpn-155-94-98-155 soln]$ ./pan -m 10000
Spin Version 6.4.5 -- 1 January 2016
Valid Options are:
	-a  find acceptance cycles
	-A  ignore assert() violations
	-b  consider it an error to exceed the depth-limit
	-cN stop at Nth error (defaults to -c1)
	-D  print state tables in dot-format and stop
	-d  print state tables and stop
	-e  create trails for all errors
	-E  ignore invalid end states
	-f  add weak fairness (to -a or -l)
	-hN use different hash-seed N:0..499 (defaults to -h0)
	-hash generate a random hash-polynomial for -h0 (see also -rhash)
	      using a seed set with -RSn (default 12345)
	-i  search for shortest path to error
	-I  like -i, but approximate and faster
	-J  reverse eval order of nested unlesses
	-l  find non-progress cycles -> disabled, requires compilation with -DNP
	-mN max depth N steps (default=10k)
	-n  no listing of unreached states
	-QN set time-limit on execution of N minutes
	-q  require empty chans in valid end states
	-r  read and execute trail - can add -v,-n,-PN,-g,-C
	-r trailfilename  read and execute trail in file
	-rN read and execute N-th error trail
	-C  read and execute trail - columnated output (can add -v,-n)
	-r -PN read and execute trail - restrict trail output to proc N
	-g  read and execute trail + msc gui support
	-S  silent replay: only user defined printfs show
	-RSn use randomization seed n
	-rhash use random hash-polynomial and randomly choose -p_rotateN, -p_permute, or p_reverse
	-T  create trail files in read-only mode
	-t_reverse  reverse order in which transitions are explored
	-tsuf replace .trail with .suf on trailfiles
	-V  print SPIN version number
	-v  verbose -- filenames in unreached state listing
	-wN hashtable of 2^N entries (defaults to -w24)
	-x  do not overwrite an existing trail file

	options -r, -C, -PN, -g, and -S can optionally be followed by
	a filename argument, as in '-r filename', naming the trailfile
[ganesh@vpn-155-94-98-155 soln]$ ./pan -a 
warning: for p.o. reduction to be valid the never claim must be stutter-invariant
(never claims generated from LTL formulae are stutter-invariant)
pan:1: acceptance cycle (at depth 367)
pan: wrote PhilLL.pml.trail

(Spin Version 6.4.5 -- 1 January 2016)
Warning: Search not completed
	+ Partial Order Reduction

Full statespace search for:
	never claim         	+ (never_0)
	assertion violations	+ (if within scope of claim)
	acceptance   cycles 	+ (fairness disabled)
	invalid end states	- (disabled by never claim)

State-vector 160 byte, depth reached 381, errors: 1
      132 states, stored
       26 states, matched
      158 transitions (= stored+matched)
        5 atomic steps
hash conflicts:         0 (resolved)

Stats on memory usage (in Megabytes):
    0.024	equivalent memory usage for states (stored*(State-vector + overhead))
    0.286	actual memory usage for states
  128.000	memory used for hash table (-w24)
    0.534	memory used for DFS stack (-m10000)
  128.730	total actual memory usage



pan: elapsed time 0.01 seconds
[ganesh@vpn-155-94-98-155 soln]$ spin -p -r -s -c -t PhilLL.pml
proc 0 = :init:
starting claim 3
using statement merging
Never claim moves to line 76	[(1)]
Starting phil with pid 2
proc 1 = phil
  2:	proc  0 (:init::1) PhilLL.pml:65 (state 1)	[(run phil(c5,c0,0))]
Starting fork with pid 3
proc 2 = fork
  3:	proc  0 (:init::1) PhilLL.pml:66 (state 2)	[(run fork(c0,c1))]
Starting phil with pid 4
proc 3 = phil
  4:	proc  0 (:init::1) PhilLL.pml:67 (state 3)	[(run phil(c1,c2,1))]
Starting fork with pid 5
proc 4 = fork
  5:	proc  0 (:init::1) PhilLL.pml:68 (state 4)	[(run fork(c2,c3))]
Starting phil with pid 6
proc 5 = phil
  6:	proc  0 (:init::1) PhilLL.pml:69 (state 5)	[(run phil(c3,c4,2))]
Starting fork with pid 7
proc 6 = fork
  7:	proc  0 (:init::1) PhilLL.pml:70 (state 6)	[(run fork(c4,c5))]
q\p   0   1   2   3   4   5   6
  4   .   .   .   .   .   lf!are_you_free
  9:	proc  5 (phil:1) PhilLL.pml:11 (state 1)	[lf!are_you_free]
  4   .   .   .   .   rp?are_you_free
 10:	proc  4 (fork:1) PhilLL.pml:37 (state 1)	[rp?are_you_free]
  4   .   .   .   .   rp!yes
 12:	proc  4 (fork:1) PhilLL.pml:37 (state 2)	[rp!yes]
  4   .   .   .   .   .   lf?yes
 13:	proc  5 (phil:1) PhilLL.pml:13 (state 2)	[lf?yes]
  5   .   .   .   .   .   rf!are_you_free
 15:	proc  5 (phil:1) PhilLL.pml:19 (state 10)	[rf!are_you_free]
  5   .   .   .   .   .   .   lp?are_you_free
 16:	proc  6 (fork:1) PhilLL.pml:45 (state 10)	[lp?are_you_free]
  5   .   .   .   .   .   .   lp!yes
 18:	proc  6 (fork:1) PhilLL.pml:45 (state 11)	[lp!yes]
  5   .   .   .   .   .   rf?yes
 19:	proc  5 (phil:1) PhilLL.pml:21 (state 11)	[rf?yes]
 21:	proc  5 (phil:1) PhilLL.pml:22 (state 12)	[progress = 1]
 23:	proc  5 (phil:1) PhilLL.pml:22 (state 13)	[progress = 0]
  4   .   .   .   .   .   lf!release
 25:	proc  5 (phil:1) PhilLL.pml:23 (state 14)	[lf!release]
  4   .   .   .   .   rp?release
 26:	proc  4 (fork:1) PhilLL.pml:42 (state 5)	[rp?release]
  2   .   .   .   lf!are_you_free
 28:	proc  3 (phil:1) PhilLL.pml:11 (state 1)	[lf!are_you_free]
  2   .   .   rp?are_you_free
 29:	proc  2 (fork:1) PhilLL.pml:37 (state 1)	[rp?are_you_free]
  5   .   .   .   .   .   rf!release
 31:	proc  5 (phil:1) PhilLL.pml:24 (state 15)	[rf!release]
  5   .   .   .   .   .   .   lp?release
 32:	proc  6 (fork:1) PhilLL.pml:50 (state 14)	[lp?release]
  4   .   .   .   .   .   lf!are_you_free
 34:	proc  5 (phil:1) PhilLL.pml:11 (state 1)	[lf!are_you_free]
  4   .   .   .   .   rp?are_you_free
 35:	proc  4 (fork:1) PhilLL.pml:37 (state 1)	[rp?are_you_free]
  4   .   .   .   .   rp!yes
 37:	proc  4 (fork:1) PhilLL.pml:37 (state 2)	[rp!yes]
  4   .   .   .   .   .   lf?yes
 38:	proc  5 (phil:1) PhilLL.pml:13 (state 2)	[lf?yes]
  5   .   .   .   .   .   rf!are_you_free
 40:	proc  5 (phil:1) PhilLL.pml:19 (state 10)	[rf!are_you_free]
  5   .   .   .   .   .   .   lp?are_you_free
 41:	proc  6 (fork:1) PhilLL.pml:45 (state 10)	[lp?are_you_free]
  5   .   .   .   .   .   .   lp!yes
 43:	proc  6 (fork:1) PhilLL.pml:45 (state 11)	[lp!yes]
  5   .   .   .   .   .   rf?yes
 44:	proc  5 (phil:1) PhilLL.pml:21 (state 11)	[rf?yes]
 46:	proc  5 (phil:1) PhilLL.pml:22 (state 12)	[progress = 1]
 48:	proc  5 (phil:1) PhilLL.pml:22 (state 13)	[progress = 0]
  2   .   .   rp!yes
 50:	proc  2 (fork:1) PhilLL.pml:37 (state 2)	[rp!yes]
  2   .   .   .   lf?yes
 51:	proc  3 (phil:1) PhilLL.pml:13 (state 2)	[lf?yes]
  4   .   .   .   .   .   lf!release
 53:	proc  5 (phil:1) PhilLL.pml:23 (state 14)	[lf!release]
  4   .   .   .   .   rp?release
 54:	proc  4 (fork:1) PhilLL.pml:42 (state 5)	[rp?release]
  5   .   .   .   .   .   rf!release
 56:	proc  5 (phil:1) PhilLL.pml:24 (state 15)	[rf!release]
  5   .   .   .   .   .   .   lp?release
 57:	proc  6 (fork:1) PhilLL.pml:50 (state 14)	[lp?release]
  4   .   .   .   .   .   lf!are_you_free
 59:	proc  5 (phil:1) PhilLL.pml:11 (state 1)	[lf!are_you_free]
  4   .   .   .   .   rp?are_you_free
 60:	proc  4 (fork:1) PhilLL.pml:37 (state 1)	[rp?are_you_free]
  4   .   .   .   .   rp!yes
 62:	proc  4 (fork:1) PhilLL.pml:37 (state 2)	[rp!yes]
  4   .   .   .   .   .   lf?yes
 63:	proc  5 (phil:1) PhilLL.pml:13 (state 2)	[lf?yes]
  5   .   .   .   .   .   rf!are_you_free
 65:	proc  5 (phil:1) PhilLL.pml:19 (state 10)	[rf!are_you_free]
  5   .   .   .   .   .   .   lp?are_you_free
 66:	proc  6 (fork:1) PhilLL.pml:45 (state 10)	[lp?are_you_free]
  5   .   .   .   .   .   .   lp!yes
 68:	proc  6 (fork:1) PhilLL.pml:45 (state 11)	[lp!yes]
  5   .   .   .   .   .   rf?yes
 69:	proc  5 (phil:1) PhilLL.pml:21 (state 11)	[rf?yes]
 71:	proc  5 (phil:1) PhilLL.pml:22 (state 12)	[progress = 1]
  3   .   .   .   rf!are_you_free
 73:	proc  3 (phil:1) PhilLL.pml:19 (state 10)	[rf!are_you_free]
  3   .   .   .   .   lp?are_you_free
 74:	proc  4 (fork:1) PhilLL.pml:40 (state 3)	[lp?are_you_free]
 76:	proc  5 (phil:1) PhilLL.pml:22 (state 13)	[progress = 0]
  3   .   .   .   .   lp!no
 78:	proc  4 (fork:1) PhilLL.pml:41 (state 4)	[lp!no]
  3   .   .   .   rf?no
 79:	proc  3 (phil:1) PhilLL.pml:26 (state 17)	[rf?no]
  4   .   .   .   .   .   lf!release
 81:	proc  5 (phil:1) PhilLL.pml:23 (state 14)	[lf!release]
  4   .   .   .   .   rp?release
 82:	proc  4 (fork:1) PhilLL.pml:42 (state 5)	[rp?release]
  5   .   .   .   .   .   rf!release
 84:	proc  5 (phil:1) PhilLL.pml:24 (state 15)	[rf!release]
  5   .   .   .   .   .   .   lp?release
 85:	proc  6 (fork:1) PhilLL.pml:50 (state 14)	[lp?release]
  4   .   .   .   .   .   lf!are_you_free
 87:	proc  5 (phil:1) PhilLL.pml:11 (state 1)	[lf!are_you_free]
  4   .   .   .   .   rp?are_you_free
 88:	proc  4 (fork:1) PhilLL.pml:37 (state 1)	[rp?are_you_free]
  4   .   .   .   .   rp!yes
 90:	proc  4 (fork:1) PhilLL.pml:37 (state 2)	[rp!yes]
  4   .   .   .   .   .   lf?yes
 91:	proc  5 (phil:1) PhilLL.pml:13 (state 2)	[lf?yes]
  5   .   .   .   .   .   rf!are_you_free
 93:	proc  5 (phil:1) PhilLL.pml:19 (state 10)	[rf!are_you_free]
  5   .   .   .   .   .   .   lp?are_you_free
 94:	proc  6 (fork:1) PhilLL.pml:45 (state 10)	[lp?are_you_free]
  5   .   .   .   .   .   .   lp!yes
 96:	proc  6 (fork:1) PhilLL.pml:45 (state 11)	[lp!yes]
  5   .   .   .   .   .   rf?yes
 97:	proc  5 (phil:1) PhilLL.pml:21 (state 11)	[rf?yes]
 99:	proc  5 (phil:1) PhilLL.pml:22 (state 12)	[progress = 1]
  6   .   lf!are_you_free
101:	proc  1 (phil:1) PhilLL.pml:11 (state 1)	[lf!are_you_free]
  6   .   .   .   .   .   .   rp?are_you_free
102:	proc  6 (fork:1) PhilLL.pml:48 (state 12)	[rp?are_you_free]
  6   .   .   .   .   .   .   rp!no
104:	proc  6 (fork:1) PhilLL.pml:49 (state 13)	[rp!no]
  6   .   lf?no
105:	proc  1 (phil:1) PhilLL.pml:14 (state 4)	[lf?no]
107:	proc  5 (phil:1) PhilLL.pml:22 (state 13)	[progress = 0]
  4   .   .   .   .   .   lf!release
109:	proc  5 (phil:1) PhilLL.pml:23 (state 14)	[lf!release]
  4   .   .   .   .   rp?release
110:	proc  4 (fork:1) PhilLL.pml:42 (state 5)	[rp?release]
  5   .   .   .   .   .   rf!release
112:	proc  5 (phil:1) PhilLL.pml:24 (state 15)	[rf!release]
  5   .   .   .   .   .   .   lp?release
113:	proc  6 (fork:1) PhilLL.pml:50 (state 14)	[lp?release]
  4   .   .   .   .   .   lf!are_you_free
115:	proc  5 (phil:1) PhilLL.pml:11 (state 1)	[lf!are_you_free]
  4   .   .   .   .   rp?are_you_free
116:	proc  4 (fork:1) PhilLL.pml:37 (state 1)	[rp?are_you_free]
  4   .   .   .   .   rp!yes
118:	proc  4 (fork:1) PhilLL.pml:37 (state 2)	[rp!yes]
  4   .   .   .   .   .   lf?yes
119:	proc  5 (phil:1) PhilLL.pml:13 (state 2)	[lf?yes]
  5   .   .   .   .   .   rf!are_you_free
121:	proc  5 (phil:1) PhilLL.pml:19 (state 10)	[rf!are_you_free]
  5   .   .   .   .   .   .   lp?are_you_free
122:	proc  6 (fork:1) PhilLL.pml:45 (state 10)	[lp?are_you_free]
  5   .   .   .   .   .   .   lp!yes
124:	proc  6 (fork:1) PhilLL.pml:45 (state 11)	[lp!yes]
  5   .   .   .   .   .   rf?yes
125:	proc  5 (phil:1) PhilLL.pml:21 (state 11)	[rf?yes]
  2   .   .   .   lf!release
127:	proc  3 (phil:1) PhilLL.pml:26 (state 18)	[lf!release]
  2   .   .   rp?release
128:	proc  2 (fork:1) PhilLL.pml:42 (state 5)	[rp?release]
130:	proc  5 (phil:1) PhilLL.pml:22 (state 12)	[progress = 1]
132:	proc  5 (phil:1) PhilLL.pml:22 (state 13)	[progress = 0]
  4   .   .   .   .   .   lf!release
134:	proc  5 (phil:1) PhilLL.pml:23 (state 14)	[lf!release]
  4   .   .   .   .   rp?release
135:	proc  4 (fork:1) PhilLL.pml:42 (state 5)	[rp?release]
  5   .   .   .   .   .   rf!release
137:	proc  5 (phil:1) PhilLL.pml:24 (state 15)	[rf!release]
  5   .   .   .   .   .   .   lp?release
138:	proc  6 (fork:1) PhilLL.pml:50 (state 14)	[lp?release]
  4   .   .   .   .   .   lf!are_you_free
140:	proc  5 (phil:1) PhilLL.pml:11 (state 1)	[lf!are_you_free]
  4   .   .   .   .   rp?are_you_free
141:	proc  4 (fork:1) PhilLL.pml:37 (state 1)	[rp?are_you_free]
  4   .   .   .   .   rp!yes
143:	proc  4 (fork:1) PhilLL.pml:37 (state 2)	[rp!yes]
  4   .   .   .   .   .   lf?yes
144:	proc  5 (phil:1) PhilLL.pml:13 (state 2)	[lf?yes]
  5   .   .   .   .   .   rf!are_you_free
146:	proc  5 (phil:1) PhilLL.pml:19 (state 10)	[rf!are_you_free]
  5   .   .   .   .   .   .   lp?are_you_free
147:	proc  6 (fork:1) PhilLL.pml:45 (state 10)	[lp?are_you_free]
  2   .   .   .   lf!are_you_free
149:	proc  3 (phil:1) PhilLL.pml:11 (state 1)	[lf!are_you_free]
  2   .   .   rp?are_you_free
150:	proc  2 (fork:1) PhilLL.pml:37 (state 1)	[rp?are_you_free]
  5   .   .   .   .   .   .   lp!yes
152:	proc  6 (fork:1) PhilLL.pml:45 (state 11)	[lp!yes]
  5   .   .   .   .   .   rf?yes
153:	proc  5 (phil:1) PhilLL.pml:21 (state 11)	[rf?yes]
155:	proc  5 (phil:1) PhilLL.pml:22 (state 12)	[progress = 1]
157:	proc  5 (phil:1) PhilLL.pml:22 (state 13)	[progress = 0]
  4   .   .   .   .   .   lf!release
159:	proc  5 (phil:1) PhilLL.pml:23 (state 14)	[lf!release]
  4   .   .   .   .   rp?release
160:	proc  4 (fork:1) PhilLL.pml:42 (state 5)	[rp?release]
  5   .   .   .   .   .   rf!release
162:	proc  5 (phil:1) PhilLL.pml:24 (state 15)	[rf!release]
  5   .   .   .   .   .   .   lp?release
163:	proc  6 (fork:1) PhilLL.pml:50 (state 14)	[lp?release]
  4   .   .   .   .   .   lf!are_you_free
165:	proc  5 (phil:1) PhilLL.pml:11 (state 1)	[lf!are_you_free]
  4   .   .   .   .   rp?are_you_free
166:	proc  4 (fork:1) PhilLL.pml:37 (state 1)	[rp?are_you_free]
  4   .   .   .   .   rp!yes
168:	proc  4 (fork:1) PhilLL.pml:37 (state 2)	[rp!yes]
  4   .   .   .   .   .   lf?yes
169:	proc  5 (phil:1) PhilLL.pml:13 (state 2)	[lf?yes]
  2   .   .   rp!yes
171:	proc  2 (fork:1) PhilLL.pml:37 (state 2)	[rp!yes]
  2   .   .   .   lf?yes
172:	proc  3 (phil:1) PhilLL.pml:13 (state 2)	[lf?yes]
  5   .   .   .   .   .   rf!are_you_free
174:	proc  5 (phil:1) PhilLL.pml:19 (state 10)	[rf!are_you_free]
  5   .   .   .   .   .   .   lp?are_you_free
175:	proc  6 (fork:1) PhilLL.pml:45 (state 10)	[lp?are_you_free]
  5   .   .   .   .   .   .   lp!yes
177:	proc  6 (fork:1) PhilLL.pml:45 (state 11)	[lp!yes]
  5   .   .   .   .   .   rf?yes
178:	proc  5 (phil:1) PhilLL.pml:21 (state 11)	[rf?yes]
180:	proc  5 (phil:1) PhilLL.pml:22 (state 12)	[progress = 1]
182:	proc  5 (phil:1) PhilLL.pml:22 (state 13)	[progress = 0]
  4   .   .   .   .   .   lf!release
184:	proc  5 (phil:1) PhilLL.pml:23 (state 14)	[lf!release]
  4   .   .   .   .   rp?release
185:	proc  4 (fork:1) PhilLL.pml:42 (state 5)	[rp?release]
  5   .   .   .   .   .   rf!release
187:	proc  5 (phil:1) PhilLL.pml:24 (state 15)	[rf!release]
  5   .   .   .   .   .   .   lp?release
188:	proc  6 (fork:1) PhilLL.pml:50 (state 14)	[lp?release]
  4   .   .   .   .   .   lf!are_you_free
190:	proc  5 (phil:1) PhilLL.pml:11 (state 1)	[lf!are_you_free]
  4   .   .   .   .   rp?are_you_free
191:	proc  4 (fork:1) PhilLL.pml:37 (state 1)	[rp?are_you_free]
  6   .   lf!are_you_free
193:	proc  1 (phil:1) PhilLL.pml:11 (state 1)	[lf!are_you_free]
  6   .   .   .   .   .   .   rp?are_you_free
194:	proc  6 (fork:1) PhilLL.pml:37 (state 1)	[rp?are_you_free]
  6   .   .   .   .   .   .   rp!yes
196:	proc  6 (fork:1) PhilLL.pml:37 (state 2)	[rp!yes]
  6   .   lf?yes
197:	proc  1 (phil:1) PhilLL.pml:13 (state 2)	[lf?yes]
  4   .   .   .   .   rp!yes
199:	proc  4 (fork:1) PhilLL.pml:37 (state 2)	[rp!yes]
  4   .   .   .   .   .   lf?yes
200:	proc  5 (phil:1) PhilLL.pml:13 (state 2)	[lf?yes]
  5   .   .   .   .   .   rf!are_you_free
202:	proc  5 (phil:1) PhilLL.pml:19 (state 10)	[rf!are_you_free]
  5   .   .   .   .   .   .   lp?are_you_free
203:	proc  6 (fork:1) PhilLL.pml:40 (state 3)	[lp?are_you_free]
  5   .   .   .   .   .   .   lp!no
205:	proc  6 (fork:1) PhilLL.pml:41 (state 4)	[lp!no]
  5   .   .   .   .   .   rf?no
206:	proc  5 (phil:1) PhilLL.pml:26 (state 17)	[rf?no]
  4   .   .   .   .   .   lf!release
208:	proc  5 (phil:1) PhilLL.pml:26 (state 18)	[lf!release]
  4   .   .   .   .   rp?release
209:	proc  4 (fork:1) PhilLL.pml:42 (state 5)	[rp?release]
  3   .   .   .   rf!are_you_free
211:	proc  3 (phil:1) PhilLL.pml:19 (state 10)	[rf!are_you_free]
  3   .   .   .   .   lp?are_you_free
212:	proc  4 (fork:1) PhilLL.pml:45 (state 10)	[lp?are_you_free]
  3   .   .   .   .   lp!yes
214:	proc  4 (fork:1) PhilLL.pml:45 (state 11)	[lp!yes]
  3   .   .   .   rf?yes
215:	proc  3 (phil:1) PhilLL.pml:21 (state 11)	[rf?yes]
  4   .   .   .   .   .   lf!are_you_free
217:	proc  5 (phil:1) PhilLL.pml:11 (state 1)	[lf!are_you_free]
  4   .   .   .   .   rp?are_you_free
218:	proc  4 (fork:1) PhilLL.pml:48 (state 12)	[rp?are_you_free]
  4   .   .   .   .   rp!no
220:	proc  4 (fork:1) PhilLL.pml:49 (state 13)	[rp!no]
  4   .   .   .   .   .   lf?no
221:	proc  5 (phil:1) PhilLL.pml:14 (state 4)	[lf?no]
223:	proc  3 (phil:1) PhilLL.pml:22 (state 12)	[progress = 1]
  4   .   .   .   .   .   lf!are_you_free
225:	proc  5 (phil:1) PhilLL.pml:11 (state 1)	[lf!are_you_free]
  4   .   .   .   .   rp?are_you_free
226:	proc  4 (fork:1) PhilLL.pml:48 (state 12)	[rp?are_you_free]
228:	proc  3 (phil:1) PhilLL.pml:22 (state 13)	[progress = 0]
  4   .   .   .   .   rp!no
230:	proc  4 (fork:1) PhilLL.pml:49 (state 13)	[rp!no]
  4   .   .   .   .   .   lf?no
231:	proc  5 (phil:1) PhilLL.pml:14 (state 4)	[lf?no]
  2   .   .   .   lf!release
233:	proc  3 (phil:1) PhilLL.pml:23 (state 14)	[lf!release]
  2   .   .   rp?release
234:	proc  2 (fork:1) PhilLL.pml:42 (state 5)	[rp?release]
  4   .   .   .   .   .   lf!are_you_free
236:	proc  5 (phil:1) PhilLL.pml:11 (state 1)	[lf!are_you_free]
  4   .   .   .   .   rp?are_you_free
237:	proc  4 (fork:1) PhilLL.pml:48 (state 12)	[rp?are_you_free]
  1   .   rf!are_you_free
239:	proc  1 (phil:1) PhilLL.pml:19 (state 10)	[rf!are_you_free]
  1   .   .   lp?are_you_free
240:	proc  2 (fork:1) PhilLL.pml:45 (state 10)	[lp?are_you_free]
  4   .   .   .   .   rp!no
242:	proc  4 (fork:1) PhilLL.pml:49 (state 13)	[rp!no]
  4   .   .   .   .   .   lf?no
243:	proc  5 (phil:1) PhilLL.pml:14 (state 4)	[lf?no]
  3   .   .   .   rf!release
245:	proc  3 (phil:1) PhilLL.pml:24 (state 15)	[rf!release]
  3   .   .   .   .   lp?release
246:	proc  4 (fork:1) PhilLL.pml:50 (state 14)	[lp?release]
  4   .   .   .   .   .   lf!are_you_free
248:	proc  5 (phil:1) PhilLL.pml:11 (state 1)	[lf!are_you_free]
  4   .   .   .   .   rp?are_you_free
249:	proc  4 (fork:1) PhilLL.pml:37 (state 1)	[rp?are_you_free]
  4   .   .   .   .   rp!yes
251:	proc  4 (fork:1) PhilLL.pml:37 (state 2)	[rp!yes]
  4   .   .   .   .   .   lf?yes
252:	proc  5 (phil:1) PhilLL.pml:13 (state 2)	[lf?yes]
  5   .   .   .   .   .   rf!are_you_free
254:	proc  5 (phil:1) PhilLL.pml:19 (state 10)	[rf!are_you_free]
  5   .   .   .   .   .   .   lp?are_you_free
255:	proc  6 (fork:1) PhilLL.pml:40 (state 3)	[lp?are_you_free]
  5   .   .   .   .   .   .   lp!no
257:	proc  6 (fork:1) PhilLL.pml:41 (state 4)	[lp!no]
  5   .   .   .   .   .   rf?no
258:	proc  5 (phil:1) PhilLL.pml:26 (state 17)	[rf?no]
  4   .   .   .   .   .   lf!release
260:	proc  5 (phil:1) PhilLL.pml:26 (state 18)	[lf!release]
  4   .   .   .   .   rp?release
261:	proc  4 (fork:1) PhilLL.pml:42 (state 5)	[rp?release]
  1   .   .   lp!yes
263:	proc  2 (fork:1) PhilLL.pml:45 (state 11)	[lp!yes]
  1   .   rf?yes
264:	proc  1 (phil:1) PhilLL.pml:21 (state 11)	[rf?yes]
  4   .   .   .   .   .   lf!are_you_free
266:	proc  5 (phil:1) PhilLL.pml:11 (state 1)	[lf!are_you_free]
  4   .   .   .   .   rp?are_you_free
267:	proc  4 (fork:1) PhilLL.pml:37 (state 1)	[rp?are_you_free]
  4   .   .   .   .   rp!yes
269:	proc  4 (fork:1) PhilLL.pml:37 (state 2)	[rp!yes]
  4   .   .   .   .   .   lf?yes
270:	proc  5 (phil:1) PhilLL.pml:13 (state 2)	[lf?yes]
  5   .   .   .   .   .   rf!are_you_free
272:	proc  5 (phil:1) PhilLL.pml:19 (state 10)	[rf!are_you_free]
  5   .   .   .   .   .   .   lp?are_you_free
273:	proc  6 (fork:1) PhilLL.pml:40 (state 3)	[lp?are_you_free]
  5   .   .   .   .   .   .   lp!no
275:	proc  6 (fork:1) PhilLL.pml:41 (state 4)	[lp!no]
  5   .   .   .   .   .   rf?no
276:	proc  5 (phil:1) PhilLL.pml:26 (state 17)	[rf?no]
  2   .   .   .   lf!are_you_free
278:	proc  3 (phil:1) PhilLL.pml:11 (state 1)	[lf!are_you_free]
  2   .   .   rp?are_you_free
279:	proc  2 (fork:1) PhilLL.pml:48 (state 12)	[rp?are_you_free]
  4   .   .   .   .   .   lf!release
281:	proc  5 (phil:1) PhilLL.pml:26 (state 18)	[lf!release]
  4   .   .   .   .   rp?release
282:	proc  4 (fork:1) PhilLL.pml:42 (state 5)	[rp?release]
  4   .   .   .   .   .   lf!are_you_free
284:	proc  5 (phil:1) PhilLL.pml:11 (state 1)	[lf!are_you_free]
  4   .   .   .   .   rp?are_you_free
285:	proc  4 (fork:1) PhilLL.pml:37 (state 1)	[rp?are_you_free]
  4   .   .   .   .   rp!yes
287:	proc  4 (fork:1) PhilLL.pml:37 (state 2)	[rp!yes]
  4   .   .   .   .   .   lf?yes
288:	proc  5 (phil:1) PhilLL.pml:13 (state 2)	[lf?yes]
  5   .   .   .   .   .   rf!are_you_free
290:	proc  5 (phil:1) PhilLL.pml:19 (state 10)	[rf!are_you_free]
  5   .   .   .   .   .   .   lp?are_you_free
291:	proc  6 (fork:1) PhilLL.pml:40 (state 3)	[lp?are_you_free]
  2   .   .   rp!no
293:	proc  2 (fork:1) PhilLL.pml:49 (state 13)	[rp!no]
  2   .   .   .   lf?no
294:	proc  3 (phil:1) PhilLL.pml:14 (state 4)	[lf?no]
  5   .   .   .   .   .   .   lp!no
296:	proc  6 (fork:1) PhilLL.pml:41 (state 4)	[lp!no]
  5   .   .   .   .   .   rf?no
297:	proc  5 (phil:1) PhilLL.pml:26 (state 17)	[rf?no]
  4   .   .   .   .   .   lf!release
299:	proc  5 (phil:1) PhilLL.pml:26 (state 18)	[lf!release]
  4   .   .   .   .   rp?release
300:	proc  4 (fork:1) PhilLL.pml:42 (state 5)	[rp?release]
  4   .   .   .   .   .   lf!are_you_free
302:	proc  5 (phil:1) PhilLL.pml:11 (state 1)	[lf!are_you_free]
  4   .   .   .   .   rp?are_you_free
303:	proc  4 (fork:1) PhilLL.pml:37 (state 1)	[rp?are_you_free]
  4   .   .   .   .   rp!yes
305:	proc  4 (fork:1) PhilLL.pml:37 (state 2)	[rp!yes]
  4   .   .   .   .   .   lf?yes
306:	proc  5 (phil:1) PhilLL.pml:13 (state 2)	[lf?yes]
308:	proc  1 (phil:1) PhilLL.pml:22 (state 12)	[progress = 1]
  5   .   .   .   .   .   rf!are_you_free
310:	proc  5 (phil:1) PhilLL.pml:19 (state 10)	[rf!are_you_free]
  5   .   .   .   .   .   .   lp?are_you_free
311:	proc  6 (fork:1) PhilLL.pml:40 (state 3)	[lp?are_you_free]
  5   .   .   .   .   .   .   lp!no
313:	proc  6 (fork:1) PhilLL.pml:41 (state 4)	[lp!no]
  5   .   .   .   .   .   rf?no
314:	proc  5 (phil:1) PhilLL.pml:26 (state 17)	[rf?no]
  4   .   .   .   .   .   lf!release
316:	proc  5 (phil:1) PhilLL.pml:26 (state 18)	[lf!release]
  4   .   .   .   .   rp?release
317:	proc  4 (fork:1) PhilLL.pml:42 (state 5)	[rp?release]
  4   .   .   .   .   .   lf!are_you_free
319:	proc  5 (phil:1) PhilLL.pml:11 (state 1)	[lf!are_you_free]
  4   .   .   .   .   rp?are_you_free
320:	proc  4 (fork:1) PhilLL.pml:37 (state 1)	[rp?are_you_free]
  2   .   .   .   lf!are_you_free
322:	proc  3 (phil:1) PhilLL.pml:11 (state 1)	[lf!are_you_free]
  2   .   .   rp?are_you_free
323:	proc  2 (fork:1) PhilLL.pml:48 (state 12)	[rp?are_you_free]
  4   .   .   .   .   rp!yes
325:	proc  4 (fork:1) PhilLL.pml:37 (state 2)	[rp!yes]
  4   .   .   .   .   .   lf?yes
326:	proc  5 (phil:1) PhilLL.pml:13 (state 2)	[lf?yes]
  5   .   .   .   .   .   rf!are_you_free
328:	proc  5 (phil:1) PhilLL.pml:19 (state 10)	[rf!are_you_free]
  5   .   .   .   .   .   .   lp?are_you_free
329:	proc  6 (fork:1) PhilLL.pml:40 (state 3)	[lp?are_you_free]
  5   .   .   .   .   .   .   lp!no
331:	proc  6 (fork:1) PhilLL.pml:41 (state 4)	[lp!no]
  5   .   .   .   .   .   rf?no
332:	proc  5 (phil:1) PhilLL.pml:26 (state 17)	[rf?no]
  4   .   .   .   .   .   lf!release
334:	proc  5 (phil:1) PhilLL.pml:26 (state 18)	[lf!release]
  4   .   .   .   .   rp?release
335:	proc  4 (fork:1) PhilLL.pml:42 (state 5)	[rp?release]
337:	proc  1 (phil:1) PhilLL.pml:22 (state 13)	[progress = 0]
  4   .   .   .   .   .   lf!are_you_free
339:	proc  5 (phil:1) PhilLL.pml:11 (state 1)	[lf!are_you_free]
  4   .   .   .   .   rp?are_you_free
340:	proc  4 (fork:1) PhilLL.pml:37 (state 1)	[rp?are_you_free]
  4   .   .   .   .   rp!yes
342:	proc  4 (fork:1) PhilLL.pml:37 (state 2)	[rp!yes]
  4   .   .   .   .   .   lf?yes
343:	proc  5 (phil:1) PhilLL.pml:13 (state 2)	[lf?yes]
  5   .   .   .   .   .   rf!are_you_free
345:	proc  5 (phil:1) PhilLL.pml:19 (state 10)	[rf!are_you_free]
  5   .   .   .   .   .   .   lp?are_you_free
346:	proc  6 (fork:1) PhilLL.pml:40 (state 3)	[lp?are_you_free]
  5   .   .   .   .   .   .   lp!no
348:	proc  6 (fork:1) PhilLL.pml:41 (state 4)	[lp!no]
  5   .   .   .   .   .   rf?no
349:	proc  5 (phil:1) PhilLL.pml:26 (state 17)	[rf?no]
  2   .   .   rp!no
351:	proc  2 (fork:1) PhilLL.pml:49 (state 13)	[rp!no]
  2   .   .   .   lf?no
352:	proc  3 (phil:1) PhilLL.pml:14 (state 4)	[lf?no]
  4   .   .   .   .   .   lf!release
354:	proc  5 (phil:1) PhilLL.pml:26 (state 18)	[lf!release]
  4   .   .   .   .   rp?release
355:	proc  4 (fork:1) PhilLL.pml:42 (state 5)	[rp?release]
  4   .   .   .   .   .   lf!are_you_free
357:	proc  5 (phil:1) PhilLL.pml:11 (state 1)	[lf!are_you_free]
  4   .   .   .   .   rp?are_you_free
358:	proc  4 (fork:1) PhilLL.pml:37 (state 1)	[rp?are_you_free]
  4   .   .   .   .   rp!yes
360:	proc  4 (fork:1) PhilLL.pml:37 (state 2)	[rp!yes]
  4   .   .   .   .   .   lf?yes
361:	proc  5 (phil:1) PhilLL.pml:13 (state 2)	[lf?yes]
  5   .   .   .   .   .   rf!are_you_free
363:	proc  5 (phil:1) PhilLL.pml:19 (state 10)	[rf!are_you_free]
  5   .   .   .   .   .   .   lp?are_you_free
364:	proc  6 (fork:1) PhilLL.pml:40 (state 3)	[lp?are_you_free]
Never claim moves to line 77	[(!(progress))]
  5   .   .   .   .   .   .   lp!no
366:	proc  6 (fork:1) PhilLL.pml:41 (state 4)	[lp!no]
  5   .   .   .   .   .   rf?no
367:	proc  5 (phil:1) PhilLL.pml:26 (state 17)	[rf?no]
  <<<<<START OF CYCLE>>>>>
Never claim moves to line 80	[(1)]
  4   .   .   .   .   .   lf!release
369:	proc  5 (phil:1) PhilLL.pml:26 (state 18)	[lf!release]
  4   .   .   .   .   rp?release
370:	proc  4 (fork:1) PhilLL.pml:42 (state 5)	[rp?release]
  4   .   .   .   .   .   lf!are_you_free
372:	proc  5 (phil:1) PhilLL.pml:11 (state 1)	[lf!are_you_free]
  4   .   .   .   .   rp?are_you_free
373:	proc  4 (fork:1) PhilLL.pml:37 (state 1)	[rp?are_you_free]
  4   .   .   .   .   rp!yes
375:	proc  4 (fork:1) PhilLL.pml:37 (state 2)	[rp!yes]
  4   .   .   .   .   .   lf?yes
376:	proc  5 (phil:1) PhilLL.pml:13 (state 2)	[lf?yes]
  5   .   .   .   .   .   rf!are_you_free
378:	proc  5 (phil:1) PhilLL.pml:19 (state 10)	[rf!are_you_free]
  5   .   .   .   .   .   .   lp?are_you_free
379:	proc  6 (fork:1) PhilLL.pml:40 (state 3)	[lp?are_you_free]
  5   .   .   .   .   .   .   lp!no
381:	proc  6 (fork:1) PhilLL.pml:41 (state 4)	[lp!no]
  5   .   .   .   .   .   rf?no
382:	proc  5 (phil:1) PhilLL.pml:26 (state 17)	[rf?no]
spin: trail ends after 382 steps
-------------
final state:
-------------
#processes: 7
		progress = 0
382:	proc  6 (fork:1) PhilLL.pml:39 (state 7)
382:	proc  5 (phil:1) PhilLL.pml:26 (state 18)
382:	proc  4 (fork:1) PhilLL.pml:39 (state 7)
382:	proc  3 (phil:1) PhilLL.pml:10 (state 7)
382:	proc  2 (fork:1) PhilLL.pml:47 (state 16)
382:	proc  1 (phil:1) PhilLL.pml:23 (state 14)
382:	proc  0 (:init::1) PhilLL.pml:72 (state 8) <valid end state>
382:	proc  - (never_0:1) PhilLL.pml:80 (state 8)
7 processes created
[ganesh@vpn-155-94-98-155 soln]$

--*/

