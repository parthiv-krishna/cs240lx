module shifter
    (
        input logic [31:0] in,
        input logic [4:0] shift,
        output logic [31:0] out
    );

    wire [31:0] shift1, shift2, shift4, shift8;

    mux32 m1(
        .a({in[30:0], 1'b0}),
        .b(in),
        .sel(shift[0]),
        .out(shift1)
    );

    mux32 m2(
        .a({shift1[29:0], 2'b0}),
        .b(shift1),
        .sel(shift[1]),
        .out(shift2)
    );

    mux32 m4(
        .a({shift2[27:0], 4'b0}),
        .b(shift2),
        .sel(shift[2]),
        .out(shift4)
    );

    mux32 m8(
        .a({shift4[23:0], 8'b0}),
        .b(shift4),
        .sel(shift[3]),
        .out(shift8)
    );

    mux32 m16(
        .a({shift8[15:0], 16'b0}),
        .b(shift8),
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
