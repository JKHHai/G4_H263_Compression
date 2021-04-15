/******************************************************************************
*
* Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

/*
 * helloworld.c: simple test application
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 */

/***************************** Include Files *********************************/
#include "xparameters.h"
#include "xil_exception.h"
#include "xstreamer.h"
#include "xil_cache.h"
#include "xllfifo.h"
#include <xuartlite.h>
#include "xstatus.h"
//#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "platform.h"
#include "xil_printf.h"

/***************** Macros (Inline Functions) Definitions *********************/
#define PI 3.1415926
#define BLOCK_SIZE 8
#define SCALE 7
#define VALUE_WIDTH 10
#define NON_VALUE_WIDTH 32-VALUE_WIDTH

#define STATE_COLLECT_INPUT 0
#define STATE_DECODE_BITSTREAM 3
#define STATE_COMPUTATION 1
#define STATE_OUTPUT_TRANSMISSION 2
#define STATE_IDLE 4

#define DECODER_VALUE(X) (((X) << NON_VALUE_WIDTH) >> NON_VALUE_WIDTH)
#define DECODER_VALUE_MSB(X) (DECODER_VALUE((X))>>(VALUE_WIDTH-1))
#define DECODER_VALUE_PADDING(X) ((DECODER_VALUE_MSB((X)) == 1) ? (0xFFFFFFFF << VALUE_WIDTH) : 0)

#define MODE_ZZ_DIAGONAL 0
#define MODE_ZZ_VERTICAL 1 // zig-zag is currently located at a vertical wall (eg. j = 0 or j = BLOCK_SIZE - 1) and therefore the next move must be down
#define MODE_ZZ_HORIZONTAL 2 // zig-zag is currently located at a horizontal wall (eg. i = 0 or i = BLOCK_SIZE - 1) and therefore the next move must be to the right

#define FIFO_DEV_ID	   	XPAR_AXI_FIFO_0_DEVICE_ID
#define UARTLITE_DEV_ID 	XPAR_UARTLITE_0_DEVICE_ID
#define WORD_SIZE 4			/* Size of words in bytes */
#define MAX_PACKET_LEN BLOCK_SIZE*BLOCK_SIZE
#define NO_OF_PACKETS 64
#define MAX_DATA_BUFFER_SIZE NO_OF_PACKETS*MAX_PACKET_LEN

/************************** Function Prototypes ******************************/
struct Tuple
{
    int32_t x_l;
    int32_t x_l_other_end;
};
int setup_fifo(XLlFifo *InstancePtr, u16 DeviceId);
void setup_quantization_table(int32_t* quantization_table);
int RxReceive(XLlFifo *InstancePtr, u32 *encoded_bitstream);
void run_length_decoder(u32 *encoded_bitstream, int32_t *quantized_matrix);
void de_zig_zag(int32_t* bitstream, int32_t* dct_coeffs);
void dequantizer(int32_t *quantized_matrix, int32_t *quantization_table, int32_t *dequantized_matrix);
void transpose_simple(int32_t* input_matrix, int32_t* transposed_matrix);
void compute_inverse_al_for_ul(int32_t* al_values);
void compute_inverse_al_for_vl(int32_t* al_values);
struct Tuple compute_inverse_dct_coeff(int l, int32_t* al_values_for_ul, int32_t* al_values_for_vl, int32_t* ym_values);
void compute_one_dimensional_inverse_dct(int32_t* al_values_for_ul, int32_t* al_values_for_vl, int32_t* yl_values, int32_t* xm_values);
void compute_inverse_dct(int32_t* al_values_for_ul, int32_t* al_values_for_vl, int32_t* dct_coeffs, int32_t* image);


