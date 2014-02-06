function[a,b,sig_a,sig_b] = analyse_voltage(data)
%based on lab manual
%returns best fit for a+bx=y 
%and uncertainty on a and b

[rows ~]=size(data);

for i=1:rows;
    alpha=sum(1/data(:,3).^2);
    beta=sum(data(:,1)./data(:,3).^2);
    gamma=sum(data(:,2)./data(:,3).^2);
    lambda=sum(data(:,1).^2./data(:,3).^2);
    rho=sum(data(:,1).*data(:,2)./data(:,3).^2);
end

[a,b,sig_a,sig_b]=use_inv(alpha,beta,lambda,gamma,rho);