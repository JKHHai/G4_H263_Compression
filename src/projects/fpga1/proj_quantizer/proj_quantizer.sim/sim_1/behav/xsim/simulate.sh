#!/bin/bash -f
# ****************************************************************************
# Vivado (TM) v2018.3 (64-bit)
#
# Filename    : simulate.sh
# Simulator   : Xilinx Vivado Simulator
# Description : Script for simulating the design by launching the simulator
#
# Generated by Vivado on Wed Apr 14 23:06:42 EDT 2021
# SW Build 2405991 on Thu Dec  6 23:36:41 MST 2018
#
# Copyright 1986-2018 Xilinx, Inc. All Rights Reserved.
#
# usage: simulate.sh
#
# ****************************************************************************
ExecStep()
{
"$@"
RETVAL=$?
if [ $RETVAL -ne 0 ]
then
exit $RETVAL
fi
}
ExecStep xsim tb_quantizer_behav -key {Behavioral:sim_1:Functional:tb_quantizer} -tclbatch tb_quantizer.tcl -view /home/justin/jhai/universityoftoronto/ECE532/final_project/G4_H263_Compression/src/projects/fpga1/proj_quantizer/tb_quantizer_behav.wcfg -log simulate.log