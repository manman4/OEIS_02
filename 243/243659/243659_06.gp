\\ G.f.: 1 + Series_Reversion( x / ((1+x)^3 * (1+2*x)) ). 
my(N=40, x='x+O('x^N)); Vec(1 + serreverse( x / ((1+x)^3 * (1+2*x)) ))  
