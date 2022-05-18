module shifter
    (
        input logic [31:0] in,
        input logic [4:0] shift,
        output logic [31:0] out
    );

    /* verilator lint_off UNOPTFLAT */ 
    wire [31:0] intermediate[3:0];

    mux32 m1(
        .a({in[30:0], 1'b0}),
        .b(in),
        .sel(shift[0]),
        .out(intermediate[0])
    );

    mux32 m2(
        .a({intermediate[0][29:0], 2'b0}),
        .b(intermediate[0]),
        .sel(shift[1]),
        .out(intermediate[1])
    );

    mux32 m4(
        .a({intermediate[1][27:0], 4'b0}),
        .b(intermediate[1]),
        .sel(shift[2]),
        .out(intermediate[2])
    );

    mux32 m8(
        .a({intermediate[2][23:0], 8'b0}),
        .b(intermediate[2]),
        .sel(shift[3]),
        .out(intermediate[3])
    );

    mux32 m16(
        .a({intermediate[3][15:0], 16'b0}),
        .b(intermediate[3]),
        .sel(shift[4]),
        .out(out)
    );

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
