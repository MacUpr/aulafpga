module comb_with_wire(
  input  logic a, b,
  output logic y
);
  // NET (conexão física)
  wire and_ab;

  // Atribuições contínuas (característica de NETs)
  assign and_ab = a & b;
  assign y      = and_ab | b;
endmodule
