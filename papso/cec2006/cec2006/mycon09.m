function [c,ceq] = mycon09(x),
%G09 (Hock and Schittkowski, 1981, problem 100)
%usage: [c,ceq] = mycon09(x)

[f,c,ceq] = mlbsuite(x,4,0,'g09');
