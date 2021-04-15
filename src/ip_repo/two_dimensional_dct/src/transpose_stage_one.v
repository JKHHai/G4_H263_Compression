`timescale 1ns / 1ps
/*
        Input: 2x2 matrix
        Visualize pipeline:
        P1: I1 - - - -  |    | - reg0 - O1
        P2: I2 - reg1 - |    | - - - -  O2
    
        ctrl:
        0 - Pipelines don't intersect (P1 directs to O1)
        1 - Pipelines intersect - (P1 directs to O2, vice versa)
        Ctrl switches every cycle
*/

module transpose_stage_one
    #(
        parameter VALUE_WIDTH = 17
    )
    (
        input i_clk,
        input i_aresetn,

        input i_enable,
        input [VALUE_WIDTH-1:0] i_in0,
        input [VALUE_WIDTH-1:0] i_in1,

        output o_enable,
        output [VALUE_WIDTH-1:0] o_out0,
        output [VALUE_WIDTH-1:0] o_out1
    );
    // Declarations
    // localparam STATE_IDLE = 2'd0;
    localparam STATE_UNSWITCHED = 2'd1;
    localparam STATE_SWITCHED = 2'd2;
    localparam DURATION = 1;
    reg [1:0] r_core_state;
    reg r_state_duration;
    // Declarations - Switch
    reg [VALUE_WIDTH-1:0] r_reg0;
    reg [VALUE_WIDTH-1:0] r_reg1;
    reg r_o_enable;
    wire w_select;
    wire [VALUE_WIDTH-1:0] w_switch_in0;
    wire [VALUE_WIDTH-1:0] w_switch_in1;
    wire [VALUE_WIDTH-1:0] w_switch_out0;
    wire [VALUE_WIDTH-1:0] w_switch_out1;

    // Assignments
    assign w_select = (r_core_state == STATE_SWITCHED) ? 1 : 0;
    assign w_switch_in0 = i_in0;
    assign w_switch_in1 = r_reg1;
    assign o_enable = r_o_enable;
    assign o_out0 = r_reg0;
    assign o_out1 = w_switch_out1;

    // Modules
    path_switch #(.VALUE_WIDTH(VALUE_WIDTH)) path_switcher
    (
        .i_select(w_select),
        .i_in0(w_switch_in0),
        .i_in1(w_switch_in1),
        .o_out0(w_switch_out0),
        .o_out1(w_switch_out1)
    );

    always @(posedge i_clk)
    begin
        if (i_aresetn == 1'b0)
        begin
            r_core_state <= STATE_UNSWITCHED;
            r_state_duration <= 0;
            r_reg0 <= 0;
            r_reg1 <= 0;
            r_o_enable <= 0;
        end
        else
        begin
            // r_reg0 <= (r_core_state != STATE_IDLE) ? w_switch_out0 : 0;
            // r_reg1 <= (r_core_state != STATE_IDLE) ? i_in1 : 0;
            r_reg0 <= w_switch_out0;
            r_reg1 <= i_in1;
            case (r_core_state)
                // STATE_IDLE:
                // begin
                //     r_core_state <= (i_enable == 1) ? STATE_UNSWITCHED : STATE_IDLE;
                //     r_o_enable <= (i_enable == 1) ? 1 : 0;
                // end
                STATE_UNSWITCHED:
                begin
                    if (i_enable == 1'b1)
                    begin    
                        if (r_state_duration == (DURATION-1))
                        begin
                            r_core_state <= STATE_SWITCHED;
                            r_o_enable <= 1'b1;
                        end
                        else
                        begin
                            r_core_state <= STATE_UNSWITCHED;
                            r_o_enable <= r_o_enable;
                        end
                        r_state_duration <= (r_state_duration == (DURATION-1)) ? 0 : r_state_duration + 1;
                    end
                    else
                    begin
                        r_core_state <= STATE_UNSWITCHED;
                        r_o_enable <= 0;
                        r_state_duration <= 0;
                    end
                end
                STATE_SWITCHED:
                begin
                    r_core_state <= (r_state_duration == (DURATION-1)) ? STATE_UNSWITCHED : STATE_SWITCHED;
                    r_state_duration <= (r_state_duration == (DURATION-1)) ? 0 : r_state_duration + 1;
                end
            endcase
        end
    end

endmodule