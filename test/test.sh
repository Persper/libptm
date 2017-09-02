#! /bin/bash

export LD_LIBRARY_PATH=../gcc/x86_64-unknown-linux-gnu/libitm/.libs/

odir=/tmp

make -s
es=$?
if [ $es -ne 0 ]; then
  exit $es
fi

es=0

echo "Testing footprint method:"
export ITM_DEFAULT_METHOD=footprint
./footprint.o ./footprint.o > $odir/footprint.output 2>&1
if diff footprint.output $odir/footprint.output > /dev/null; then
  echo -e "\tPassed"
else
  echo -e "\tFailed"
  es=1
fi

exit $es
