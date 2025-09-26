module dff_with_logic(
  input  logic clk, rst_n, d,
  output logic q
);
  always @(posedge clk or negedge rst_n) begin
    if (!rst_n) q <= 1'b0;
    else        q <= d;
  end
endmodule
