// ========================================
// testbench.sv
// ========================================

module tb;
  top dut();
  
  // Todas as variáveis declaradas no topo (compatível com Icarus)
  logic [7:0] r, g, b;
  logic [7:0] mag_r, mag_g, mag_b;
  logic [23:0] magenta24;
  bit ok;
  
  initial begin
    $dumpfile("waveform.vcd");
    $dumpvars(0, tb);
    
    // 1) Preenche a imagem com azul
    dut.u_img.fill(8'h00, 8'h00, 8'hFF);  // BLUE
    dut.u_img.print_image("FILL BLUE");
    
    // 2) Define alguns pixels coloridos
    dut.u_img.set_pixel(0, 0, 8'hFF, 8'h00, 8'h00);  // RED
    dut.u_img.set_pixel(1, 0, 8'h00, 8'hFF, 8'h00);  // GREEN
    dut.u_img.set_pixel(2, 0, 8'hFF, 8'hFF, 8'hFF);  // WHITE
    dut.u_img.set_pixel(3, 2, 8'h80, 8'h80, 8'h00);  // amarelo escuro
    dut.u_img.print_image("AFTER SET_PIXEL");
    
    // 3) Lê um pixel específico e mostra campos individuais
    dut.u_img.get_pixel(2, 0, r, g, b, ok);
    if (ok) $display("Pixel(2,0): R=0x%02h G=0x%02h B=0x%02h  (packed=0x%06h)", r, g, b, dut.u_img.pack24(r, g, b));
    
    // 4) Inverte todas as cores
    dut.u_img.invert();
    dut.u_img.print_image("AFTER INVERT");
    
    // 5) Converte para tons de cinza
    dut.u_img.to_grayscale();
    dut.u_img.print_image("AFTER GRAYSCALE");
    
    // 6) Demonstra empacotamento/desempacotamento 24b -> componentes RGB
    magenta24 = 24'hFF00FF;
    mag_r = magenta24[23:16];  // Canal R
    mag_g = magenta24[15:8];   // Canal G
    mag_b = magenta24[7:0];    // Canal B
    dut.u_img.set_pixel(0, 2, mag_r, mag_g, mag_b);
    dut.u_img.print_image("AFTER UNPACK24 (MAGENTA @ 0,2)");
    
    #10 $finish;
  end
endmodule