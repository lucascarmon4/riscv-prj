# Simulador RISC-V

Este simulador implementa, de forma didática, um processador **RV32I** simples.  
Ele executa instruções carregadas na memória, manipula RAM e VRAM através do barramento, trata interrupções externas e permite ao usuário acompanhar tudo passo a passo por meio de um menu interativo.

Em termos práticos, ele funciona como um **pequeno computador RISC-V completo**, mas totalmente visível e controlado pelo usuário.

---

## Como Executar

Se você já possui o executável `main.exe`, basta rodar:

```
main.exe
```

O simulador abrirá um **menu interativo** no terminal para controle manual da CPU.

---

## Como Funciona (Menu e Opções)

Ao iniciar, o simulador apresenta o menu:

```
================ MENU ================
1) Executar 1 instrucao
2) Executar N instrucoes (tecla 'k' dispara IRQ externa)
3) Mostrar registradores (dump CPU)
4) Dump de RAM (primeiros 64 bytes)
5) Dump de VRAM ASCII (matriz 32 colunas)
6) Editar um byte da RAM
7) Editar um byte da VRAM
0) Sair
```

### **1) Executar 1 instrução**

Executa um único ciclo da CPU (busca, decodificação e execução).

### **2) Executar N instruções**

Executa várias instruções em sequência.  
Durante a execução, pressionar **`k`** dispara uma **interrupção externa**, desviando o PC para o handler.

### **3) Mostrar registradores**

Exibe:

-   valores de x0 até x31
-   valor atual do PC

### **4) Dump de RAM**

Mostra os primeiros 64 bytes da RAM.

### **5) Dump de VRAM (ASCII)**

Mostra a VRAM como matriz de caracteres ASCII (32 colunas).

### **6) Editar RAM**

Permite modificar manualmente um byte da RAM.

### **7) Editar VRAM**

Permite modificar manualmente um byte da VRAM.

---

## Como Compilar

Na raiz do projeto:

```
g++ -std=c++17 -Iinclude src/*.cpp -o main.exe
```

Com otimização:

```
g++ -O2 -std=c++17 -Iinclude src/*.cpp -o main.exe
```

---

## Requisitos

-   Compilador C++17 (MinGW recomendado)
-   Windows (para suporte à função `_kbhit()` usada no menu)

---

O projeto já vem com um programa de teste em RISC-V, VRAM funcional, IO mapeado em memória, interrupção externa via teclado e um menu para interação direta com a execução da CPU.
