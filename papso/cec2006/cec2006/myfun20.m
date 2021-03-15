function f=myfun20(x)
%G20
%usage:  [f] = myfun20(x); and [c,ceq] = myfun20(x)
%
%lb=zeros(24,1);
%ub=10.*ones(24,1);
%b=[44.094 58.12  58.12 137.4 120.9 170.9 62.501 84.94 133.425 82.507 46.07 60.097 44.094 58.12 58.12 137.4 120.9 170.9 62.501 84.94 133.425 82.507 46.07 60.097];
%d=[31.244 36.12 34.784 92.7 82.7 91.6 56.708 82.7 80.8 64.517 49.4 49.1];
%f=(0.7302 * 530.0 * (14.7 / 40));
%Aeq = [ones(1,24);[1./d(1:12),f./b(13:24)]];
%beq = [1 1.671]';
%FcnCount = 0; % count the number of function evaluations
%for i = 1:100, % or we can use FcnCount as a stop criteria
%  x0 = lb + rand(24,1).*(ub-lb); % random starting point
%  [X20(:,i),F20(i),exitflag,output] = fmincon('myfun20',x0,[],[],Aeq,beq,lb,ub,'mycon20');
%  FcnCount = output.funcCount + FcnCount; % increment function evaluation counter
%end
%% now pick out the best solution found from all these hill-climbs
%[dummy,i] = min(F20); x20 = X20(:,i),[f20,g20,h20]=mlbsuite(x20,6,14,'g20'), FcnCount

% Jane Jing Liang, Nov 2005

[f,g,h] = mlbsuite(x, 6, 14, 'g20');
f = f; % because its a maximization problem