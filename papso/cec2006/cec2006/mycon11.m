function [c,ceq] = mycon11(x),
%G11 (Maa and Shanblatt, 1992)
%usage: [c,ceq] = mycon11(x)

[f,c,ceq] = mlbsuite(x,0,1,'g11');
