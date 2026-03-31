\\ a(0) = 1; a(n) = Sum_{k=1..n} binomial(2*n-1,2*k-1) * (2*(k-1))! * a(n-k). 
a(n) = if(n==0, 1, sum(k=1, n, binomial(2*n-1,2*k-1) * (2*(k-1))! * a(n-k)));                                                                     
for(n=0, 20, print1(a(n),", "));       