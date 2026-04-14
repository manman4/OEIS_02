\\ Let A(n,k) = (2*n)! * [x^(2*n)] C(x)^k. A(0,k) = 1 and A(n,k) = k*(k+3) * A(n-1,k+6) - k*(k+2) * A(n-1,k+4) for n > 0. a(n) = A(n,3).
a(n, k=3) = if(n==0, 1, k*(k+3)*a(n-1, k+6)-k*(k+2)*a(n-1, k+4));
for(n=0, 20, print1(a(n), ", "));

