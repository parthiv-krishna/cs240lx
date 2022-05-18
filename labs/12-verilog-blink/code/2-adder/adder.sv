module adder
    (
        input logic [31:0] a, b,
        output logic [31:0] sum
    );

    // turn off linter
    /* verilator lint_off UNOPTFLAT */ 
    wire [31:0] carry;

    half_adder lsb(
        .a(a[0]),
        .b(b[0]),
        .s(sum[0]),
        .c(carry[0])
    );

    generate
        genvar i;
        for (i = 1; i < 32; i = i + 1) begin
            full_adder bit_u(
                .a(a[i]),
                .b(b[i]),
                .cin(carry[i-1]),
                .s(sum[i]),
                .cout(carry[i])
            );
        end
    endgenerate
endmodule

module half_adder
    (
        input logic a, b,
        output logic c, s
    );
    assign c = a & b;
    assign s = a ^ b;
endmodule

module full_adder
    (
        input logic a, b, cin,
        output logic cout, s
    );
    assign cout = (a & b) | (a & cin) | (b & cin);
    assign s = a ^ b ^ cin;
endmodule
