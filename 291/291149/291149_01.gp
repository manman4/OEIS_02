a(n) = {
    my(CF=1+x*O(x^n), M=round((n+1)^(1.0/4))); 
    for(k=0, M, 
        CF=1/(1-x^((M-k+1)^4)*CF)
    ); 
    polcoeff(CF, n, x)
};
for(n=0, 82, print1(a(n),", "))  