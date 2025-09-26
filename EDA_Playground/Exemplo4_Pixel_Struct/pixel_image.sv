// ========================================
// pixel_image.sv
// ========================================

// Módulo de imagem usando arrays separados para RGB (compatível com Icarus)
module pixel_image #(parameter int W = 4, H = 3);
  
  // Arrays separados para canais RGB (substituindo struct packed)
  logic [7:0] img_r [0:H-1][0:W-1];  // Canal Red
  logic [7:0] img_g [0:H-1][0:W-1];  // Canal Green  
  logic [7:0] img_b [0:H-1][0:W-1];  // Canal Blue
  
  // Constrói um pixel a partir de canais (retorna valor empacotado)
  function automatic logic [23:0] mk_pixel(input logic [7:0] r, g, b);
    return {r, g, b};
  endfunction
  
  // Converte para 24 bits (0xRRGGBB)
  function automatic logic [23:0] pack24(input logic [7:0] r, g, b);
    return {r, g, b};
  endfunction
  
  // Constrói pixel a partir de 24 bits (0xRRGGBB) - retorna apenas R
  function automatic logic [7:0] unpack24_r(input logic [23:0] x);
    return x[23:16];
  endfunction
  
  // Extrai canal G de 24 bits
  function automatic logic [7:0] unpack24_g(input logic [23:0] x);
    return x[15:8];
  endfunction
  
  // Extrai canal B de 24 bits
  function automatic logic [7:0] unpack24_b(input logic [23:0] x);
    return x[7:0];
  endfunction
  
  // Preenche a imagem inteira com uma cor
  task automatic fill(input logic [7:0] r, g, b);
    for (int y = 0; y < H; y++) begin
      for (int x = 0; x < W; x++) begin
        img_r[y][x] = r;
        img_g[y][x] = g;
        img_b[y][x] = b;
      end
    end
  endtask
  
  // Define um pixel (com checagem simples de limites)
  task automatic set_pixel(input int x, y, input logic [7:0] r, g, b);
    if (x >= 0 && x < W && y >= 0 && y < H) begin
      img_r[y][x] = r;
      img_g[y][x] = g;
      img_b[y][x] = b;
    end else begin
      $display("[WARN] set_pixel fora da área: x=%0d y=%0d", x, y);
    end
  endtask
  
  // Lê um pixel (retorna 'ok' = 0 se fora da área)
  task automatic get_pixel(input int x, y, output logic [7:0] r, g, b, output bit ok);
    if (x >= 0 && x < W && y >= 0 && y < H) begin
      r = img_r[y][x];
      g = img_g[y][x];
      b = img_b[y][x];
      ok = 1;
    end else begin
      r = 8'h00;
      g = 8'h00;
      b = 8'h00;
      ok = 0;
    end
  endtask
  
  // Inverte cores de toda a imagem (negativo)
  task automatic invert();
    for (int y = 0; y < H; y++) begin
      for (int x = 0; x < W; x++) begin
        img_r[y][x] = ~img_r[y][x];
        img_g[y][x] = ~img_g[y][x];
        img_b[y][x] = ~img_b[y][x];
      end
    end
  endtask
  
  // Converte para tom de cinza por média simples (r+g+b)/3
  task automatic to_grayscale();
    int gray;
    for (int y = 0; y < H; y++) begin
      for (int x = 0; x < W; x++) begin
        gray = (img_r[y][x] + img_g[y][x] + img_b[y][x]) / 3;
        img_r[y][x] = gray[7:0];
        img_g[y][x] = gray[7:0];
        img_b[y][x] = gray[7:0];
      end
    end
  endtask
  
  // Imprime a imagem no console em formato 0xRRGGBB
  task automatic print_image(string title);
    if (title == "") title = "IMAGE";
    $display("---- %s (%0dx%0d) ----", title, W, H);
    for (int y = 0; y < H; y++) begin
      string line = "";
      for (int x = 0; x < W; x++) begin
        line = {line, $sformatf(" 0x%06h", pack24(img_r[y][x], img_g[y][x], img_b[y][x]))};
      end
      $display("%s", line);
    end
    $display("------------------------");
  endtask
  
endmodule

