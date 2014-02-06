function [ success ] = analogWriteVector (pin,values)
% analogWriteVector(pin,values) writes PWM vector of values to pin
  [ rows cols ] = size (pin);
  if ((rows ~= 1) || (cols ~= 1)) 
    error('pin must be a scalar')
  end
  [ rows cols ] = size (values);
  if (rows ~= 1)
    error('values must be a vector')
  end
  if ((pin < 0)||(pin > 13))
    error('pin must be in the range 0 through 13');
  end
  if (cols > 700)
    error('length of values vector must be no more than 700')
  end
  if ((min(values) < 0)||(max(values) > 255))
    error('values must be in the range 0 through 255');
  end
  rc = arduinoIO([ 6 cols+1 pin values]);
  if ((rc(1) ~= 6) || (rc(2) ~= 0))
    error('some thing bad has happened');
  end
  success = 1;
end
