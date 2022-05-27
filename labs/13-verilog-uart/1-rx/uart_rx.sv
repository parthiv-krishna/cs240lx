`define STATE_IDLE 2'b00
`define STATE_START 2'b01
`define STATE_DATA 2'b10
`define STATE_STOP 2'b11
`define OVERSAMPLE 16
`define OS_WIDTH 4
`define SAMPLE_IDX 7


module uart_rx
    (
        input logic clk, rst,
        input logic rx,            // serial data
        input logic tick,          // baud rate oversampled tick
        output logic rx_done_tick, // pulse one tick when done
        output logic [7:0] dout    // output data
    );

    /* verilator public_module */

    
    wire [1:0] state_curr;
    reg [1:0] state_next;


    dffre #(2) state_ff (
        .clk(clk),
        .r(rst),
        .en(tick),
        .d(state_next),
        .q(state_curr)
    );


    wire [`OS_WIDTH-1:0] tick_count_curr;
    wire [`OS_WIDTH-1:0] tick_count_next = tick_count_curr + 1;
    dffre  #(`OS_WIDTH) tick_count_ff (
        .clk(clk),
        .r(rst || (state_curr == `STATE_IDLE)),
        .en((state_curr != `STATE_IDLE) && tick),
        .d(tick_count_next),
        .q(tick_count_curr)
    );

    wire bit_done = (tick_count_curr == {`OS_WIDTH{1'd1}});
    wire do_sample = (tick_count_curr == `OS_WIDTH'd`SAMPLE_IDX);
    
    wire [3:0] bit_count_curr;
    wire [3:0] bit_count_next = (state_curr == `STATE_DATA) ? bit_count_curr + 1 : 0;
    dffre #(4) bit_count_ff (
        .clk(clk),
        .r(rst || (state_curr == `STATE_START)),
        .en((state_curr == `STATE_DATA) && bit_done && tick),
        .d(bit_count_next),
        .q(bit_count_curr)
    );

    wire [7:0] dout_next = dout | ({7'b0, rx} << bit_count_curr);
    dffre #(8) dout_ff (
        .clk(clk),
        .r(rst || (state_curr == `STATE_START)),
        .en(do_sample),
        .d(dout_next),
        .q(dout)
    );

    always_comb begin
        case (state_curr)
            `STATE_IDLE: begin
                state_next = rx ? `STATE_IDLE : `STATE_START;
            end

            `STATE_START: begin
                state_next = bit_done ? `STATE_DATA : `STATE_START;
            end

            `STATE_DATA: begin
                state_next = (bit_done && bit_count_curr == 7) ? `STATE_STOP : `STATE_DATA;
            end

            `STATE_STOP: begin
                state_next = bit_done ? `STATE_IDLE : `STATE_STOP;
            end

        endcase
    end

    assign rx_done_tick = (state_curr == `STATE_STOP) && do_sample;
    
endmodule
