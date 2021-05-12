M=50;

a160891(n) = sumdiv(n, d, moebius(n/d)*d^4)/eulerphi(n);
a(n) = sumdiv(n, d, a160891(d));
for(n=1, M, print1(a(n), ", "));