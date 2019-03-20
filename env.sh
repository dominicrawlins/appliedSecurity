export PATH="${PATH}:/opt/gcc-arm-none-eabi/7-2017-q4-major/bin"
export PATH="${PATH}:/opt/lpc21isp/bin"
export PATH="${PATH}:/opt/picoscope/bin"

which arm-none-eabi-gcc
which lpc21isp
which picoscope


cd ./scale-hw
export SCALE_HW="${PWD}"

cd ${SCALE_HW}/target/lpc1313fbd48

export TARGET="${PWD}"

make --no-builtin-rules clean all

cd ~/appliedSecurity
