%Frequency must be in kHz (and b\w .160 kHz --> 2.5 kHz)

function[] = functionGenerator(analogFrequency,analogAmplitude,type)

a = -0.001321569994774;
b=0.019198139388147;
c = 0.0184;
d=0.0089;
digitalAmplitude =(analogAmplitude-a)./b;
digitalFrequency = (((1/analogFrequency)-c)./d);

e=-.1186;
g=.0135;

redAmp=@(f)e+g*f;


length = digitalFrequency; %test-this is determined from freq
%x is the range; y is the function
x = 0:(2*pi)/(length-1):2*pi;

if ~(strcmpi(type,'sine') |strcmpi(type,'triangle')|strcmpi(type,'square')|strcmpi(type,'sawtooth'))
    error('Input must be of type: sine, triangle, square or sawtooth')
end
    
if (strcmpi(type,'sine'))
    %analogWriteVector(10,128+digitalAmplitude*sin(2*pi*[0:digitalFrequency-1]./(digitalFrequency)));
    y = sin(x)+1;
end 
  
if (strcmpi(type,'triangle'))
    y = sawtooth(x,0.5) + 1;
end
(analogFrequency-c)./d;
if (strcmpi(type,'square'))
    %y = square(x)+1;
    y = sin(x) +sin(3*x)/3 + sin(5*x)/5;
end

if (strcmpi(type,'sawtooth'))
    y = sawtooth(x,1) + 1;
end

amplitude = digitalAmplitude;
y = y*redAmp(analogFrequency)*amplitude;
analogWriteVector(10, y);

plot(x,y);

end
