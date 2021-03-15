function [c,ceq] = mycon10(x),
%G10 (Hock and Schittkowski, 1981, problem 106, heat exchanger design)
%usage: [c,ceq] = mycon10(x)

[f,c,ceq] = mlbsuite(x,6,0,'g10');
c = c(4:6); % the first three are linear constraints