int main()
{
    init_platform();
    int i;
    int j;
	// Setup
    extern char start_of_HEAP;
	xil_printf("%X\n",&start_of_HEAP);
    // Setup - FIFO
	XLlFifo FifoInstance;
	int fifo_status = setup_fifo(&FifoInstance, FIFO_DEV_ID);
    // Setup - Quantization Table
	int32_t* quantization_table = (int32_t*) malloc(MAX_PACKET_LEN*sizeof(int32_t));
	setup_quantization_table(quantization_table);
    // Setup - Inverse DCT Constants
    int32_t* al_values_for_ul = (int32_t*) malloc(BLOCK_SIZE/2 * BLOCK_SIZE/2 * sizeof(int32_t));
    int32_t* al_values_for_vl = (int32_t*) malloc(BLOCK_SIZE/2 * BLOCK_SIZE/2 * sizeof(int32_t));
    compute_inverse_al_for_ul(al_values_for_ul);
    compute_inverse_al_for_vl(al_values_for_vl);
    // Setup - UART
    XUartLite myUart;
    u32 uart_status;
    int num_bytes_sent;
    char image_string[WORD_SIZE+1];
    image_string[WORD_SIZE] = '\n';
//    uart_status = XUartLite_Initialize(&myUart, UARTLITE_DEV_ID);
//    uart_status = XUartLite_SelfTest(&myUart);

    // Instantiate Data Values
    int core_state = STATE_COLLECT_INPUT;
    int rx_status;
    u32 fifo_output_length;
    u32* encoded_bitstream = (u32*) malloc(MAX_PACKET_LEN*sizeof(u32));
    int value;
    int frequency;
    int curr_pos_bitstream = 0;
    int curr_pos_fifo = 1;
	int32_t* zig_zagged_matrix = (int32_t*) malloc(MAX_PACKET_LEN*sizeof(int32_t));
	int32_t* quantized_matrix = (int32_t*) malloc(MAX_PACKET_LEN*sizeof(int32_t));
	int32_t* dequantized_matrix = (int32_t*) malloc(MAX_PACKET_LEN*sizeof(int32_t));
	int32_t* image = (int32_t*) malloc(MAX_PACKET_LEN*sizeof(int32_t));
	for (i=0; i<MAX_PACKET_LEN; i++)
	{
		encoded_bitstream[i] = 0;
		quantized_matrix[i] = 0;
		dequantized_matrix[i] = 0;
		zig_zagged_matrix[i] = 0;
        image[i] = 0;
	}

	while(1)
    {
        switch(core_state)
        {
            case STATE_COLLECT_INPUT:
//                RxReceive(&FifoInstance, encoded_bitstream);
            	encoded_bitstream[0] = 4;
            	encoded_bitstream[1] = 0b0000010000010000;
            	encoded_bitstream[2] = 0b0000011111110010;
            	encoded_bitstream[3] = 0b0000011111111110;
            	encoded_bitstream[4] = 0b1111010000000000;
                fifo_output_length = encoded_bitstream[0];
                value = DECODER_VALUE(encoded_bitstream[1]) | DECODER_VALUE_PADDING(encoded_bitstream[1]);
                frequency = encoded_bitstream[1] >> VALUE_WIDTH;
//                for (i=0; i<fifo_output_length; i++)
//                {
//                	xil_printf("Received Data: 0x%x\n", encoded_bitstream[i+1]);
//                }
                core_state = STATE_DECODE_BITSTREAM;
                break;
            case STATE_DECODE_BITSTREAM:
                zig_zagged_matrix[curr_pos_bitstream] = value;
                curr_pos_bitstream += 1;
                frequency -= 1;
                if (curr_pos_bitstream == MAX_PACKET_LEN) // Fully constructed bitstream
                {
                    curr_pos_bitstream = 0;
                    core_state = STATE_COMPUTATION;
                }
                else if (frequency == 0 && curr_pos_fifo == fifo_output_length) // At end of FIFO
                {
                    curr_pos_fifo = 1;
                    frequency = 0;
                    value = 0;
                    for (i=0; i<fifo_output_length+1; i++)
                    {
                        encoded_bitstream[i] = 0;
                    }
                    core_state = STATE_COLLECT_INPUT;
                }
                else if (frequency == 0)
                {
                    curr_pos_fifo += 1;
                    value = DECODER_VALUE(encoded_bitstream[curr_pos_fifo]) | DECODER_VALUE_PADDING(encoded_bitstream[curr_pos_fifo]);
                    frequency = encoded_bitstream[curr_pos_fifo] >> VALUE_WIDTH;
                    core_state = STATE_DECODE_BITSTREAM;
                }
                else
                {
                    core_state = STATE_DECODE_BITSTREAM;
                }
//                xil_printf("curr_pos_bitstream: %d, curr_pos_fifo: %d, frequency: %d, next state: %d\n", curr_pos_bitstream, curr_pos_fifo, frequency, core_state);
                break;
            case STATE_COMPUTATION:
                xil_printf("Computation!\n");
                 for (i=0; i<MAX_PACKET_LEN; i++)
				 {
				 	xil_printf("Zig-Zagged Value %d: %d\n", i, zig_zagged_matrix[i]);
				 }
                 de_zig_zag(zig_zagged_matrix, quantized_matrix);
            	 for (i=0; i<MAX_PACKET_LEN; i++)
            	 {
            	 	xil_printf("De-Zig-Zagged Value %d: %d\n", i, quantized_matrix[i]);
            	 }
                 dequantizer(quantized_matrix, quantization_table, dequantized_matrix);
                 for (i=0; i<MAX_PACKET_LEN; i++)
            	 {
            	 	xil_printf("Dequantized Value %d: %d\n", i, dequantized_matrix[i]);
            	 }
                 compute_inverse_dct(al_values_for_ul, al_values_for_vl, dequantized_matrix, image);
                 for (i=0; i<MAX_PACKET_LEN; i++)
                 {
                     xil_printf("Image Value %d: %d\n", i, image[i]>>SCALE);
                 }
                core_state = STATE_OUTPUT_TRANSMISSION;
                break;
            case STATE_OUTPUT_TRANSMISSION:
            	for (i=0; i<MAX_PACKET_LEN; i++)
            	{
//                    itoa(image[i], image_string, 16);
//                    image_string[WORD_SIZE] = '\n';
                    num_bytes_sent = XUartLite_Send(&myUart, image[i], WORD_SIZE);
                    while(XUartLite_IsSending(&myUart)) {}
                    num_bytes_sent = XUartLite_Send(&myUart, '\n', 1);
                    while(XUartLite_IsSending(&myUart)) {}
            	}
                core_state = STATE_IDLE;
                break;
        }
    }
	
	// Cleanup
	free(quantization_table);
	free(al_values_for_ul);
	free(al_values_for_vl);
    free(encoded_bitstream);
    free(zig_zagged_matrix);
	free(quantized_matrix);
	free(dequantized_matrix);
	free(image);
    cleanup_platform();
    return 0;
}

