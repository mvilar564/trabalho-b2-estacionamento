
#include <stdio.h>
#include <string.h>

#include "estacionamento.h"

/* ---------- Auxiliares de interface ---------- */

static void mostrar_menu(void) {
    printf("\n===== ESTACIONAMENTO =====\n");
    printf("1 - Cadastrar veiculo\n");
    printf("0 - Encerrar\n");
}

static void mostrar_resultado(Resultado r) {
    switch (r) {
        case RES_OK:
            printf("Operacao realizada com sucesso.\n");
            break;
        case RES_LOTADO:
            printf("Recusado: estacionamento lotado (20 vagas ocupadas).\n");
            break;
        case RES_DUPLICADO:
            printf("Recusado: identificador ja cadastrado.\n");
            break;
        case RES_NAO_ENCONTRADO:
            printf("Veiculo nao localizado.\n");
            break;
        case RES_POSICAO_INVALIDA:
            printf("Recusado: posicao fora do estacionamento (fileira 1-4, coluna 1-5).\n");
            break;
        case RES_OCUPADA:
            printf("Recusado: a vaga de destino esta ocupada.\n");
            break;
        case RES_MESMA_POSICAO:
            printf("O veiculo ja esta nessa vaga; nada foi alterado.\n");
            break;
        case RES_DADO_INVALIDO:
            printf("Recusado: dados invalidos.\n");
            break;
        case RES_SEM_MEMORIA:
            printf("Erro: nao foi possivel ampliar a memoria; nada foi alterado.\n");
            break;
    }
}

/* Le e valida o identificador */
static int pedir_id(char *id, size_t tam) {
    printf("Identificador (7 caracteres, A-Z e 0-9): ");
    fflush(stdout);

    if (ler_linha(id, tam) != LINHA_OK || !normalizar_id(id)) {
        printf("Identificador invalido (precisa ter 7 caracteres, somente A-Z e 0-9).\n");
        return 0;
    }

    return 1;
}

/* Le fileira e coluna */
static int pedir_posicao(int *fileira, int *coluna) {
    if (ler_inteiro("Fileira (1-4): ", fileira) != LINHA_OK ||
        ler_inteiro("Coluna (1-5): ", coluna) != LINHA_OK) {

        printf("Entrada numerica invalida. Operacao cancelada.\n");
        return 0;
    }

    return 1;
}

/* ---------- Cadastro de veiculos ---------- */

static void tela_cadastrar(Colecao *c, int mapa[][COLUNAS]) {
    Veiculo v;
    char buf[128];
    Resultado r;

    if (c->qtd >= TOTAL_VAGAS) {
        mostrar_resultado(RES_LOTADO);
        return;
    }

    memset(&v, 0, sizeof v);

    if (!pedir_id(buf, sizeof buf)) {
        return;
    }

    if (buscar_indice(c, buf) >= 0) {
        mostrar_resultado(RES_DUPLICADO);
        return;
    }

    memcpy(v.id, buf, TAM_ID + 1);

    printf("Descricao (1 a 60 caracteres): ");
    fflush(stdout);

    if (ler_linha(buf, sizeof buf) != LINHA_OK ||
        !validar_descricao(buf)) {

        printf("Descricao invalida (vazia ou maior que 60 caracteres). Cadastro cancelado.\n");
        return;
    }

    memcpy(v.descricao, buf, strlen(buf) + 1);

    if (ler_inteiro("Categoria (1 = carro, 2 = moto): ", &v.categoria) != LINHA_OK ||
        !validar_categoria(v.categoria)) {

        printf("Categoria invalida. Cadastro cancelado.\n");
        return;
    }

    if (!pedir_posicao(&v.fileira, &v.coluna)) {
        return;
    }

    r = cadastrar_veiculo(c, mapa, &v);

    if (r == RES_OK) {
        printf("Veiculo %s cadastrado na vaga F%d/C%d.\n",
               v.id, v.fileira, v.coluna);
    } else {
        mostrar_resultado(r);
    }
}

/* ---------- Programa principal ---------- */

int main(void) {
    int mapa[FILEIRAS][COLUNAS];
    Colecao colecao;
    int opcao = -1;
    int st;

    mapa_iniciar(mapa);

    if (!colecao_iniciar(&colecao)) {
        fprintf(stderr, "Erro: falha ao alocar memoria inicial.\n");
        return 1;
    }

    do {
        mostrar_menu();

        st = ler_inteiro("Opcao: ", &opcao);

        if (st == LINHA_EOF) {
            opcao = 0;
            continue;
        }

        if (st != LINHA_OK) {
            opcao = -1;
        }

        switch (opcao) {
            case 1:
                tela_cadastrar(&colecao, mapa);
                break;

            case 0:
                break;

            default:
                printf("Opcao inexistente. Tente novamente.\n");
                break;
        }

    } while (opcao != 0);

    colecao_liberar(&colecao);
    printf("Memoria liberada. Ate logo!\n");

    return 0;
}
