M=17;

a(n) = if(n<4, ((n==0)||(n==3))*n!, 2*(n-1) * a(n-1) - (n-1)*(n-2) * a(n-2) + 6*binomial(n-1,2) * a(n-3) - 12*binomial(n-1,3) * a(n-4));
for(n=0, M, print1(a(n), ", "));