int setup_fifo(XLlFifo *InstancePtr, u16 DeviceId)
{
	int Status;
	Status = XST_SUCCESS;
	
	XLlFifo_Config *Config = XLlFfio_LookupConfig(DeviceId);
	if (!Config) {
		xil_printf("No config found for %d\r\n", DeviceId);
		return XST_FAILURE;
	}

	Status = XLlFifo_CfgInitialize(InstancePtr, Config, Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		xil_printf("Initialization failed\n\r");
		return Status;
	}

	/* Check for the Reset value */
	Status = XLlFifo_Status(InstancePtr);
	XLlFifo_IntClear(InstancePtr,0xffffffff);
	Status = XLlFifo_Status(InstancePtr);
	if(Status != 0x0) {
		xil_printf("\n ERROR : Reset value of ISR0 : 0x%x\t"
			    "Expected : 0x0\n\r",
			    XLlFifo_Status(InstancePtr));
		return XST_FAILURE;
	}

	return Status;
}

void setup_quantization_table(int32_t* quantization_table)
{
	quantization_table[0]  = 16;
	quantization_table[1]  = 11;
	quantization_table[2]  = 10;
	quantization_table[3]  = 16;
	quantization_table[4]  = 24;
	quantization_table[5]  = 40;
	quantization_table[6]  = 51;
	quantization_table[7]  = 61;
	quantization_table[8]  = 12;
	quantization_table[9]  = 12;
	quantization_table[10] = 14;
	quantization_table[11] = 19;
	quantization_table[12] = 26;
	quantization_table[13] = 58;
	quantization_table[14] = 60;
	quantization_table[15] = 55;
	quantization_table[16] = 14;
	quantization_table[17] = 13;
	quantization_table[18] = 16;
	quantization_table[19] = 24;
	quantization_table[20] = 40;
	quantization_table[21] = 57;
	quantization_table[22] = 69;
	quantization_table[23] = 56;
	quantization_table[24] = 14;
	quantization_table[25] = 17;
	quantization_table[26] = 22;
	quantization_table[27] = 29;
	quantization_table[28] = 51;
	quantization_table[29] = 87;
	quantization_table[30] = 80;
	quantization_table[31] = 62;
	quantization_table[32] = 18;
	quantization_table[33] = 22;
	quantization_table[34] = 37;
	quantization_table[35] = 56;
	quantization_table[36] = 68;
	quantization_table[37] = 109;
	quantization_table[38] = 103;
	quantization_table[39] = 77;
	quantization_table[40] = 24;
	quantization_table[41] = 35;
	quantization_table[42] = 55;
	quantization_table[43] = 64;
	quantization_table[44] = 81;
	quantization_table[45] = 104;
	quantization_table[46] = 113;
	quantization_table[47] = 92;
	quantization_table[48] = 49;
	quantization_table[49] = 64;
	quantization_table[50] = 78;
	quantization_table[51] = 87;
	quantization_table[52] = 103;
	quantization_table[53] = 121;
	quantization_table[54] = 120;
	quantization_table[55] = 101;
	quantization_table[56] = 72;
	quantization_table[57] = 92;
	quantization_table[58] = 95;
	quantization_table[59] = 98;
	quantization_table[60] = 112;
	quantization_table[61] = 100;
	quantization_table[62] = 103;
	quantization_table[63] = 99;
}

