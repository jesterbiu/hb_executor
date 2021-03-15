function f = myfun11(x),
%G11 (Maa and Shanblatt, 1992)
%usage:  [f] = myfun11(x); and [c,ceq] = myfun11(x)
%
%lb = [-1;-1];
%ub = [1;1];
%FcnCount = 0; % count the number of function evaluations
%for i = 1:100, % or we can use FcnCount as a stop criteria
%  x0 = lb + rand(2,1).*(ub-lb); % random starting point
%  [X11(:,i),F11(i),exitflag,output] = fmincon('myfun11',x0,[],[],[],[],lb,ub,'mycon11');
%  FcnCount = output.funcCount + FcnCount; % increment function evaluation counter
%end
%% now pick out the best solution found from all these hill-climbs
%[dummy,i] = min(F11); x11 = X11(:,i), [f11,g11,h11] = mlbsuite(x11,0,1,'g11'), FcnCount

[f, g, h] = mlbsuite(x,0,1,'g11');
