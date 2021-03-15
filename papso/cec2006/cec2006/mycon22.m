function [c, ceq] = mycon22(x)
%G22
%usage: [c,ceq] = mycon22(x)

% Jane Jing Liang, Nov 2005

[f, c, h] = mlbsuite(x, 1, 19, 'g22');

% c= -x(1,:)+abs(x(2,:)).^0.6+abs(x(3,:)).^0.6+abs(x(4,:)).^0.6;

h_num=[7, 8, 9, 12:19];
ceq=h(h_num,:);