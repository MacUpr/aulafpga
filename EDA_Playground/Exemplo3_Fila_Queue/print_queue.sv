// ========================================
// print_queue.sv
// ========================================

// Spooler de impressão usando array estático (compatível com Icarus)
module print_spooler;
  
  // Parâmetros da fila
  parameter MAX_JOBS = 100;
  
  // Arrays separados para nome e páginas 
  string job_names[0:MAX_JOBS-1];
  int    job_pages[0:MAX_JOBS-1];
  
  // Controle da fila
  int queue_size;
  int front_ptr;
  int rear_ptr;
  
  // Inicialização
  initial begin
    queue_size = 0;
    front_ptr = 0;
    rear_ptr = 0;
    
    // Inicializar arrays
    for (int i = 0; i < MAX_JOBS; i++) begin
      job_names[i] = "";
      job_pages[i] = 0;
    end
  end
  
  // Insere no fim da fila
  task automatic enqueue(input string name, input int pages);
    if (queue_size >= MAX_JOBS) begin
      $display("[ENQUEUE] ERRO: Fila cheia! Não foi possível adicionar '%s'", name);
    end else begin
      job_names[rear_ptr] = name;
      job_pages[rear_ptr] = pages;
      rear_ptr = (rear_ptr + 1) % MAX_JOBS;
      queue_size = queue_size + 1;
      
      $display("[ENQUEUE] '%s' (%0d pág). Tamanho atual: %0d", name, pages, queue_size);
    end
  endtask
  
  // Remove do início da fila (se houver)
  task automatic dequeue(output string name, output int pages, output bit ok);
    if (queue_size == 0) begin
      ok = 0;
      name = "";
      pages = 0;
      $display("[DEQUEUE] Fila vazia.");
    end else begin
      ok = 1;
      name = job_names[front_ptr];
      pages = job_pages[front_ptr];
      
      // Limpar posição
      job_names[front_ptr] = "";
      job_pages[front_ptr] = 0;
      
      front_ptr = (front_ptr + 1) % MAX_JOBS;
      queue_size = queue_size - 1;
      
      $display("[DEQUEUE] '%s' (%0d pág). Tamanho atual: %0d", name, pages, queue_size);
    end
  endtask
  
  // Olha o próximo da fila sem remover
  task automatic peek(output string name, output int pages, output bit ok);
    if (queue_size == 0) begin
      ok = 0;
      name = "";
      pages = 0;
      $display("[PEEK] Fila vazia.");
    end else begin
      ok = 1;
      name = job_names[front_ptr];
      pages = job_pages[front_ptr];
      $display("[PEEK] Próximo: '%s' (%0d pág).", name, pages);
    end
  endtask
  
  // Quantidade de jobs na fila
  function automatic int size();
    return queue_size;
  endfunction
  
  // Fila está vazia?
  function automatic bit empty();
    return (queue_size == 0);
  endfunction
  
  // Soma total de páginas pendentes
  function automatic int total_pages();
    int sum = 0;
    int idx = front_ptr;
    
    for (int i = 0; i < queue_size; i++) begin
      sum = sum + job_pages[idx];
      idx = (idx + 1) % MAX_JOBS;
    end
    
    return sum;
  endfunction
  
  // Imprime um snapshot do estado da fila
  task automatic print_status();
    int idx;
    
    $display("------ STATUS DA FILA ------");
    $display("Jobs na fila: %0d | Páginas totais: %0d", size(), total_pages());
    
    if (queue_size == 0) begin
      $display("  (vazia)");
    end else begin
      idx = front_ptr;
      for (int i = 0; i < queue_size; i++) begin
        $display("  [%0d] '%s' (%0d pág)", i, job_names[idx], job_pages[idx]);
        idx = (idx + 1) % MAX_JOBS;
      end
    end
    
    $display("----------------------------");
  endtask
  
endmodule