function [c,ceq] = mycon12(x),
%G12 Michalewicz and Koziel (1999)
%usage: [c,ceq] = mycon12(x)

[f,c,ceq] = mlbsuite(x,1,0,'g12');
