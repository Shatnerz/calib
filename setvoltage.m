%The values of a and b are specific to workstation 12
function[] = setvoltage(number)

if ((number<0) || (number>4.891))
    error('Input must be between 0 and 4.891')
end
a = -0.001321569994774;
b=0.019198139388147;
sigma_a =  2.1107e-08;
sigma_b =1.4755e-14;


analognumber = round((number-a)./b);

analogWrite(10,analognumber);

err = sqrt(sigma_a+(analognumber.^2).*sigma_b);
fprintf('Actual Output Voltage: %.4g +/- %.1g V\n',(a+b*analognumber),err)
end