module shifter
    (
        input logic [31:0] in,
        input logic [4:0] shift,
        input logic right,
        output logic [31:0] out
    );

    /* verilator lint_off UNOPTFLAT */ 
    wire [31:0] intermediate[5:0];
    wire [31:0] zero = 32'd0;

    assign intermediate[0] = in;
    assign out = intermediate[5];

    generate
        genvar i;
        for (i = 0; i <= 4; i = i + 1) begin
            logic [31:0] left_shift = {intermediate[i][(31 - (2 ** i)):0], zero[(2 ** i) - 1:0]};
            logic [31:0] right_shift = {zero[(2 ** i) - 1:0], intermediate[i][31:(2 ** i)]};
            mux32 mux(
                .a(right ? right_shift : left_shift),
                .b({intermediate[i]}),
                .sel(shift[i]),
                .out({intermediate[i + 1]})
            );
        end
    endgenerate

endmodule

module mux32
    (
        input logic [31:0] a,
        input logic [31:0] b,
        input logic sel,
        output logic [31:0] out
    );
    
    assign out = sel ? a : b;

endmodule
