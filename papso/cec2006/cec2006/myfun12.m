function f = myfun12(x),
%G12 Michalewicz and Koziel (1999)
%usage:  [f] = myfun12(x); and [c,ceq] = myfun12(x)
%
%opts=optimset('fmincon');
%opts.MaxFunEvals=100000;
%opts.TolCon=1E-10;
%opts.TolFun=1E-10;
%lb = zeros(3,1);
%ub = 10*ones(3,1);
%FcnCount = 0; % count the number of function evaluations
%for i = 1:100, % or we can use FcnCount as a stop criteria
%  x0 = lb + rand(3,1).*(ub-lb); % random starting point
%  [X12(:,i),F12(i),exitflag,output] = fmincon('myfun12',x0,[],[],[],[],lb,ub,'mycon12', opts);
%  FcnCount = output.funcCount + FcnCount; % increment function evaluation counter
%end
%% now pick out the best solution found from all these hill-climbs
%[dummy,i] = min(F12); x12 = X12(:,i), [f12,g12,h12] = mlbsuite(x12,1,0,'g12'), FcnCount

[f,c,ceq] = mlbsuite(x,1,0,'g12');

