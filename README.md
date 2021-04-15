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
* src
* docs
  * final presentation slides
  * final report doc 


## Authors
* Justin Hai
* Bob (Yufeng) Zhou
* Isamu Poy

## Acknowledgements
We would like to thank Xilinx for providing open source 3rd party IP cores (TEMAC, UARTLite, AXIS FIFO, etc.) which we have utilized in our design. We also like to thank Professor Chow for supplying the class with Xilinx FPGAs over the FPGA-net and the TAs for their continued support on Piazza.
