function f=myfun24(x)
%G24
%usage:  [f] = mycon24(x); and [c,ceq] = mycon24(x)
%
%lb=[0 0]';
%ub=[3 4]';
%FcnCount = 0; % count the number of function evaluations
%for i = 1:100, % or we can use FcnCount as a stop criteria
%  x0 = lb + rand(2,1).*(ub-lb); % random starting point
%  [X24(:,i),F24(i),exitflag,output] = fmincon('myfun24',x0,[],[],[],[],lb,ub,'mycon24');
%  FcnCount = output.funcCount + FcnCount; % increment function evaluation counter
%end
%% now pick out the best solution found from all these hill-climbs
%[dummy,i] = min(F24); x24 = X24(:,i),[f24,g24,h24]=mlbsuite(x24,2,0,'g24'), FcnCount

% Jane Jing Liang, Nov 2005
[f,g,h] = mlbsuite(x, 2, 0, 'g24');
