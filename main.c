
#include <stdio.h>
#include <string.h>

#include "estacionamento.h"

/* ---------- Auxiliares de interface ---------- */



static void mostrar_menu(void) {
    printf("\n===== ESTACIONAMENTO =====\n");
    printf("1 - Cadastrar veiculo\n");
    printf("2 - Consultar veiculo por identificador\n");
    printf("3 - Atualizar descricao ou categoria\n");
    printf("4 - Mover veiculo para outra vaga\n");
    printf("5 - Remover veiculo\n");
    printf("6 - Listar veiculos cadastrados\n");
    printf("7 - Exibir mapa de vagas\n");
    printf("8 - Exibir relatorio da ocupacao\n");
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

            case 2:
                tela_consultar(&colecao);
                break;

            case 3:
                tela_atualizar(&colecao);
                break;

            case 4:
                tela_mover(&colecao, mapa);
                break;

            case 5:
                tela_remover(&colecao, mapa);
                break;

            case 6:
                tela_listar(&colecao);
                break;

            case 7:
                exibir_mapa(mapa);
                break;

            case 8:
                tela_relatorio(&colecao, mapa);
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

/* ---------- Consulta e atualizacao ---------- */

static const char *nome_categoria(int cat) {
    return cat == CAT_CARRO ? "carro" : "moto";
}

static void mostrar_veiculo(const Veiculo *v) {
    printf("Identificador: %s\n", v->id);
    printf("Descricao....: %s\n", v->descricao);
    printf("Categoria....: %s\n", nome_categoria(v->categoria));
    printf("Posicao......: F%d/C%d\n", v->fileira, v->coluna);
}

static void tela_consultar(const Colecao *c) {
    char id[128];
    int idx;

    if (!pedir_id(id, sizeof id)) {
        return;
    }

    idx = buscar_indice(c, id);

    if (idx < 0) {
        mostrar_resultado(RES_NAO_ENCONTRADO);
        return;
    }

    mostrar_veiculo(&c->dados[idx]);
}

static void tela_atualizar(Colecao *c) {
    char id[128];
    char desc[128];
    int cat = 0;
    int st;
    Resultado r;

    if (!pedir_id(id, sizeof id)) {
        return;
    }

    if (buscar_indice(c, id) < 0) {
        mostrar_resultado(RES_NAO_ENCONTRADO);
        return;
    }

    printf("Nova descricao (Enter para manter): ");
    fflush(stdout);

    if (ler_linha(desc, sizeof desc) != LINHA_OK) {
        printf("Entrada invalida. Nada foi alterado.\n");
        return;
    }

    st = ler_inteiro(
        "Nova categoria (1 = carro, 2 = moto, 0 = manter): ",
        &cat
    );

    if (st != LINHA_OK || cat < 0 || cat > 2) {
        printf("Categoria invalida. Nada foi alterado.\n");
        return;
    }

    if (desc[0] == '\0' && cat == 0) {
        printf("Nada para alterar.\n");
        return;
    }

    r = atualizar_veiculo(
        c,
        id,
        desc[0] == '\0' ? NULL : desc,
        cat
    );

    if (r == RES_DADO_INVALIDO) {
        printf("Descricao invalida (mais de 60 caracteres ou so espacos). Nada foi alterado.\n");
    } else if (r == RES_OK) {
        printf("Veiculo %s atualizado.\n", id);
    } else {
        mostrar_resultado(r);
    }
}

/* ---------- Movimentacao e remocao ---------- */

static void tela_mover(Colecao *c, int mapa[][COLUNAS]) {
    char id[128];
    int fileira, coluna;
    Resultado r;

    if (!pedir_id(id, sizeof id)) {
        return;
    }

    if (buscar_indice(c, id) < 0) {
        mostrar_resultado(RES_NAO_ENCONTRADO);
        return;
    }

    if (!pedir_posicao(&fileira, &coluna)) {
        return;
    }

    r = mover_veiculo(c, mapa, id, fileira, coluna);

    if (r == RES_OK) {
        printf("Veiculo %s movido para F%d/C%d.\n",
               id, fileira, coluna);
    } else {
        mostrar_resultado(r);
    }
}

static void tela_remover(Colecao *c, int mapa[][COLUNAS]) {
    char id[128];
    Resultado r;

    if (!pedir_id(id, sizeof id)) {
        return;
    }

    r = remover_veiculo(c, mapa, id);

    if (r == RES_OK) {
        printf("Veiculo %s removido; vaga liberada.\n", id);
    } else {
        mostrar_resultado(r);
    }
}

/* ---------- Listagem e relatorio ---------- */

static void tela_listar(const Colecao *c) {
    int i;

    if (c->qtd == 0) {
        printf("Nenhum veiculo cadastrado.\n");
        return;
    }

    printf("%-9s %-30s %-10s %s\n",
           "ID", "Descricao", "Categoria", "Vaga");

    for (i = 0; i < c->qtd; i++) {
        printf("%-9s %-30s %-10s F%d/C%d\n",
               c->dados[i].id,
               c->dados[i].descricao,
               nome_categoria(c->dados[i].categoria),
               c->dados[i].fileira,
               c->dados[i].coluna);
    }
}

static void tela_relatorio(
    const Colecao *c,
    int mapa[][COLUNAS]
) {
    Relatorio r = calcular_relatorio(c, mapa);

    printf("Total de vagas.....: %d\n", r.total);
    printf("Vagas ocupadas.....: %d\n", r.ocupadas);
    printf("Vagas livres.......: %d\n", r.livres);
    printf("Percentual ocupacao: %.2f%%\n", r.percentual);
    printf("Carros.............: %d\n", r.carros);
    printf("Motos..............: %d\n", r.motos);

    printf("Registros em uso / capacidade reservada: %d / %d\n",
           c->qtd, c->capacidade);

    printf("Consistencia matriz x registros: %s\n",
           r.consistente ? "OK" : "FALHOU");
}
