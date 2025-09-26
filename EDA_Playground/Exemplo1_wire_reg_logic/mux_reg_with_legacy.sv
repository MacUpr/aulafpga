module mux_with_reg_legacy(
  input  logic a, b, sel,
  output reg   y
);
  // 'reg' é variável procedural (legado).
  always @* begin
    if (sel) y = a;
    else     y = b;
  end
endmodule
