#ifndef ESTACIONAMENTO_H
#define ESTACIONAMENTO_H

#include <stddef.h>

/* ---------- Constantes do problema ---------- */
#define FILEIRAS 4
#define COLUNAS 5
#define TOTAL_VAGAS (FILEIRAS * COLUNAS)
#define TAM_ID 7
#define TAM_DESC 60
#define CAP_INICIAL 2

#define VAGA_LIVRE 0
#define VAGA_OCUPADA 1

#define CAT_CARRO 1
#define CAT_MOTO 2

/* Retornos das funcoes de leitura */
#define LINHA_OK 0
#define LINHA_INVALIDA 1
#define LINHA_EOF -1

/* ---------- Tipos ---------- */

/* Registro de um veiculo. Fileira e coluna sao guardadas de 1 a 4 / 1 a 5
   (como o usuario ve); a conversao para indice da matriz (-1) acontece
   apenas ao acessar o mapa. */
typedef struct {
    char id[TAM_ID + 1];
    char descricao[TAM_DESC + 1];
    int categoria;
    int fileira;
    int coluna;
} Veiculo;

/* Vetor dinamico de veiculos: dados + quantidade usada + capacidade reservada */
typedef struct {
    Veiculo *dados;
    int qtd;
    int capacidade;
} Colecao;

typedef struct {
    int total;
    int ocupadas;
    int livres;
    double percentual;
    int carros;
    int motos;
    int consistente; /* 1 se matriz e registros concordam */
} Relatorio;

typedef enum {
    RES_OK = 0,
    RES_LOTADO,
    RES_DUPLICADO,
    RES_NAO_ENCONTRADO,
    RES_POSICAO_INVALIDA,
    RES_OCUPADA,
    RES_MESMA_POSICAO,
    RES_DADO_INVALIDO,
    RES_SEM_MEMORIA
} Resultado;

/* ---------- Entrada segura ---------- */
int ler_linha(char *buf, size_t tam);
int ler_inteiro(const char *msg, int *valor);

/* ---------- Validacao ---------- */
int validar_id(const char *id);
int normalizar_id(char *id);
int validar_descricao(const char *desc);
int validar_categoria(int categoria);
int posicao_valida(int fileira, int coluna);

/* ---------- Mapa de vagas (matriz 4 x 5) ---------- */
void mapa_iniciar(int mapa[][COLUNAS]);
void exibir_mapa(int mapa[][COLUNAS]);

/* ---------- Colecao dinamica ---------- */
int colecao_iniciar(Colecao *c);
int colecao_ampliar(Colecao *c);
void colecao_liberar(Colecao *c);
int buscar_indice(const Colecao *c, const char *id);

/* ---------- Operacoes (nao imprimem nada; devolvem um Resultado) ---------- */
Resultado cadastrar_veiculo(Colecao *c, int mapa[][COLUNAS], const Veiculo *novo);
Resultado atualizar_veiculo(Colecao *c, const char *id, const char *nova_desc, int nova_cat);
Resultado mover_veiculo(Colecao *c, int mapa[][COLUNAS], const char *id, int fileira, int coluna);
Resultado remover_veiculo(Colecao *c, int mapa[][COLUNAS], const char *id);

/* ---------- Relatorio e consistencia ---------- */
int verificar_consistencia(const Colecao *c, int mapa[][COLUNAS]);
Relatorio calcular_relatorio(const Colecao *c, int mapa[][COLUNAS]);

#endif
