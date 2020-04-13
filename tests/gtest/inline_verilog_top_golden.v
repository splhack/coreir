module coreir_term #(
    parameter width = 1
) (
    input [width-1:0] in
);

endmodule

module corebit_term (
    input in
);

endmodule

module RTLMonitor (
    input CLK,
    input handshake_arr_0_ready,
    input handshake_arr_0_valid,
    input handshake_arr_1_ready,
    input handshake_arr_1_valid,
    input handshake_arr_2_ready,
    input handshake_arr_2_valid,
    input handshake_ready,
    input handshake_valid,
    input [3:0] in1,
    input [3:0] in2,
    input intermediate_tuple__0,
    input intermediate_tuple__1,
    input mon_temp1,
    input mon_temp2,
    input out
);
wire [3:0] arr_2d_0;
wire [3:0] arr_2d_1;
assign arr_2d_0 = in1;
assign arr_2d_1 = in2;
corebit_term corebit_term_inst0 (
    .in(intermediate_tuple__0)
);
corebit_term corebit_term_inst1 (
    .in(arr_2d_0[1])
);
corebit_term corebit_term_inst2 (
    .in(handshake_valid)
);
coreir_term #(
    .width(4)
) term_inst0 (
    .in(arr_2d_0)
);
coreir_term #(
    .width(4)
) term_inst1 (
    .in(arr_2d_1)
);

logic temp1, temp2;
logic temp3;
assign temp1 = |(in1);
assign temp2 = &(in1) & intermediate_tuple__0;
assign temp3 = temp1 ^ temp2 & arr_2d_0[1];
assert property (@(posedge CLK) handshake_valid -> out === temp1 && temp2);
logic [3:0] temp4 [1:0];
assign temp4 = '{arr_2d_1, arr_2d_0};
                                   
endmodule

