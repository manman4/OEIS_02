\\ G.f. A(x) satisfies A(x) = 1 + x * (1 - A(x) + A(x)^2 - A(x)^3 + A(x)^4 - A(x)^5 + A(x)^6).
seq(n) = my(A=1); for(i=1, n, A=1 + x * (1 - A + A^2 - A^3 + A^4 - A^5 + A^6) +x*O(x^n) ); Vec(A);
seq(25)                   
