function f=myfun18(x)
%G18
%usage:  [f] = myfun18(x); and [c,ceq] = myfun18(x)
%
%lb=[-10 -10 -10 -10 -10 -10 -10 -10 0]';
%ub=[10 10 10 10 10 10 10 10 20]';
%FcnCount = 0; % count the number of function evaluations
%for i = 1:100, % or we can use FcnCount as a stop criteria
%  x0 = lb + rand(9,1).*(ub-lb); % random starting point
%  [X18(:,i),F18(i),exitflag,output] = fmincon('myfun18',x0,[],[],[],[],lb,ub,'mycon18');
%  FcnCount = output.funcCount + FcnCount; % increment function evaluation counter
%end
%% now pick out the best solution found from all these hill-climbs
%[dummy,i] = min(F18); x18 = X18(:,i), [f18,g18,h18]=mlbsuite(x18,13,0,'g18'), FcnCount

% Jane Jing Liang, Nov 2005

[f g h] = mlbsuite(x, 13, 0, 'g18');
