M=30;

a(n) = if(n<7, 1, a(n-1)+binomial(n-1, 6)*a(n-7)); 
for(n=0, M, print1(a(n), ", "));