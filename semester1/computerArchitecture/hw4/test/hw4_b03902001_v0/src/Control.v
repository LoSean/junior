module Control(
	Op_i,
	RegDst_o,
	ALUOp_o,
	ALUSrc_o,
	RegWrite_o
);

input [5:0] Op_i;
output RegDst_o, ALUSrc_o, RegWrite_o;
output [1:0] ALUOp_o; 

assign RegDst_o = (Op_i == 6'b000000)? 1'b1:
		  (Op_i == 6'b001000)? 1'b0:
		  1'b0;

assign ALUSrc_o = (Op_i == 6'b000000)? 1'b0:
		  (Op_i == 6'b001000)? 1'b1:
		  1'b0;

assign RegWrite_o = (Op_i == 6'b000000)? 1'b1:
		    (Op_i == 6'b001000)? 1'b1:
		    1'b0;

assign ALUOp_o = (Op_i == 6'b000000)? 2'b00:
		 (Op_i == 6'b001000)? 2'b01:
		 2'b00;
endmodule
