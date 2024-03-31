seq(n) = my(A=1); for(i=1, n, A=(1 + x * A^3 * (1+A^3) +x*O(x^n) )^(1/2) ); Vec(A);
seq(25)                                                                             

\\ A^2 = A370474
seq(n) = my(A=1); for(i=1, n, A=(1 + x * A^3 * (1+A^3) +x*O(x^n) )^(1/2) ); Vec(A^2);
seq(25)