#!/bin/bash

#ptxas -m64 -arch=sm_30  --opt-level 3 --fmad true -v -warn-spills -Werror mcx_core_30.ptx -o mcx_core_30.cubin
ptxas -m64 -arch=sm_30 -warn-spills -fmad=true -O3 mcx_core-sm30.ptx -o mcx_core-sm30.ptx.cubin

cuobjdump mcx_core-sm30.ptx.cubin -sass > mcx_core-sm30_sass
