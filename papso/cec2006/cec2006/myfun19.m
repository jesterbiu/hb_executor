function f=myfun19(x)
%G19
%usage:  [f] = myfun19(x); and [c,ceq] = myfun19(x)
%
%lb = zeros(15,1);
%ub = 10.*ones(15,1);
%FcnCount = 0; % count the number of function evaluations
%for i = 1:100, % or we can use FcnCount as a stop criteria
%  x0 = lb + rand(15,1).*(ub-lb); % random starting point
%  [X19(:,i),F19(i),exitflag,output] = fmincon('myfun19',x0,[],[],[],[],lb,ub,'mycon19');
%  FcnCount = output.funcCount + FcnCount; % increment function evaluation counter
%end
%% now pick out the best solution found from all these hill-climbs
%[dummy,i] = min(F19); x19 = X19(:,i),[f19,g19,h19]=mlbsuite(x19,5,0,'g19'), FcnCount

% Jane Jing Liang, Nov 2005

[f g h] = mlbsuite(x, 5, 0, 'g19');
