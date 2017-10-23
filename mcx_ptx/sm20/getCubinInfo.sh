#!/bin/bash

## extract all the cubin files from the program
cuobjdump mcx -xelf all

## dump the resource usage
cuobjdump mcx_core.sm_20.cubin -res-usage

### control flow graph
#nvdisasm -cfg mcx_core.sm_20.cubin  | dot -Tpng -omcx_core.sm_20.png

cuobjdump  mcx -ptx  > mcx_ptx
cuobjdump  mcx -sass  > mcx_sass
