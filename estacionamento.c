
#include "estacionamento.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ================= ENTRADA SEGURA ================= */

int ler_linha(char *buf, size_t tam) {
    size_t n;
    int ch;
    int longa = 0;

    if (fgets(buf, (int)tam, stdin) == NULL) {
        return LINHA_EOF;
    }

    n = strlen(buf);

    if (n > 0 && buf[n - 1] == '\n') {
        buf[n - 1] = '\0';
        n--;

        if (n > 0 && buf[n - 1] == '\r') {
            buf[n - 1] = '\0';
        }

        return LINHA_OK;
    }

    while ((ch = getchar()) != '\n' && ch != EOF) {
        longa = 1;
    }

    return longa ? LINHA_INVALIDA : LINHA_OK;
}

int ler_inteiro(const char *msg, int *valor) {
    char buf[32];
    char *fim;
    long v;
    int st;

    printf("%s", msg);
    fflush(stdout);

    st = ler_linha(buf, sizeof buf);

    if (st != LINHA_OK) {
        return st;
    }

    errno = 0;
    v = strtol(buf, &fim, 10);

    if (fim == buf || errno != 0) {
        return LINHA_INVALIDA;
    }

    while (*fim == ' ' || *fim == '\t') {
        fim++;
    }

    if (*fim != '\0' || v < INT_MIN || v > INT_MAX) {
        return LINHA_INVALIDA;
    }

    *valor = (int)v;
    return LINHA_OK;
}

/* ================= VALIDACOES ================= */

int validar_id(const char *id) {
    size_t i;

    if (strlen(id) != TAM_ID) {
        return 0;
    }

    for (i = 0; i < TAM_ID; i++) {
        char ch = id[i];

        if (!((ch >= 'A' && ch <= 'Z') ||
              (ch >= '0' && ch <= '9'))) {
            return 0;
        }
    }

    return 1;
}

int normalizar_id(char *id) {
    size_t i;

    for (i = 0; id[i] != '\0'; i++) {
        if (id[i] >= 'a' && id[i] <= 'z') {
            id[i] = (char)(id[i] - 'a' + 'A');
        }
    }

    return validar_id(id);
}

int validar_descricao(const char *desc) {
    size_t i;
    size_t n = strlen(desc);

    if (n == 0 || n > TAM_DESC) {
        return 0;
    }

    for (i = 0; i < n; i++) {
        if (desc[i] != ' ' && desc[i] != '\t') {
            return 1;
        }
    }

    return 0;
}

int validar_categoria(int categoria) {
    return categoria == CAT_CARRO ||
           categoria == CAT_MOTO;
}

int posicao_valida(int fileira, int coluna) {
    return fileira >= 1 &&
           fileira <= FILEIRAS &&
           coluna >= 1 &&
           coluna <= COLUNAS;
}

/* ================= MAPA DE VAGAS ================= */

void mapa_iniciar(int mapa[][COLUNAS]) {
    int i, j;

    for (i = 0; i < FILEIRAS; i++) {
        for (j = 0; j < COLUNAS; j++) {
            mapa[i][j] = VAGA_LIVRE;
        }
    }
}

void exibir_mapa(int mapa[][COLUNAS]) {
    int i, j;

    printf("   ");

    for (j = 0; j < COLUNAS; j++) {
        printf(" C%d", j + 1);
    }

    printf("\n");

    for (i = 0; i < FILEIRAS; i++) {
        printf("F%d ", i + 1);

        for (j = 0; j < COLUNAS; j++) {
            printf("  %c",
                   mapa[i][j] == VAGA_OCUPADA ? 'X' : '.');
        }

        printf("\n");
    }

    printf("Legenda: X = ocupada; . = livre\n");
}

/* ================= COLECAO DINAMICA ================= */

int colecao_iniciar(Colecao *c) {
    c->qtd = 0;

    c->dados = malloc(
        CAP_INICIAL * sizeof *c->dados
    );

    if (c->dados == NULL) {
        c->capacidade = 0;
        return 0;
    }

    c->capacidade = CAP_INICIAL;
    return 1;
}

int colecao_ampliar(Colecao *c) {
    int nova_cap = c->capacidade > 0
        ? c->capacidade * 2
        : CAP_INICIAL;

    Veiculo *tmp = realloc(
        c->dados,
        (size_t)nova_cap * sizeof *tmp
    );

    if (tmp == NULL) {
        return 0;
    }

    c->dados = tmp;
    c->capacidade = nova_cap;

    return 1;
}

void colecao_liberar(Colecao *c) {
    free(c->dados);

    c->dados = NULL;
    c->qtd = 0;
    c->capacidade = 0;
}

int buscar_indice(const Colecao *c, const char *id) {
    int i;

    for (i = 0; i < c->qtd; i++) {
        if (strcmp(c->dados[i].id, id) == 0) {
            return i;
        }
    }

    return -1;
}

/* ================= CADASTRO ================= */

Resultado cadastrar_veiculo(
    Colecao *c,
    int mapa[][COLUNAS],
    const Veiculo *novo
) {
    if (!validar_id(novo->id) ||
        !validar_descricao(novo->descricao) ||
        !validar_categoria(novo->categoria)) {
        return RES_DADO_INVALIDO;
    }

    if (c->qtd >= TOTAL_VAGAS) {
        return RES_LOTADO;
    }

    if (buscar_indice(c, novo->id) >= 0) {
        return RES_DUPLICADO;
    }

    if (!posicao_valida(novo->fileira, novo->coluna)) {
        return RES_POSICAO_INVALIDA;
    }

    if (mapa[novo->fileira - 1][novo->coluna - 1]
        != VAGA_LIVRE) {
        return RES_OCUPADA;
    }

    if (c->qtd == c->capacidade &&
        !colecao_ampliar(c)) {
        return RES_SEM_MEMORIA;
    }

    c->dados[c->qtd] = *novo;
    c->qtd++;

    mapa[novo->fileira - 1][novo->coluna - 1] =
        VAGA_OCUPADA;

    return RES_OK;
}

/* ================= ATUALIZACAO ================= */

Resultado atualizar_veiculo(
    Colecao *c,
    const char *id,
    const char *nova_desc,
    int nova_cat
) {
    int idx = buscar_indice(c, id);
    Veiculo *v;

    if (idx < 0) {
        return RES_NAO_ENCONTRADO;
    }

    if ((nova_desc != NULL &&
         !validar_descricao(nova_desc)) ||
        (nova_cat != 0 &&
         !validar_categoria(nova_cat))) {
        return RES_DADO_INVALIDO;
    }

    v = &c->dados[idx];

    if (nova_desc != NULL) {
        snprintf(
            v->descricao,
            sizeof v->descricao,
            "%s",
            nova_desc
        );
    }

    if (nova_cat != 0) {
        v->categoria = nova_cat;
    }

    return RES_OK;
}
