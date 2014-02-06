%Values are hard coded for station 12

function[analogvoltage] = getvoltage(digitalvoltage)


if ((digitalvoltage<0) || (digitalvoltage>1023))
    error('Input must be between 0 and 1023');
end
if rem(digitalvoltage,1)~=0
    digitalvoltage=round(digitalvoltage);
    fprintf('Input value must be integer. Rounded to %d\n',digitalvoltage);
end
c = -0.002699220587270;
d = 0.004799794968931;
sig_c = 0.005255991830104;
sig_d =5.904305273214652e-11;


analogvoltage = d.*digitalvoltage+c;
end