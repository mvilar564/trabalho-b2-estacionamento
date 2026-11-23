# Sistema de Gestão de Estacionamento

## Identificação
- Disciplina: Lógica de Programação e Algoritmos
- Semestre: 2026/2
- Trabalho: Sistema de Gestão de Estacionamento
- Integrante 1: Miguel Vilar Martins Ferreira — GitHub: [mvilar564](https://github.com/mvilar564)
- Integrante 2: [NOME] — GitHub: [PERFIL]

## Cenário e escopo
Sistema de terminal em C para gerenciar um estacionamento fictício de 20 vagas (4 fileiras × 5 colunas). Os registros permanecem apenas durante a execução; não há persistência em arquivos. Os identificadores e descrições usados nos testes são fictícios.

## Funcionalidades
1. Cadastrar veículo
2. Consultar veículo por identificador
3. Atualizar descrição ou categoria
4. Mover veículo para outra vaga
5. Remover veículo
6. Listar veículos cadastrados
7. Exibir mapa de vagas
8. Exibir relatório da ocupação (inclui registros em uso e capacidade reservada da coleção)
0. Encerrar

## Estruturas de dados
- Matriz `int mapa[4][5]`: 0 = livre e 1 = ocupada.
- `struct Veiculo`: identificador de 7 caracteres, descrição de até 60 caracteres, categoria (1 = carro, 2 = moto), fileira e coluna (guardadas de 1 a 4 e de 1 a 5; a conversão para índice da matriz é feita ao acessar o mapa).
- `Colecao`: vetor dinâmico de `Veiculo` (`dados`), com quantidade utilizada (`qtd`) e capacidade reservada (`capacidade`).
- A coleção inicia com capacidade 2 (`malloc`) e dobra com `realloc` quando está cheia. O `realloc` usa um ponteiro temporário, de modo que uma falha não perde os registros existentes. A lotação física permanece limitada a 20 veículos, independentemente da capacidade da coleção.
- Na remoção, os elementos seguintes são deslocados com `memmove` para não deixar lacunas. No encerramento, a memória da coleção é liberada com `free`.
- Consistência: número de registros ativos = número de células com valor 1 na matriz = vagas ocupadas no relatório (verificada por `verificar_consistencia` e exibida no relatório).

## Modularização
- `main.c`: menu e interface das operações (telas), sem variáveis globais; o estado (`mapa` e `Colecao`) é declarado em `main` e passado por parâmetro.
- `estacionamento.h`: constantes, tipos e protótipos.
- `estacionamento.c`: leitura segura (`ler_linha`, `ler_inteiro`, sem `gets`), validações (`validar_id`, `normalizar_id`, `validar_descricao`, `posicao_valida`), mapa (`mapa_iniciar`, `exibir_mapa`), coleção dinâmica (`colecao_iniciar`, `colecao_ampliar`, `colecao_liberar`, `buscar_indice`), operações (`cadastrar_veiculo`, `atualizar_veiculo`, `mover_veiculo`, `remover_veiculo`), relatório (`calcular_relatorio`) e `verificar_consistencia`. As operações não imprimem nada: validam tudo antes de alterar o estado e devolvem um código `Resultado`.

## Compilação e execução
Na pasta que contém os arquivos:

```bash
gcc -std=c11 -Wall -Wextra -pedantic main.c estacionamento.c -o estacionamento
./estacionamento
```

No Windows, executar `estacionamento.exe` após a compilação. Se os arquivos estiverem em uma pasta `src/`, usar `src/main.c src/estacionamento.c` no comando.

## Casos de teste
Os casos foram executados com o programa compilado com as flags acima (sem warnings), também com AddressSanitizer e UndefinedBehaviorSanitizer, sem erros de memória nem vazamentos reportados.

| Caso | Entrada/operação | Resultado esperado | Resultado observado | Situação |
|---|---|---|---|---|
| T01 – Estacionamento vazio | Iniciar e consultar mapa, listagem e relatório | Mapa livre, listagem vazia, 0,00% | Mapa com 20 vagas livres, mensagem "Nenhum veículo cadastrado" e relatório com 0 ocupadas e 0,00% | APROVADO |
| T02 – Cadastro válido | Cadastrar `ABC1D23`, "Carro azul", carro, F1/C1 | Registro aparece na consulta/listagem e vaga fica ocupada | Confirmação "cadastrado na vaga F1/C1"; consulta mostra os dados; mapa exibe X em F1/C1 | APROVADO |
| T03 – Duplicidade sem distinção de caixa | Cadastrar `ABC1D23` e depois `abc1d23` | Segundo cadastro recusado como duplicado | "Recusado: identificador já cadastrado"; permanece 1 registro | APROVADO |
| T04 – Vaga ocupada/índice inválido | Cadastrar em vaga ocupada; fileira 5; coluna 0; fileira -1; coluna não numérica | Operação recusada sem alteração parcial | Todas as tentativas recusadas; relatório continua com 1 vaga ocupada e consistência OK | APROVADO |
| T05 – Crescimento dinâmico | Cadastrar 2, 3 e 5 veículos e consultar o relatório | Capacidade aumenta de 2 para 4 e registros anteriores são preservados | Com 2 registros: 2/2; com 3: 3/4; com 5: 5/8; todos os registros permanecem na listagem | APROVADO |
| T06 – Atualização | Alterar descrição e categoria de `ABC1D23`; tentar descrição de 61 caracteres, categoria 5 e ID inexistente | Dados atualizados sem criar outro registro; ID e vaga preservados; entradas inválidas não alteram nada | Descrição e categoria atualizadas, 1 registro; entradas inválidas recusadas e ID inexistente "não localizado" | APROVADO |
| T07 – Movimentação | Mover para vaga ocupada, para a própria vaga e para vaga livre (F2/C3) | Recusas sem alteração; origem livre, destino ocupado e posição do registro atualizada | Destino ocupado recusado; mesma posição informada sem alteração; F1/C1 liberada e F2/C3 ocupada | APROVADO |
| T08 – Remoção | Remover veículo cadastrado e tentar remover novamente | Registro removido e vaga liberada | Veículo removido, vaga liberada, sem lacuna na listagem; segunda tentativa "não localizado"; consistência OK | APROVADO |
| T09 – Lotação | Cadastrar 20 veículos e tentar o 21º | 21º cadastro recusado | 21º recusado por lotação; relatório com 20 ocupadas e 100,00%; após remover um veículo, novo cadastro aceito | APROVADO |
| T10 – Encerramento | Escolher opção 0 | Memória dinâmica liberada e programa encerrado | Mensagem de encerramento; sem vazamentos nem erros de memória reportados pelo AddressSanitizer | APROVADO |

Testes adicionais de validação: identificador com 6 ou 8 caracteres, com espaço ou símbolo; descrição vazia, só com espaços ou com 61 caracteres (60 é aceita); categoria 0, 3 ou não numérica; linha de entrada muito longa; opção de menu inválida; fim de entrada no meio de uma operação. Todos foram recusados sem alterar o estado.

Verificações extras: (a) falha de `realloc` simulada, substituindo a função por uma que retorna `NULL`: o 3º cadastro foi recusado com mensagem de erro, e os 2 veículos anteriores, o mapa e a consistência permaneceram intactos; (b) 400 execuções com entradas aleatórias (opções, IDs, descrições longas, números inválidos), sem erros de memória e com a consistência matriz × registros sempre OK.

## Limitações conhecidas
- O limite de 60 caracteres da descrição é medido em bytes. Em UTF-8, letras acentuadas ocupam 2 bytes; portanto, uma descrição com muitos acentos pode ser recusada antes de 60 letras. Para evitar problemas de codificação no terminal, as mensagens do programa são exibidas sem acentos.
- Cada campo inválido no cadastro cancela a operação e retorna ao menu (o roteiro permite), em vez de pedir o campo novamente.
- Na atualização, deixar a descrição em branco significa "manter a atual"; no cadastro, descrição vazia é recusada.
- A capacidade da coleção não é reduzida após remoções (não é exigido pelo roteiro).

## Responsabilidades dos integrantes
- Miguel Vilar Martins Ferreira — Participei do desenvolvimento da funcionalidade de cadastro de veículos, acompanhando sua implementação, analisando seu funcionamento e compreendendo as validações necessárias para registrar os veículos nas vagas disponíveis. O desenvolvimento contou com auxílio de ferramentas de Inteligência Artificial, utilizadas como apoio na elaboração e organização do  código, esclarecimento de dúvidas e realização de testes.
- Integrante 2 — [DESCREVER funcionalidades desenvolvidas, de acordo com o histórico do repositório].

## Integração das branches
[DESCREVER branches utilizadas, funcionalidades integradas e como ocorreu a integração à branch principal.]

## Declaração individual de autoria e uso de IA

### Integrante 1 — Miguel Vilar Martins Ferreira
- Funcionalidades em que atuou: [PREENCHER].
- Utilizou IA generativa: SIM.
- Ferramentas e finalidade: Claude (Anthropic), para planejar a estrutura do programa, gerar o código-fonte (`main.c`, `estacionamento.c`, `estacionamento.h`), criar e executar testes automatizados e esclarecer dúvidas sobre o código. [ACRESCENTAR outros usos, se houver.]
- Exemplos de solicitações relevantes e sugestões incorporadas: enviei o roteiro do trabalho e pedi ajuda para desenvolvê-lo e para escrever os códigos; o Claude propôs a divisão em funções, a struct `Colecao` e as validações, e gerou os três arquivos de código, que foram incorporados. [DESCREVER o que revisei, testei, compreendi ou modifiquei por conta própria.]
- Código externo adaptado e fonte: [PREENCHER ou declarar que não houve].

### Integrante 2 — [NOME]
- Funcionalidades em que atuou: [PREENCHER].
- Utilizou IA generativa: [SIM/NÃO].
- Ferramentas e finalidade: [PREENCHER; se não utilizou, declarar expressamente].
- Exemplos de solicitações relevantes e sugestões incorporadas: [PREENCHER].
- Código externo adaptado e fonte: [PREENCHER ou declarar que não houve].
