function [ success ] = analogWrite (pin,value)
% analogWrite(pin,value) writes PWM value to pin
  [ rows cols ] = size (pin);
  if ((rows ~= 1) || (cols ~= 1)) 
    error('pin must be a scalar')
  end
  [ rows cols ] = size (value);
  if ((rows ~= 1) || (cols ~= 1)) 
    error('value must be a scalar')
  end
  if ((pin < 0)||(pin > 13))
    error('pin must be in the range 0 through 13');
  end
  if ((value < 0)||(value > 255))
    error('value must be in the range 0 through 255');
  end
  rc = arduinoIO([ 3 2 pin value]);
  if ((rc(1) ~= 3) || (rc(2) ~= 0))
    error('some thing bad has happened');
  end
  success = 1;
end
