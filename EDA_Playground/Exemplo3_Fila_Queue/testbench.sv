module tb;
  
  // Instância do design top
  top dut();
  
  // Variáveis para receber dados do dequeue/peek
  string job_name;
  int job_pages;
  bit ok;
  
  // Geração simples de "tempo" e checkpoints de log
  initial begin
    $dumpfile("waveform.vcd");
    $dumpvars(0, tb);
    
    // Início: fila vazia
    dut.u_spooler.print_status();
    
    // 1) Enfileira três documentos
    dut.u_spooler.enqueue("Relatorio_Projeto.pdf", 12);
    dut.u_spooler.enqueue("Apresentacao.pptx", 25);
    dut.u_spooler.enqueue("Planilha_Custos.xlsx", 7);
    
    // Snapshot do estado
    dut.u_spooler.print_status();
    
    // 2) Espiar quem é o próximo
    dut.u_spooler.peek(job_name, job_pages, ok);
    
    // 3) Imprimir (dequeue) dois documentos
    dut.u_spooler.dequeue(job_name, job_pages, ok);
    dut.u_spooler.dequeue(job_name, job_pages, ok);
    
    // Estado após duas remoções
    dut.u_spooler.print_status();
    
    // 4) Inserir mais um job e checar métricas
    dut.u_spooler.enqueue("Fotos_Evento.zip", 40);
    $display("Size=%0d | Empty=%0b | TotalPages=%0d",
             dut.u_spooler.size(), dut.u_spooler.empty(), dut.u_spooler.total_pages());
    
    // 5) Esvaziar completamente a fila
    while (!dut.u_spooler.empty()) begin
      dut.u_spooler.dequeue(job_name, job_pages, ok);
    end
    dut.u_spooler.print_status();
    
    // 6) Testar remoção/peek com fila vazia
    dut.u_spooler.dequeue(job_name, job_pages, ok);
    dut.u_spooler.peek(job_name, job_pages, ok);
    
    // 7) Teste adicional: encher a fila
    $display("\n--- TESTE DE CAPACIDADE ---");
    for (int i = 1; i <= 5; i++) begin
      $sformatf(job_name, "Job_%0d.pdf", i);
      dut.u_spooler.enqueue(job_name, i * 3);
    end
    dut.u_spooler.print_status();
    
    // Encerrar simulação
    #10 $finish;
  end
  
endmodule