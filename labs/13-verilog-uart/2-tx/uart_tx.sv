`define STATE_IDLE 2'b00
`define STATE_START 2'b01
`define STATE_DATA 2'b10
`define STATE_STOP 2'b11
`define OVERSAMPLE 16
`define OS_WIDTH 4
`define SAMPLE_IDX 7


module uart_tx
    (
        input logic clk, rst,
        input logic tx_start,      // pulse one tick when transmission should begin
        input logic tick,          // baud rate oversampled tick
        input logic [7:0] din,     // data to send (user must keep data valid
                                   // at least until tx_done_tick is asserted)
        output logic tx_done_tick, // pulse one tick when done
        output logic tx            // serial data
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
    
    wire [3:0] bit_count_curr;
    wire [3:0] bit_count_next = (state_curr == `STATE_DATA) ? bit_count_curr + 1 : 0;
    dffre #(4) bit_count_ff (
        .clk(clk),
        .r(rst || (state_curr == `STATE_START)),
        .en((state_curr == `STATE_DATA) && bit_done && tick),
        .d(bit_count_next),
        .q(bit_count_curr)
    );

    wire [7:0] tx_wide = (state_curr == `STATE_DATA) ? (din >> bit_count_curr) : {7'b0, (state_curr != `STATE_START)};
    wire tx_next = tx_wide[0];
    dffre tx_ff (
        .clk(clk),
        .r(rst || (state_curr == `STATE_START)),
        .en(tick_count_curr == 0),
        .d(tx_next),
        .q(tx)
    );

    always_comb begin
        case (state_curr)
            `STATE_IDLE: begin
                state_next = tx_start ? `STATE_START : `STATE_IDLE;
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

    assign tx_done_tick = (state_curr == `STATE_STOP) && bit_done;
    
endmodule
