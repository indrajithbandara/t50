#ifndef __CYCLE_COUNTING_INCLUDED__
#define __CYCLE_COUNTING_INCLUDED__

/* ==========================================
    Quick & Dirty cycle counting...

    begin_tsc() e end_tsc()

    são usadas para contar a quantidade de ciclos
    de máquina gastos num bloco de código.

    Exemplo de uso:

      uint64_t t;

      BEGIN_TSC;      
      f();
      END_TSC(t);

    É conveniente compilar o código sob teste com a
    opção -O0, já que o compilador poderá 'sumir' com
    o código, por causa da otimização.

    As macros, em si, não são "otimizáveis", por assim dizer.
   ========================================== */

#include <stdint.h>

uint64_t __local_tsc;

#ifdef __x86_64__
#define REGS1 "rbx","rcx"
#define REGS2 "rcx"
#else
#ifdef __i386__
#define REGS1 "ebx","ecx"
#define REGS2 "ecx"
#else
#error cycle counting will work only on x86-64 or i386 platforms!
#endif
#endif

// Macro: Inicia a medição.
#define BEGIN_TSC do { \
  uint32_t a, d; \
\
  __asm__ __volatile__ ( \
    "xorl %%eax,%%eax\n" \
    "cpuid\n" \
    "rdtsc\n" \
    : "=a" (a), "=d" (d) :: REGS1 \
  ); \
\
  __local_tsc = ((uint64_t)d << 32) | a; \
} while (0)

// Macro: Finaliza a medição.
// NOTA: A rotina anterior usava armazenamento temporário
//       para guardar a contagem vinda de rdtscp. Ainda,
//       CPUID era usado para serialização (que parece ser inóquo!).
//       Obtive resultados melhores retirando a serialização e
//       devolvendo os constraints para EAX e EDX, salvando apenas ECX.
// PS:   rdtscp também serializa, deixando o CPUID supérfluo!
#define END_TSC(c) do { \
  uint32_t a, d; \
\
  __asm__ __volatile__ ( \
    "rdtscp\n" \
    : "=a" (a), "=d" (d) :: REGS2 \
  ); \
\
  (c) = (((uint64_t)d << 32) | a) - __local_tsc; \
} while (0)

#endif  // __CYCLE_COUNTING_INCLUDED__ 
