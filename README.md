# G4_H263_Compression
## What is it?
This repository contains the software and hardware necessary to create a media compression system using the H.263 algorithm. At the current state, the system only supports image compression but can be extended to video with image compensation. We hope that this project will serve as a basis for future media compression based designs
## How to use
1. Download or clone the repository from GitHub
2. Unzip any files necessary
3. Open the Vivado project "fifoStream" from src
4. Open command line and install pyserial and matplotlib libraries
5. Open Vivado SDK and run the software project
6. Set the ILA triggers for TVALID being high for run-length encoder
7. Go inside the src directory and run serial2.py, and input sample image cat.jpg when prompted
8. x-out of the GUI prompting for input, and x-out the pop up cat image
9. Observe the packets being sent over serial port, and check the trigger for the encoded data

## Repository Structure
Legend:  
**folder**  
*file.txt*  
**G4_H263_Compression:**  
* **doc:** Holds all documents including presentations, etc.  
  * *final_report.pdf*
  * *presentation_mid_project_demo.pdf:* Slides from mid-project demo.
  * *presentation_final_demo.pdf*: Slides from final demo. 
* README.MD
* **src:** Holds all Verilog, C and Python files used in project
  * **files:**
    * **fpga1:**
      * *cat.jpeg:* Test image
      * *fifo_mem.v:* FIFO used by frame generator
      * *frame_destructor_custom.v:* Converts Ethernet frames into AXI-Stream transfers
      * *frame_generator_custom.v:* Converts AXI-Stream transfers into Ethernet frames
      * *one_dimension_dct.v*
      * *quantizer.v*
      * *run_length_encoder.v*
      * *Serial2.py:* Serial communication
      * *transpose.v*
      * *two_dimension_dct.v*
      * *zig_zag.v*
    * **fpga2:**
      * *Helloworld.c:* Contains decompression functions and Microblaze flow
  * **ip_repo:**
    * **fpga1:**
      * **fpga1_deployment_receiving_block:** Receives AXI-Stream packets
      * **fpga1_deployment_source_block:** Sends AXI-Stream packets
      * **ip_frame_destructor_custom**
      * **ip_frame_generator_custom**
      * **ip_quantizer**
      * **ip_run_length_encoder**
      * **ip_transpose**
      * **ip_zig_zag**
      * **one_dimensional_dct**
      * **two_dimensional_dct**
  * **projects:** Holds all Vivado projects used to generate bitstreams/test cores
    * **fpga1**:
      * **fifoStream**: Contains top-level block design for FPGA 1
        * **fifoStream.sdk/streamFIFO/src/***helloworld.c*: Contains functions for connecting to PC 1 via UARTLite and sending data to   compression cores.
      * **proj_compression_deployment:** Used to test compression algorithm on FPGA.
      * **proj_frame_generator_custom:** Used to test in simulation.
      * **proj_frame_generator_custom_deployment:** Used to test core on FPGA.
      * **proj_frame_destructor_custom:** Used to test in simulation.
      * **proj_frame_destructor_custom_deployment:** Used to test core on FPGA.
      * **proj_one_dimensional_dct:** Used to test in simulation.
      * **proj_one_dimensional_dct_deployment:** Used to test core on FPGA.
      * **proj_quantizer:** Used to test in simulation.
      * **proj_quantizer_deployment:** Used to test core on FPGA.
      * **proj_run_length_encoder:** Used to test in simulation. 
      * **proj_run_length_encoder_deployment:** Used to test core on FPGA.
      * **proj_transpose:** Used to test in simulation.
      * **proj_transpose_deployment:** Used to test core on FPGA.
      * **proj_two_dimension_dct:** Used to test in simulation.
      * **proj_two_dimension_dct_deployment:** Used to test core on FPGA.
      * **proj_zig_zag:** Used to test in simulation.
      * **proj_zig_zag_deployment:** Used to test core on FPGA.
    * **fpga2:**
      * **proj_fpga_2_deployment:** Contains top-level block design for FPGA 2. 
  * **tb:** Holds all testbench files
    * **fpga1:**
      * *tb_frame_destructor_custom.sv*
      * *tb_frame_generator_custom.sv*
      * *tb_one_dimension_dct.sv*
      * *tb_path_switch.sv*
      * *tb_quantizer.sv*
      * *tb_run_length_encoder.sv*
      * *tb_transpose.sv*
      * *tb_two_dimension_dct.sv*
      * *tb_zig_zag.sv*

## Authors
* Justin Hai
* Bob (Yufeng) Zhou
* Isamu Poy

## Acknowledgements
We would like to thank Xilinx for providing open source 3rd party IP cores (TEMAC, UARTLite, AXIS FIFO, etc.) which we have utilized in our design. We also like to thank Professor Chow for supplying the class with Xilinx FPGAs over the FPGA-net and the TAs for their continued support on Piazza.
