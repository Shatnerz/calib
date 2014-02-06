function [ value ] = analogRead (pin)
% analogRead(pin) returns value read from analog to digital convertor pin
  [ rows cols ] = size (pin);
  if ((rows ~= 1) || (cols ~= 1)) 
    error('pin must be a single value')
  end
  if ((pin < 0)||(pin > 5))
    error('pin must be in the range 0 through 5');
  end
  rc = arduinoIO([ 2 1 pin ]);
  if ((rc(1) ~= 2) || (rc(2) ~= 1))
    error('some thing bad has happened');
  end
  value = rc(3);
end
