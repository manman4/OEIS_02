\\ a(n) = 4^n * binomial((8*n-1)/2,n).
a(n) = 4^n * binomial((8*n-1)/2,n);
for(n=0, 20, print1(a(n), ", "));

\\ a(n) = [x^n] 1/(1-4*x)^((6*n+1)/2).
b(n) = my(x='x+O('x^(n+1))); polcoef( 1/(1-4*x)^((6*n+1)/2), n);
for(n=0, 50, print1(a(n)-b(n), ", "));

\\ a(n) = [x^n] (1+4*x)^((8*n-1)/2).
c(n) = my(x='x+O('x^(n+1))); polcoef( (1+4*x)^((8*n-1)/2), n);
for(n=0, 50, print1(a(n)-c(n), ", "));