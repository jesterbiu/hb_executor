function [c,ceq] = mycon13(x),
%G13 (Hock And Schittkowski, 1981)
%usage: [c,ceq] = mycon13(x)

[f,c,ceq] = mlbsuite(x,0,3,'g13');
