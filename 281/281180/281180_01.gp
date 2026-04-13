\\ a(n) = (2*n-1)! * [x^(2*n-1)] 2*S(x)*(1+S(x)^2)^3 - x*(1+S(x)^2)^2 - S(x), where e.g.f. S(x) satisfies S(x) = Integral (1 + S(x)^2)^2 dx.
a(n) = my(S=x); S = serreverse( intformal( 1/(1 + x^2 +x*O(x^(2*n)))^2)); S;
v=a(217);
for(n=1, 217, write("b281180.txt", n, " ", (2*n-1)! * polcoef(v, 2*n-1)));