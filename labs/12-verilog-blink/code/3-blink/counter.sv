// Output q will count up to M and then reset to 0.
module counter
    #(
        parameter M = 10
    )
    (
        input  logic clk, rst,
        output logic [$clog2(M)-1:0] q
    );

    logic [$clog2(M)-1:0] d = (q < M) ? (q + 1) : 0;

    always_ff @(posedge clk, posedge rst) begin
        if (rst) begin
            q <= 0;
        end else begin
            q <= d;
        end
    end

    
endmodule
