`include "pixel_image.sv"
module top;
  // Instancia a "imagem" com parâmetros padrão (largura x altura)
  pixel_image #(.W(4), .H(3)) u_img();
endmodule