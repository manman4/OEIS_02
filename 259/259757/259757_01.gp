\\ G.f. A(x) satisfies A(x) = 1 + x * (1 - A(x) + A(x)^2 - A(x)^3 + A(x)^4).
seq(n) = my(A=1); for(i=1, n, A=1 + x * (1 - A + A^2 - A^3 + A^4) +x*O(x^n) ); Vec(A);
seq(25)                   