/*
 Function is blocking: Will only work when it has received a packet on the FIFO
 Once a packet is found, it will read a single packet (referenced by TLAST)

 */
int RxReceive(XLlFifo *InstancePtr, u32 *bitstream)
{

	int i;
	int Status;
	u32 RxWord;
	u32 ReceiveLength;
	
    while (XLlFifo_iRxOccupancy(InstancePtr) == 0); // Can be read multiple times without resetting
	xil_printf(" Receiving data ....\n\r");
    ReceiveLength = (XLlFifo_iRxGetLen(InstancePtr))/WORD_SIZE;
    xil_printf("FIFO Length: %d\n", ReceiveLength);
    bitstream[0] = ReceiveLength;
    for (i=0; i < ReceiveLength; i++) {
        RxWord = XLlFifo_RxGetWord(InstancePtr);
        bitstream[i+1] = RxWord;
    }

	Status = XLlFifo_IsRxDone(InstancePtr);
	if(Status != TRUE){
		xil_printf("Failing in receive complete ... \r\n");
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

void run_length_decoder(u32* encoded_bitstream, int32_t* quantized_matrix)
{
    int i;
    int j;
    int curr_pos_bitstream = 0;
	int value;
    int frequency;
    for (i = 0; i < BLOCK_SIZE * BLOCK_SIZE; i += 1)
    {
		value = DECODER_VALUE(encoded_bitstream[i]) | DECODER_VALUE_PADDING(encoded_bitstream[i]);
		frequency = encoded_bitstream[i] >> VALUE_WIDTH;
        for (j = 0; j < frequency; j++)
        {
			quantized_matrix[curr_pos_bitstream + j] = value;
        }
        curr_pos_bitstream += frequency;
		if (curr_pos_bitstream == BLOCK_SIZE*BLOCK_SIZE)
		{
			break;
		}
    }
}

void de_zig_zag(int32_t* bitstream, int32_t* dct_coeffs)
{
    /*
        Zigzagging implemented using an FSM

    */
    int i = 0;
    int j = 0;
    int k = 0;
    int mode = MODE_ZZ_HORIZONTAL;
    /*
        Direction:
        | 1 2 3 4 |
        | 5 6 7 8 |
        | 9 A B C |
        | D E F 0 |
        1 represents moving in direction towards top right (eg. 6->3)
        -1 represents moving in direction towards bottom left (eg. 6->9)
    */
    int direction = 1;
    while (1)
    {
//    	xil_printf("i: %d, j: %d, direction: %d, k: %d, bitstream[k]: %d\n", i, j, direction, k, bitstream[k]);
        dct_coeffs[8*i + j] = bitstream[k];
        k += 1;
        if (i == BLOCK_SIZE-1 && j == BLOCK_SIZE-1)
        {
            break;
        }
        switch(mode)
        {
            case MODE_ZZ_DIAGONAL:
                i -= direction;
                j += direction;
                if (i == 0 || i == BLOCK_SIZE-1) // Prioritize horizontal mode first, because at all corners we want to move horizontally
                {
                    mode = MODE_ZZ_HORIZONTAL;
                }
                else if (j == 0 || j == BLOCK_SIZE-1)
                {
                    mode = MODE_ZZ_VERTICAL;
                }
                break;
            case MODE_ZZ_VERTICAL:
                /*
                    Vertical wall:
                    | 1 2 3 4 |
                    | 5 6 7 8 |
                    | 9 A B C |
                    | D E F 0 |
                    For instance, when 2 -> 5, we hit a vertical wall. In this case, we must then transition down to 9 and reverse the diagonal direction, before proceeding diagonally again
                */
                i += 1;
                direction = direction * -1;
                mode = MODE_ZZ_DIAGONAL;
                break;
            case MODE_ZZ_HORIZONTAL:
                /*
                    Horizontal wall:
                    | 1 2 3 4 |
                    | 5 6 7 8 |
                    | 9 A B C |
                    | D E F 0 |
                    For instance, when 6 -> 3, we hit a vertical wall. In this case, we must then transition right to 4 and reverse the diagonal direction, before proceeding diagonally again
                */
                j += 1;
                direction = direction * -1;
                mode = MODE_ZZ_DIAGONAL;
                break;
        }
    }
}

void dequantizer(int32_t *quantized_matrix, int32_t *quantization_table, int32_t *dequantized_matrix)
{
	int i;
	for (i=0; i<BLOCK_SIZE*BLOCK_SIZE; i++)
	{
		dequantized_matrix[i] = quantized_matrix[i] * quantization_table[i];
	}
}

void transpose_simple(int32_t* input_matrix, int32_t* transposed_matrix)
{
    int i;
    int j;
    for (i=0; i<BLOCK_SIZE; i++)
    {
        for(j=0;j<BLOCK_SIZE; j++)
        {
            transposed_matrix[8*i+j] = input_matrix[8*j+i];
        }
    }
}

void compute_inverse_al_for_ul(int32_t* al_values)
{
    /*
        Format: al_values[4*L+M]
        Each al_value is fixed point with Scale of 7
    */
    al_values[4*0+0] = 45;
    al_values[4*0+1] = 59;
    al_values[4*0+2] = 45;
    al_values[4*0+3] = 24;
    al_values[4*1+0] = 45;
    al_values[4*1+1] = 24;
    al_values[4*1+2] = 45;
    al_values[4*1+3] = -59;
    al_values[4*2+0] = 45;
    al_values[4*2+1] = -24;
    al_values[4*2+2] = -45;
    al_values[4*2+3] = 59;
    al_values[4*3+0] = 45;
    al_values[4*3+1] = -59;
    al_values[4*3+2] = 45;
    al_values[4*3+3] = -24;
}

void compute_inverse_al_for_vl(int32_t* al_values)
{
    /*
        Format: al_values[4*L+M]
        Each al_value is fixed point with Scale of 7
    */
    al_values[4*0+0] = 63;
    al_values[4*0+1] = 53;
    al_values[4*0+2] = 36;
    al_values[4*0+3] = 12;
    al_values[4*1+0] = 53;
    al_values[4*1+1] = -12;
    al_values[4*1+2] = -63;
    al_values[4*1+3] = -36;
    al_values[4*2+0] = 36;
    al_values[4*2+1] = -63;
    al_values[4*2+2] = 12;
    al_values[4*2+3] = 53;
    al_values[4*3+0] = 12;
    al_values[4*3+1] = -36;
    al_values[4*3+2] = 53;
    al_values[4*3+3] = -63;
}

struct Tuple compute_inverse_dct_coeff(int l, int32_t* al_values_for_ul, int32_t* al_values_for_vl, int32_t* ym_values)
{
    // 2. Compute u and v
    int32_t u = 0;
    int32_t v = 0;
    int m;
    for (m = 0; m < BLOCK_SIZE / 2; m++)
    {
        u += (al_values_for_ul[4*l+m] * ym_values[2*m]) >> SCALE;
        v += (al_values_for_vl[4*l+m] * ym_values[2*m + 1]) >> SCALE;
    }
    // printf("l=%d u_l: %d\n", l, u);
    // printf("l=%d v_l: %d\n", l, v);
    // 3. Compute coefficients
    int correction_value = 1;
    int32_t x_l = correction_value * (u + v);
    int32_t x_l_other_end = correction_value * (u - v);
    // printf("x_l: %.2f\n", x_l);
    // printf("x_l_other: %.2f\n", x_l_other_end);
    struct Tuple pixels = {x_l, x_l_other_end};
    return pixels;
}

void compute_one_dimensional_inverse_dct(int32_t* al_values_for_ul, int32_t* al_values_for_vl, int32_t* yl_values, int32_t* xm_values)
{
    int l;
    struct Tuple pixels;
    for (l = 0; l < BLOCK_SIZE / 2; l++)
    {
        pixels = compute_inverse_dct_coeff(l, al_values_for_ul, al_values_for_vl, yl_values);
        xm_values[l] = pixels.x_l;
        xm_values[BLOCK_SIZE-1-l] = pixels.x_l_other_end;
    }
}

void compute_inverse_dct(int32_t* al_values_for_ul, int32_t* al_values_for_vl, int32_t* dct_coeffs, int32_t* image)
{
   // Input: matrix with BLOCK_SIZE x BLOCK_SIZE coefficients
   // Output: matrix with BLOCK_SIZE x BLOCK_SIZE pixels
   // i represents the rows, j represents the columns
   int i;
   int j;
   // 1. Perform 1-D IDCT on the rows of the coefficients
   int32_t* xm_values = (int32_t*) malloc(BLOCK_SIZE * sizeof(int32_t));
   int32_t* yl_values = (int32_t*) malloc(BLOCK_SIZE * sizeof(int32_t));
   int32_t* idct_stage_one = (int32_t*) malloc(BLOCK_SIZE * BLOCK_SIZE * sizeof(int32_t));
   for (i = 0; i < BLOCK_SIZE; i++)
   {
       for (j = 0; j < BLOCK_SIZE; j++)
       {
           // Convert to Fixed Point
           yl_values[j] = dct_coeffs[8*i + j] << SCALE;
        //    printf("yl_value %d %d: %d\n", i, j, yl_values[j]);
       }
       compute_one_dimensional_inverse_dct(al_values_for_ul, al_values_for_vl, yl_values, xm_values);
       for (j = 0; j < BLOCK_SIZE; j++)
       {
           idct_stage_one[8*i + j] = xm_values[j];
       }
   }
//    for(int k=0; k<BLOCK_SIZE*BLOCK_SIZE; k++)
//     {
//         printf ("1D DCT Coefficient %d: %d\n", k, idct_stage_one[k]);
//     }
   // 2. Transpose the coefficients
   int32_t* transposed_matrix = (int32_t*) malloc(BLOCK_SIZE * BLOCK_SIZE * sizeof(int32_t));
   transpose_simple(idct_stage_one, transposed_matrix);
   // 3. Perform the 1-D DCT again
   for (i = 0; i < BLOCK_SIZE; i++)
   {
       for (j = 0; j < BLOCK_SIZE; j++)
       {
           yl_values[j] = transposed_matrix[8*i + j];
       }
       compute_one_dimensional_inverse_dct(al_values_for_ul, al_values_for_vl, yl_values, xm_values);
       for (j = 0; j < BLOCK_SIZE; j++)
       {
           image[8*i + j] = xm_values[j];
       }
   }
   free(xm_values);
   free(yl_values);
   free(idct_stage_one);
   free(transposed_matrix);
}
