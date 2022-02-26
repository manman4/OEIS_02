M=28;

a(n) = if(n<5, 1, a(n-1)+binomial(n-1, 4)*a(n-5)); 
for(n=0, M, print1(a(n), ", "));