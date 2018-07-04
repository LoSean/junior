`define ADD 3'd0
`define SUB 3'd1
`define MUL 3'd2
`define AND 3'd3
`define OR 3'd4

module ALU_Control(
	funct_i,
	ALUOp_i,
	ALUCtrl_o
);

input [5:0] funct_i;
input [1:0] ALUOp_i;
output [2:0] ALUCtrl_o;
reg [2:0] ALUCtrl_o;

always @ (*)
begin
	if (ALUOp_i == 2'b00)
	begin
		if (funct_i == 6'b100000)
			ALUCtrl_o = `ADD;
		else if (funct_i == 6'b100010)
			ALUCtrl_o = `SUB;
		else if (funct_i == 6'b011000)
			ALUCtrl_o = `MUL;
		else if (funct_i == 6'b100100)
			ALUCtrl_o = `AND;
		else if (funct_i == 6'b100101)
			ALUCtrl_o = `OR;
		else
			ALUCtrl_o = 3'b000;
	end
	else if (ALUOp_i == 2'b01)
		ALUCtrl_o = `ADD;
	else
		ALUCtrl_o = 3'b000;
end

endmodule
