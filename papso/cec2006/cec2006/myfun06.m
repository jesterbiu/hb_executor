function f = myfun06(x),
%G06 (Flounds and Pardalos, 1987)
%usage:  [f] = myfun06(x); and [c,ceq] = myfun06(x)
%
%lb = [13;0];
%ub = [100;100];
%FcnCount = 0; % count the number of function evaluations
%for i = 1:100, % or we can use FcnCount as a stop criteria
%  x0 = lb + rand(2,1).*(ub-lb); % random starting point
%  [X06(:,i),F06(i),exitflag,output] = fmincon('myfun06',x0,[],[],[],[],lb,ub,'mycon06');
%  FcnCount = output.funcCount + FcnCount; % increment function evaluation counter
%end
%% now pick out the best solution found from all these hill-climbs
%[dummy,i] = min(F06); x06 = X06(:,i), [f06,g06,h06] = mlbsuite(x06,2,0,'g06'), FcnCount

[f, c, ceq] = mlbsuite(x, 2, 0, 'g06');
