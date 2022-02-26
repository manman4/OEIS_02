M=24;

a(n) = if(n<3, 1, a(n-1)-3!*binomial(n-1, 2)*a(n-3));
for(n=0, M, print1(a(n), ", "));