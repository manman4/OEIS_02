a(n) = my(A=1+x*O(x^n)); for(i=0, n, A=1-x*A^4*(1-2*A)); polcoeff(A, n); 
for(n=0, 20, print1(a(n),", "))    