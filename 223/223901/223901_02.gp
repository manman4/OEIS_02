M=20;

\\ a(n) = (-1)^n * Sum_{k=0..3} k! * Stirling2(4,k+1) * |Stirling1(n+1,k+1)|.
a(n) = (-1)^n * sum(k=0, 3, k!*stirling(4, k+1, 2)*abs(stirling(n+1, k+1, 1))); 
for(n=0, M, print1(a(n), ", ")); 