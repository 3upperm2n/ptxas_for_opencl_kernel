#!/bin/bash

#ptxas -m64 -arch=sm_30  --opt-level 3 --fmad true -v -warn-spills -Werror mcx_core_30.ptx -o mcx_core_30.cubin
ptxas -m64 -arch=sm_30 -warn-spills -fmad=true -O3 mcx_core_30.ptx -o mcx_core_30.cubin


cuobjdump  mcx_core_30.cubin  -sass > mcx_core_30_sass
