T(n,m) = my(A=1+x); for(i=1, n, A = 1+x + subst(A, x, x*A +x*O(x^n)) - A); polcoef(A^m, n);   

\\ Let b(n,k) = [x^n] A(x)^k.
\\ b(0,1) = b(1,1) = 1;   b(n,1) =         Sum_{j=1..n-1} b(j,1) * b(n-j,j).
\\ For k > 1, b(0,k) = 1; b(n,k) = (1/n) * Sum_{j=1..n} ((k+1)*j-n) * b(j,1) * b(n-j,k).
\\ a(n) = b(n,1). 
b(n, k) = if((n==0 & k>0) || n*k==1, 1, if(k==1, sum(j=1, n-1, b(j, 1)*b(n-j, j)), sum(j=1, n, ((k+1)*j-n)*b(j, 1)*b(n-j, k))/n));
for(k=1, 5, for(n=0, 13, print1(b(n,k)-T(n,k), ", ")); print);

for(n=0, 13, print1(b(n, 1),", "));
