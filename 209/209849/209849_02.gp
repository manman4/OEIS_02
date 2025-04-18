M=20;

T(n, k) = sum(j=k, n, 2^(n-j)*stirling(n, j, 2)*stirling(j, k, 1)); 
for(n=1, M, for(k=1, n, print1(T(n, k),", ")));

\\ E.g.f. of column k (with leading zeros): f(x)^k / k! with f(x) = log(1 + (exp(2*x) - 1)/2).
S(n, k) = my(N=30, x='x+O('x^(n+1))); n! * polcoef( log(1 + (exp(2*x) - 1)/2)^k /k!, n);
for(n=0, M, for(k=0, n, print1(T(n, k)-S(n, k),", ")));


