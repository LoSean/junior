`define ADD 3'd0
`define SUB 3'd1
`define MUL 3'd2
`define AND 3'd3
`define OR 3'd4

module ALU(
	data1_i,
	data2_i,
	ALUCtrl_i,
	data_o,
	Zero_o
);

input [31:0] data1_i, data2_i;
input [2:0] ALUCtrl_i;
output [31:0] data_o;
output Zero_o;
reg [31:0] data_o;
reg Zero_o;

always @(*)
begin
case(ALUCtrl_i)
	`ADD: data_o = data1_i + data2_i;
	`SUB: data_o = data1_i - data2_i;
	`MUL: data_o = data1_i * data2_i;
	`AND: data_o = data1_i & data2_i;
	`OR: data_o = data1_i | data2_i;
	default: data_o = data1_i;
endcase
if (data_o == 32'd0) 
	Zero_o = 1'b1;
else
	Zero_o = 1'b0;
end

endmodule
