M=70;

a(n) = if(n==0, 1, sumdiv(n, d, (-1)^(d-n/d+1)*binomial(d, n/d-1))); 
for(n=0, M, print1(a(n), ", "));