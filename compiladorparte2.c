#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define TAMANHO_MAX_LEXEMA 16
#define TAMANHO_TABELA_SIMBOLOS 100

//Tipos de token possiveis pro compilador
typedef enum {
    TOKEN_PROGRAM, TOKEN_ID, TOKEN_INTEIRO, TOKEN_BOOLEANO,
    TOKEN_INICIO, TOKEN_FIM, TOKEN_LEIA, TOKEN_ESCREVA, TOKEN_SE,
    TOKEN_SENAOSE, TOKEN_FOR, TOKEN_SET, TOKEN_ATE, TOKEN_DE,
    TOKEN_NUMERO, TOKEN_SIMBOLO, TOKEN_ERRO, TOKEN_EOF,
    TOKEN_PONTO_VIRGULA, TOKEN_ABRE_PARENTESES, TOKEN_FECHA_PARENTESES,
    TOKEN_MULTIPLICACAO, TOKEN_DOIS_PONTOS
} TipoToken;

typedef struct {
    TipoToken tipo;
    char lexema[TAMANHO_MAX_LEXEMA];
    int linha;
    union {
        int valor_inteiro;
        char str_valor[TAMANHO_MAX_LEXEMA];
    } valor;
} Token;

typedef struct {
    char identificador[TAMANHO_MAX_LEXEMA];
    int endereco;
} SimboloTabela;

// Variáveis globais
FILE *arquivo_fonte;
int linha_atual = 1;
char caractere_atual;
Token token_atual;
int proximo_endereco = 0;
static int rotulo_atual = 1;
SimboloTabela tabela_simbolos[TAMANHO_TABELA_SIMBOLOS];
int num_simbolos = 0;

