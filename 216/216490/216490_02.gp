seq(n) = my(A=1); for(i=1, n, A=1 + x*A^2*(1 + x^2*A^3 +x*O(x^n)) ); Vec(A);         
seq(30) 
