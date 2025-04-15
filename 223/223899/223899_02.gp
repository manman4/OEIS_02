M=20;

\\ a(n) = (-1)^n * Sum_{k=0..2} k! * Stirling2(3,k+1) * |Stirling1(n+1,k+1)|.
a(n) = (-1)^n * sum(k=0, 2, k!*stirling(3, k+1, 2)*abs(stirling(n+1, k+1, 1)));
for(n=0, M, print1(a(n), ", ")); 