// Protótipos
void avancar_caractere();
void ignorar_espacos_e_comentarios();
Token obter_proximo_token();
void erro_sintatico(const char *esperado);
TipoToken verificar_palavra_reservada(const char *lexema);
void funcao_for();
void funcao_set();
void funcao_read();
void funcao_write();
void declaracao_variaveis();
int busca_tabela_simbolos(const char *id);
void inserir_simbolo(const char *id);
int proximo_rotulo(void);
int converter_binario(const char* str);
void funcao_composto();

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
            while (caractere_atual != EOF) {
                if (caractere_atual == '-' && fgetc(arquivo_fonte) == '}') {
                    avancar_caractere();
                    break;
                }
                if (caractere_atual == '\n') linha_atual++;
                avancar_caractere();
            }
        }
        avancar_caractere();
    }
}
//Converte em decimal os valores em binario
int converter_binario(const char* str) {
    str += 2; 
    int resultado = 0;
    while (*str) {
        resultado = (resultado << 1) + (*str - '0');
        str++;
    }
    return resultado;
}
// Verifica se um lexema é uma palavra reservada da linguagem
TipoToken verificar_palavra_reservada(const char *lexema) {
    if (strcmp(lexema, "program") == 0) return TOKEN_PROGRAM;
    if (strcmp(lexema, "integer") == 0) return TOKEN_INTEIRO;
    if (strcmp(lexema, "begin") == 0) return TOKEN_INICIO;
    if (strcmp(lexema, "end") == 0) return TOKEN_FIM;
    if (strcmp(lexema, "read") == 0) return TOKEN_LEIA;
    if (strcmp(lexema, "write") == 0) return TOKEN_ESCREVA;
    if (strcmp(lexema, "for") == 0) return TOKEN_FOR;
    if (strcmp(lexema, "set") == 0) return TOKEN_SET;
    if (strcmp(lexema, "to") == 0) return TOKEN_ATE;
    if (strcmp(lexema, "of") == 0) return TOKEN_DE;
    return TOKEN_ID;
}
// Obtém o próximo token do arquivo fonte
Token obter_proximo_token() {
    Token token;
    token.linha = linha_atual;
    ignorar_espacos_e_comentarios();

    if (caractere_atual == EOF) {
        token.tipo = TOKEN_EOF;
        strcpy(token.lexema, "EOF");
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

    if (caractere_atual == '0' && (fgetc(arquivo_fonte) == 'b' || fgetc(arquivo_fonte) == 'B')) {
        avancar_caractere();
        avancar_caractere();
        int i = 0;
        while (caractere_atual == '0' || caractere_atual == '1') {
            if (i < TAMANHO_MAX_LEXEMA - 1) {
                token.lexema[i++] = caractere_atual;
            }
            avancar_caractere();
        }
        token.lexema[i] = '\0';
        token.tipo = TOKEN_NUMERO;
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

    token.lexema[0] = caractere_atual;
    token.lexema[1] = '\0';

    switch (caractere_atual) {
        case ';': token.tipo = TOKEN_PONTO_VIRGULA; break;
        case '(': token.tipo = TOKEN_ABRE_PARENTESES; break;
        case ')': token.tipo = TOKEN_FECHA_PARENTESES; break;
        case '*': token.tipo = TOKEN_MULTIPLICACAO; break;
        case ':': token.tipo = TOKEN_DOIS_PONTOS; break;
        default: token.tipo = TOKEN_SIMBOLO;
    }

    avancar_caractere();
    return token;
}
//Printa se encontrar um erro e quebra o codigo
void erro_sintatico(const char *esperado) {
    fprintf(stderr, "%d:erro sintatico, esperado [%s] encontrado [%s]\n", 
            linha_atual, esperado, token_atual.lexema);
    fclose(arquivo_fonte);
    exit(1);
}
// Busca um identificador na tabela de símbolos e retorna seu endereço
int busca_tabela_simbolos(const char *id) {
    for (int i = 0; i < num_simbolos; i++) {
        if (strcmp(tabela_simbolos[i].identificador, id) == 0) {
            return tabela_simbolos[i].endereco;
        }
    }
    fprintf(stderr, "%d:erro semantico, identificador [%s] nao declarado\n", 
            linha_atual, id);
    exit(1);
}
//Salva na tabela de simbolos
void inserir_simbolo(const char *id) {
    for (int i = 0; i < num_simbolos; i++) {
        if (strcmp(tabela_simbolos[i].identificador, id) == 0) {
            fprintf(stderr, "%d:erro semantico, identificador [%s] ja declarado\n", 
                    linha_atual, id);
            exit(1);
        }
    }
    
    if (num_simbolos >= TAMANHO_TABELA_SIMBOLOS) {
        fprintf(stderr, "Erro: Tabela de símbolos cheia\n");
        exit(1);
    }
    
    strcpy(tabela_simbolos[num_simbolos].identificador, id);
    tabela_simbolos[num_simbolos].endereco = proximo_endereco++;
    num_simbolos++;
}

int proximo_rotulo(void) {
    return rotulo_atual++;
}

void declaracao_variaveis() {
    while (token_atual.tipo == TOKEN_INTEIRO) {
        token_atual = obter_proximo_token();
        if (token_atual.tipo != TOKEN_ID) {
            erro_sintatico("identificador");
        }
        inserir_simbolo(token_atual.lexema);
        token_atual = obter_proximo_token();
        
        while (token_atual.tipo == TOKEN_SIMBOLO && token_atual.lexema[0] == ',') {
            token_atual = obter_proximo_token();
            if (token_atual.tipo != TOKEN_ID) {
                erro_sintatico("identificador");
            }
            inserir_simbolo(token_atual.lexema);
            token_atual = obter_proximo_token();
        }
        
        if (token_atual.tipo != TOKEN_PONTO_VIRGULA) {
            erro_sintatico(";");
        }
        token_atual = obter_proximo_token();
    }
    
    printf("INPP\n");
    printf("AMEM %d\n", num_simbolos);
}
//Função de leitura
void funcao_read() {
    token_atual = obter_proximo_token();
    if (token_atual.tipo != TOKEN_ABRE_PARENTESES) {
        erro_sintatico("(");
    }
    
    token_atual = obter_proximo_token();
    if (token_atual.tipo != TOKEN_ID) {
        erro_sintatico("identificador");
    }
    
    int endereco = busca_tabela_simbolos(token_atual.lexema);
    printf("LEIT\n");
    printf("ARMZ %d\n", endereco);
    
    token_atual = obter_proximo_token();
    if (token_atual.tipo != TOKEN_FECHA_PARENTESES) {
        erro_sintatico(")");
    }
    
    token_atual = obter_proximo_token();
}
//Funcao de escrever
void funcao_write() {
    token_atual = obter_proximo_token();
    if (token_atual.tipo != TOKEN_ABRE_PARENTESES) {
        erro_sintatico("(");
    }
    
    token_atual = obter_proximo_token();
    if (token_atual.tipo == TOKEN_ID) {
        int endereco = busca_tabela_simbolos(token_atual.lexema);
        printf("CRVL %d\n", endereco);
    } else if (token_atual.tipo == TOKEN_NUMERO) {
        printf("CRCT %s\n", token_atual.lexema);
    } else {
        erro_sintatico("identificador ou número");
    }
    
    printf("IMPR\n");
    
    token_atual = obter_proximo_token();
    if (token_atual.tipo != TOKEN_FECHA_PARENTESES) {
        erro_sintatico(")");
    }
    
    token_atual = obter_proximo_token();
}
//SET
void funcao_set() {
    token_atual = obter_proximo_token();
    if (token_atual.tipo != TOKEN_ID) {
        erro_sintatico("identificador");
    }
    
    int endereco_destino = busca_tabela_simbolos(token_atual.lexema);
    
    token_atual = obter_proximo_token();
    if (token_atual.tipo != TOKEN_ATE) {
        erro_sintatico("to");
    }
    
    token_atual = obter_proximo_token();
    if (token_atual.tipo == TOKEN_NUMERO) {
        char* num_str = token_atual.lexema;
        if (strlen(num_str) >= 2 && num_str[0] == '0' && (num_str[1] == 'b' || num_str[1] == 'B')) {
            int num_val = converter_binario(num_str);
            printf("CRCT %d\n", num_val);
        } else {
            printf("CRCT %s\n", num_str);
        }
    } else if (token_atual.tipo == TOKEN_ID) {
        int endereco = busca_tabela_simbolos(token_atual.lexema);
        printf("CRVL %d\n", endereco);
        
        token_atual = obter_proximo_token();
        if (token_atual.tipo == TOKEN_MULTIPLICACAO) {
            token_atual = obter_proximo_token();
            if (token_atual.tipo == TOKEN_ID) {
                endereco = busca_tabela_simbolos(token_atual.lexema);
                printf("CRVL %d\n", endereco);
                printf("MULT\n");
            }
        }
    }
    
    printf("ARMZ %d\n", endereco_destino);
    token_atual = obter_proximo_token();
}
//FOR
void funcao_for() {
    int rotulo_inicio = proximo_rotulo();
    int rotulo_fim = proximo_rotulo();
    
    token_atual = obter_proximo_token();
    if (token_atual.tipo != TOKEN_ID) {
        erro_sintatico("identificador");
    }
    
    int endereco_contador = busca_tabela_simbolos(token_atual.lexema);
    
    token_atual = obter_proximo_token();
    if (token_atual.tipo != TOKEN_DE) {
        erro_sintatico("of");
    }
    
    token_atual = obter_proximo_token();
    if (token_atual.tipo != TOKEN_NUMERO) {
        erro_sintatico("número");
    }
    
    char* num_str = token_atual.lexema;
    if (strlen(num_str) >= 2 && num_str[0] == '0' && (num_str[1] == 'b' || num_str[1] == 'B')) {
        int num_val = converter_binario(num_str);
        printf("CRCT %d\n", num_val);
    } else {
        printf("CRCT %s\n", num_str);
    }
    printf("ARMZ %d\n", endereco_contador);
    
    token_atual = obter_proximo_token();
    if (token_atual.tipo != TOKEN_ATE) {
        erro_sintatico("to");
    }
    
    token_atual = obter_proximo_token();
    int endereco_limite;
    if (token_atual.tipo == TOKEN_ID) {
        endereco_limite = busca_tabela_simbolos(token_atual.lexema);
    }
    
    token_atual = obter_proximo_token();
    if (token_atual.tipo != TOKEN_DOIS_PONTOS) {
        erro_sintatico(":");
    }
    
    printf("NADA (L%d)\n", rotulo_inicio);
    printf("CRVL %d\n", endereco_contador);
    printf("CRVL %d\n", endereco_limite);
    printf("CMEG\n");
    printf("DSVF L%d\n", rotulo_fim);
    
    token_atual = obter_proximo_token();
    funcao_set();
    
    printf("CRVL %d\n", endereco_contador);
    printf("CRCT 1\n");
    printf("SOMA\n");
    printf("ARMZ %d\n", endereco_contador);
    printf("DSVS L%d\n", rotulo_inicio);
    printf("NADA (L%d)\n", rotulo_fim);
}
//Caso use mais de uma
void funcao_composto() {
    while (token_atual.tipo != TOKEN_FIM && token_atual.tipo != TOKEN_EOF) {
        switch (token_atual.tipo) {
            case TOKEN_LEIA:
                funcao_read();
                break;
            case TOKEN_ESCREVA:
                funcao_write();
                break;
            case TOKEN_FOR:
                funcao_for();
                break;
            case TOKEN_SET:
                funcao_set();
                break;
            default:
                return;
        }
        
        if (token_atual.tipo == TOKEN_PONTO_VIRGULA) {
            token_atual = obter_proximo_token();
        }
    }
}

void imprimir_tabela_simbolos() {
    printf("\nTABELA DE SIMBOLOS\n");
    for (int i = 0; i < num_simbolos; i++) {
        printf("%s | Endereco: %d\n", 
               tabela_simbolos[i].identificador, 
               tabela_simbolos[i].endereco);
    }
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
    token_atual = obter_proximo_token();

    if (token_atual.tipo != TOKEN_PROGRAM) {
        erro_sintatico("program");
    }
    
    token_atual = obter_proximo_token();
    if (token_atual.tipo != TOKEN_ID) {
        erro_sintatico("identificador");
    }
    
    token_atual = obter_proximo_token();
    if (token_atual.tipo != TOKEN_PONTO_VIRGULA) {
        erro_sintatico(";");
    }
    
    token_atual = obter_proximo_token();
    
    declaracao_variaveis();
    
    if (token_atual.tipo != TOKEN_INICIO) {
        erro_sintatico("begin");
    }
    
    token_atual = obter_proximo_token();
    funcao_composto();
    
    if (token_atual.tipo != TOKEN_FIM) {
        erro_sintatico("end");
    }
    
    printf("PARA\n");
    imprimir_tabela_simbolos();
    
    fclose(arquivo_fonte);
    return 0;
}