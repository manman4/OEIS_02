M=65;

a(n) = if(n==0, 1, sumdiv(n, d, (-1)^(d-1)*binomial(d+n/d-2, d-1))); 
for(n=1, M, print1(a(n), ", "));