\\ A357394
\\ my(N=20, x='x+O('x^N)); concat(0, Vec(serlaplace(serreverse( exp(-2*x) * log(1 + x) ))))

\\ A357395
\\ my(N=20, x='x+O('x^N)); concat(0, Vec(serlaplace(serreverse( exp(-3*x) * log(1 + x) ))))


\\ A357334 E.g.f.: Series_Reversion( 1 - exp(-x * exp(-3*x)) ). 
\\ my(N=20, x='x+O('x^N)); concat(0, Vec(serlaplace(serreverse( 1 - exp(-x * exp(-3*x)) ))))


\\ A277489 E.g.f.: Series_Reversion( exp(x * exp(-x)) - 1 ).
my(N=20, x='x+O('x^N)); concat(0, Vec(serlaplace(serreverse( exp(x * exp(-x)) - 1 ))))

\\ A357337
\\ my(N=20, x='x+O('x^N)); concat(0, Vec(serlaplace(serreverse( exp(x * exp(-2*x)) - 1 ))))

\\ A357338
\\ my(N=20, x='x+O('x^N)); concat(0, Vec(serlaplace(serreverse( exp(x * exp(-3*x)) - 1 ))))


\\ A357423
\\ my(N=20, x='x+O('x^N)); concat(0, Vec(serlaplace(serreverse( exp(-x) * (exp(x * exp(x)) - 1) )))) 

\\ A357349
\\ my(N=20, x='x+O('x^N)); concat(0, Vec(serlaplace(serreverse( exp(-x) * (exp(x * exp(-x)) - 1) )))) 

\\ A357350
\\ my(N=20, x='x+O('x^N)); concat(0, Vec(serlaplace(serreverse( exp(-x) * (exp(x * exp(-2*x)) - 1) )))) 

\\ A357351
\\ my(N=20, x='x+O('x^N)); concat(0, Vec(serlaplace(serreverse( exp(-x) * (exp(x * exp(-3*x)) - 1) )))) 


\\ A357392
\\ my(N=20, x='x+O('x^N)); concat(0, Vec(serlaplace(serreverse( exp(-2*x) * (1 - exp(-x)) ))))
\\ my(N=20, x='x+O('x^N)); concat(0, Vec(serlaplace(serreverse( exp(-3*x) * (exp(x) - 1) ))))

\\ A357393
\\ my(N=20, x='x+O('x^N)); concat(0, Vec(serlaplace(serreverse( exp(-3*x) * (1 - exp(-x)) ))))
\\ my(N=20, x='x+O('x^N)); concat(0, Vec(serlaplace(serreverse( exp(-4*x) * (exp(x) - 1) ))))


