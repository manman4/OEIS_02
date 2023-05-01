M=20;

a(n) = if(n==0, 1, -n*a(n-1) + n^n);
for(n=0, M, print1(a(n),", ")) 