#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define TAMANHO_MAX_LEXEMA 16

typedef enum {
    TOKEN_PROGRAM, TOKEN_IDENTIFICADOR, TOKEN_INT, TOKEN_BOOLEAN,
    TOKEN_INICIO, TOKEN_FIM, TOKEN_LEIA, TOKEN_ESCREVA, TOKEN_IF,
    TOKEN_ELIF, TOKEN_FOR, TOKEN_SET, TOKEN_ATE, TOKEN_DE,
    TOKEN_VERDADEIRO, TOKEN_FALSO, TOKEN_E, TOKEN_OU, TOKEN_NAO,
    TOKEN_NUMERO, TOKEN_SIMBOLO, TOKEN_ERRO, TOKEN_EOF,
    TOKEN_PONTO_VIRGULA, TOKEN_ABRE_PARENTESES, TOKEN_FECHA_PARENTESES
} TipoToken;

typedef struct {
    TipoToken tipo;
    char lexema[TAMANHO_MAX_LEXEMA];
    int linha;
} Token;
//Variáveis globais
FILE *arquivo_fonte;
int linha_atual = 1;
char caractere_atual;
Token token_atual;

// Avança para o próximo caractere no arquivo fonte e atualiza a contagem de linhas
void avancar_caractere() {
    caractere_atual = fgetc(arquivo_fonte);
    if (caractere_atual == '\n') {
        linha_atual++;
    }
}

// Ignora espaços em branco e comentários (# para linha única, {-} para múltiplas linhas)
void ignorar_espacos_e_comentarios() {
    while (isspace(caractere_atual) || caractere_atual == '#' || caractere_atual == '{') {
        if (caractere_atual == '#') { 
            while (caractere_atual != '\n' && caractere_atual != EOF) {
                avancar_caractere();
            }
        } else if (caractere_atual == '{') { 
            avancar_caractere();
            while (!(caractere_atual == '-' && fgetc(arquivo_fonte) == '}') && caractere_atual != EOF) {
                if (caractere_atual == '\n') linha_atual++;
                avancar_caractere();
            }
            avancar_caractere(); 
        }
        avancar_caractere();
    }
}
// Verifica se um lexema é uma palavra reservada da linguagem
TipoToken verificar_palavra_reservada(const char *lexema) {
    if (strcmp(lexema, "program") == 0) return TOKEN_PROGRAM;
    if (strcmp(lexema, "integer") == 0) return TOKEN_INT;
    if (strcmp(lexema, "boolean") == 0) return TOKEN_BOOLEAN;
    if (strcmp(lexema, "begin") == 0) return TOKEN_INICIO;
    if (strcmp(lexema, "end") == 0) return TOKEN_FIM;
    if (strcmp(lexema, "read") == 0) return TOKEN_LEIA;
    if (strcmp(lexema, "write") == 0) return TOKEN_ESCREVA;
    if (strcmp(lexema, "if") == 0) return TOKEN_IF;
    if (strcmp(lexema, "elif") == 0) return TOKEN_ELIF;
    if (strcmp(lexema, "for") == 0) return TOKEN_FOR;
    if (strcmp(lexema, "set") == 0) return TOKEN_SET;
    if (strcmp(lexema, "to") == 0) return TOKEN_ATE;
    if (strcmp(lexema, "of") == 0) return TOKEN_DE;
    if (strcmp(lexema, "true") == 0) return TOKEN_VERDADEIRO;
    if (strcmp(lexema, "false") == 0) return TOKEN_FALSO;
    if (strcmp(lexema, "and") == 0) return TOKEN_E;
    if (strcmp(lexema, "or") == 0) return TOKEN_OU;
    if (strcmp(lexema, "not") == 0) return TOKEN_NAO;
    return TOKEN_IDENTIFICADOR;
}
TipoToken identificar_simbolo(char c) {
    switch (c) {
        case ';': return TOKEN_PONTO_VIRGULA;
        case '(': return TOKEN_ABRE_PARENTESES;
        case ')': return TOKEN_FECHA_PARENTESES;
        case ':':
        case '>':
        case '.':
        case ',': return TOKEN_SIMBOLO;
        default: return TOKEN_SIMBOLO;
    }
}
// Obtém o próximo token do arquivo fonte
Token obter_proximo_token() {
    Token token;
    token.linha = linha_atual;
    ignorar_espacos_e_comentarios();

    if (caractere_atual == EOF) {
        token.tipo = TOKEN_EOF;
        strcpy(token.lexema, "");
        return token;
    }

    if (isalpha(caractere_atual)) {
        int i = 0;
        while (isalnum(caractere_atual) || caractere_atual == '_') {
            if (i < TAMANHO_MAX_LEXEMA - 1) {
                token.lexema[i++] = caractere_atual;
            }
            avancar_caractere();
        }
        token.lexema[i] = '\0';
        token.tipo = verificar_palavra_reservada(token.lexema);
        return token;
    }

    if (isdigit(caractere_atual)) {
        int i = 0;
        while (isdigit(caractere_atual)) {
            if (i < TAMANHO_MAX_LEXEMA - 1) {
                token.lexema[i++] = caractere_atual;
            }
            avancar_caractere();
        }
        token.lexema[i] = '\0';
        token.tipo = TOKEN_NUMERO;
        return token;
    }

    token.tipo = identificar_simbolo(caractere_atual);
    token.lexema[0] = caractere_atual;
    token.lexema[1] = '\0';
    avancar_caractere();
    return token;
}
//Prints
void imprimir_token(Token token) {
    const char *nomes_tokens[] = {
        "program", "identificador", "integer", "boolean", "begin", "end", 
        "read", "write", "if", "elif", "for", "set", "to", "of", 
        "true", "false", "and", "or", "not", "numero", "simbolo", 
        "erro", "EOF", "ponto_virgula", "abre_par", "fecha_par"
    };

    if (token.tipo == TOKEN_IDENTIFICADOR || token.tipo == TOKEN_NUMERO) {
        printf("%d:%s | %s\n", token.linha, nomes_tokens[token.tipo], token.lexema);
    } else {
        printf("%d:%s\n", token.linha, nomes_tokens[token.tipo]);
    }
}

void erro_sintatico(const char *esperado) {
    fprintf(stderr, "%d:erro sintatico, esperado [%s] encontrado [%s]\n", 
            linha_atual, esperado, token_atual.lexema);
    fclose(arquivo_fonte);
    exit(1);
}
//Main
int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <arquivo-fonte>\n", argv[0]);
        return 1;
    }

    arquivo_fonte = fopen(argv[1], "r");
    if (!arquivo_fonte) {
        perror("Erro ao abrir o arquivo");
        return 1;
    }

    avancar_caractere();

    do {
        token_atual = obter_proximo_token();
        imprimir_token(token_atual);

        if (token_atual.tipo == TOKEN_ESCREVA) {
            Token t = obter_proximo_token();
            if (t.tipo != TOKEN_ABRE_PARENTESES) {
                erro_sintatico("(");
            }
            imprimir_token(t);

            t = obter_proximo_token();
            if (t.tipo != TOKEN_IDENTIFICADOR && t.tipo != TOKEN_NUMERO) {
                erro_sintatico("identificador ou número");
            }
            imprimir_token(t);

            t = obter_proximo_token();
            if (t.tipo != TOKEN_FECHA_PARENTESES) {
                token_atual = t;
                erro_sintatico(")");
            }
            imprimir_token(t);
        }

    } while (token_atual.tipo != TOKEN_EOF);

    printf("%d linhas analisadas, programa sintaticamente correto\n", linha_atual);

    fclose(arquivo_fonte);
    return 0;
}