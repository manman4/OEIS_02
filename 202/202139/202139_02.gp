M=30;

a(n) = n! * sum(m=1, n, (m-1)! * sum(k=0, n-m, stirling(k+m, m, 1) * 2^k * binomial(n-1,k+m-1)/(k+m)!));
for(n=0, M, print1(a(n), ", "));