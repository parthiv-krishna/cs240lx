// Count up to M and then assert max_tick for one cycle and reset
module counter
    #(
        parameter M = 10
    )
    (
        input  logic clk, rst,
        output logic max_tick
    );

    wire [31:0] count_curr;
    wire [31:0] count_next = (count_curr < M) ? (count_curr + 1) : 0;

    assign max_tick = (count_curr == (M - 1));

    dffr #(32) count(
        .clk(clk),
        .r(rst),
        .d(count_next),
        .q(count_curr)
    );

endmodule
