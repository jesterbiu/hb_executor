function f=myfun17(x)
%G17
%usage:  [f] = myfun17(x); and [c,ceq] = myfun17(x)
%
%lb=[0 0 340 340 -1000 0]';
%ub=[400 1000 420 420 1000 0.5236]';
%FcnCount = 0; % count the number of function evaluations
%for i = 1:100, % or we can use FcnCount as a stop criteria
%  x0 = lb + rand(6,1).*(ub-lb); % random starting point
%  [X17(:,i),F17(i),exitflag,output] = fmincon('myfun17',x0,[],[],[],[],lb,ub,'mycon17');
%  FcnCount = output.funcCount + FcnCount; % increment function evaluation counter
%end
%% now pick out the best solution found from all these hill-climbs
%[dummy,i] = min(F17); x17 = X17(:,i), [f17,g17,h17] = mlbsuite(x17,0,4,'g17'), FcnCount

% Jane Jing Liang, Nov 2005

[f g h] = mlbsuite(x, 0, 4, 'g17');
