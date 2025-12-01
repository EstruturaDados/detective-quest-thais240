
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NOME    64
#define MAX_PISTA  128
#define HASH_SIZE   31   // número primo pequeno para hash

typedef struct sala {
    char nome[MAX_NOME];
    char pista[MAX_PISTA];
    int visitada;            //evita recolher a mesma pista mais de uma vez, para não haver pistas duplicatas.
    struct sala *esq;
    struct sala *dir;
} Sala;

//Nó de pistas coletadas.
typedef struct pistaNode {
    char pista[MAX_PISTA];
    struct pistaNode *esq;
    struct pistaNode *dir;
} PistaNode;

/*
Entrada da do hash (lista encadeada)
com as chaves = pista, valor = suspeito.
*/
typedef struct hashEntry {
    char pista[MAX_PISTA];
    char suspeito[MAX_NOME];
    struct hashEntry *prox;
} HashEntry;

HashEntry *tabelaHash[HASH_SIZE];

//Cria dinamicamente um cômodo com o nome e sua pista.
Sala* criarSala(const char *nome, const char *pista);

/*
Navega pela árvore e ativa o sistema de pistas.
Recebe a sala inicial e ponteiro para a raiz de pistas coletadas.
Ao entrar em sala com pista (na primeira visita), faz a inserção.
*/
void explorarSalas(Sala *atual, PistaNode **raizPistas);

/*
Insere a pista coletada na árvore de pistas evitando duplicatas.
*/
PistaNode* inserirPista(PistaNode *raiz, const char *pista);

/*
 Insere a associação da pista com o suspeito no hash.
*/
void inserirNaHash(const char *pista, const char *suspeito);

/*
 Consulta qual suspeito correspondente a qual pista,
 retornando ponteiro para string do suspeito (ou NULL se não existir).
*/
const char* encontrarSuspeito(const char *pista);

/*
 mostra a árvore de pistas em ordem alfabética.
*/
void exibirPistas(PistaNode *raiz);

/*
 Percorre a as pistas coletadas e conta quantas
 pistas apontam para o suspeito acusado; também pode listar quais pistas apontaram.
*/
int verificarSuspeitoFinal(PistaNode *raiz, const char *acusado);

/*
 Limpa as pistas,
 o hash,
 e as salas liberando a memória.
*/
void liberarPistas(PistaNode *raiz);
void liberarHash();
void liberarSalas(Sala *raiz);

//pega um texto e transforma em um número que serve como endereço.
unsigned int hashString(const char *s) {
    unsigned long h = 5381;
    while (*s) {
        h = ((h << 5) + h) + (unsigned char)(*s);
        s++;
    }
    return (unsigned int)(h % HASH_SIZE);
}

//Cria um cômodo com o número de pistas.
Sala* criarSala(const char *nome, const char *pista) {
    Sala *n = (Sala*) malloc(sizeof(Sala));
    if (!n) {
        fprintf(stderr, "Erro de alocação de memória (sala)\n");
        exit(1);
    }
    strncpy(n->nome, nome, MAX_NOME-1);
    n->nome[MAX_NOME-1] = '\0';
    strncpy(n->pista, pista, MAX_PISTA-1);
    n->pista[MAX_PISTA-1] = '\0';
    n->visitada = 0;
    n->esq = n->dir = NULL;
    return n;
}

//Insere a pista coletada na árvore de pistas, evitando duplicatas.
PistaNode* inserirPista(PistaNode *raiz, const char *pista) {
    if (!pista || pista[0] == '\0') return raiz;

    if (raiz == NULL) {
        PistaNode *novo = (PistaNode*) malloc(sizeof(PistaNode));
        if (!novo) { fprintf(stderr, "Erro de alocação (pista)\n"); exit(1); }
        strncpy(novo->pista, pista, MAX_PISTA-1);
        novo->pista[MAX_PISTA-1] = '\0';
        novo->esq = novo->dir = NULL;
        return novo;
    }

    int cmp = strcmp(pista, raiz->pista);
    if (cmp == 0) {
        // já coletada; não insere duplicata
        return raiz;
    } else if (cmp < 0) {
        raiz->esq = inserirPista(raiz->esq, pista);
    } else {
        raiz->dir = inserirPista(raiz->dir, pista);
    }
    return raiz;
}

//Associa o suspeito à suas pistas.
void inserirNaHash(const char *pista, const char *suspeito) {
    if (!pista || pista[0] == '\0') return;
    unsigned int idx = hashString(pista);
    HashEntry *ent = (HashEntry*) malloc(sizeof(HashEntry));
    if (!ent) { fprintf(stderr, "Erro de alocação (hash)\n"); exit(1); }
    strncpy(ent->pista, pista, MAX_PISTA-1);
    ent->pista[MAX_PISTA-1] = '\0';
    strncpy(ent->suspeito, suspeito, MAX_NOME-1);
    ent->suspeito[MAX_NOME-1] = '\0';
    ent->prox = tabelaHash[idx];
    tabelaHash[idx] = ent;
}

//Consulta o suspeito que corresponde a cada pista.
const char* encontrarSuspeito(const char *pista) {
    if (!pista) return NULL;
    unsigned int idx = hashString(pista);
    HashEntry *cur = tabelaHash[idx];
    while (cur) {
        if (strcmp(cur->pista, pista) == 0) return cur->suspeito;
        cur = cur->prox;
    }
    return NULL;
}

//Encontra a pista atribuída a cada sala.
void explorarSalas(Sala *atual, PistaNode **raizPistas) {
    if (!atual) return;
    char opcao;
    while (1) {
        printf("\nVocê está na sala: %s\n", atual->nome);

        // coleta a pista apenas na primeira visita
        if (!atual->visitada) {
            if (strlen(atual->pista) > 0) {
                printf("Pista encontrada: \"%s\"\n", atual->pista);
                *raizPistas = inserirPista(*raizPistas, atual->pista);
            } else {
                printf("Nenhuma pista nesta sala.\n");
            }
            atual->visitada = 1;
        } else {
            printf("Sala já visitada (pista já coletada quando aplicável).\n");
        }

        //Movimentação de esquerda e direita. e sair caso não queira percorrer todo o mapa.
        printf("\nCaminhos disponíveis:\n");
        if (atual->esq) printf("  (e) Esquerda -> %s\n", atual->esq->nome);
        if (atual->dir) printf("  (d) Direita  -> %s\n", atual->dir->nome);
        printf("  (s) Sair da exploração\n");
        printf("Opção: ");
        if (scanf(" %c", &opcao) != 1) opcao = 's';
        int ch; while ((ch = getchar()) != '\n' && ch != EOF) ; //limpa o buffer Isso limpa o teclado, jogando fora tudo que sobrou na linha.

        if (opcao == 'e' || opcao == 'E') {
            if (atual->esq) atual = atual->esq;
            else printf("Não há sala à esquerda.\n");
        } else if (opcao == 'd' || opcao == 'D') {
            if (atual->dir) atual = atual->dir;
            else printf("Não há sala à direita.\n");
        } else if (opcao == 's' || opcao == 'S') {
            printf("Exploração finalizada pelo jogador.\n");
            return;
        } else {
            printf("Opção inválida. Use 'e', 'd' ou 's'.\n");
        }
    }
}

//Mostra a árvore de pistas organizada em ordem alfabética.
void exibirPistas(PistaNode *raiz) {
    if (!raiz) return;
    exibirPistas(raiz->esq);
    printf("- %s\n", raiz->pista);
    exibirPistas(raiz->dir);
}

//Conta quantas pistas encontradas apontam para o suspeito que o detetive acusou.
int verificarSuspeitoFinal(PistaNode *raiz, const char *acusado) {
    if (!raiz) return 0;
    int count = 0;
    count += verificarSuspeitoFinal(raiz->esq, acusado);
    const char *sus = encontrarSuspeito(raiz->pista);
    if (sus && strcmp(sus, acusado) == 0) {
        count++;
    }
    count += verificarSuspeitoFinal(raiz->dir, acusado);
    return count;
}

/* Vai até o último galho da esquerda e apaga;
depois vai para o último galho da direita e apaga;
depois apaga a raíz.
*/
void liberarPistas(PistaNode *raiz) {
    if (!raiz) return;
    liberarPistas(raiz->esq);
    liberarPistas(raiz->dir);
    free(raiz);
}

/*
Percorre a lista de pistas e as limpa de uma em uma.
*/
void liberarHash() {
    for (int i = 0; i < HASH_SIZE; ++i) {
        HashEntry *cur = tabelaHash[i];
        while (cur) {
            HashEntry *tmp = cur;
            cur = cur->prox;
            free(tmp);
        }
        tabelaHash[i] = NULL;
    }
}

// Conduz a ordem da salas na árvore
void liberarSalas(Sala *raiz) {
    if (!raiz) return;
    liberarSalas(raiz->esq);
    liberarSalas(raiz->dir);
    free(raiz);
}
// Retorna o motivo de cada suspeito ter cometido o crime.
const char* obterMotivo(const char *suspeito) {
    if (strcmp(suspeito, "Sr. Silva") == 0)
        return "O Sr. Silva, o filho adotado de Sr. Almeida largou a faculdade de medicina para seguir seu sonho de ser músico, seu pai descobriu e ameaçou tira-lo do testamento. Ele registrou o crime em seu diário";
    
    if (strcmp(suspeito, "Sra. Almeida") == 0)
        return "A Sra. Almeida tinha um caso secreto revelado pela carta aberta encontrada. Ela o envenenou com um chá de espirradeiras de sua estufa.";
    
    if (strcmp(suspeito, "Jardineiro") == 0)
        return "O Jardineiro planejava arrombar o cofre e fugir com o dinheiro, mas sr. Almeida ouviu o barulho. Ele não teve tempo de se livrar da arma do crime";

    return "Motivo desconhecido.";
}
// -----------------------------
/* 
O main() monta a mansão, um hash com 3 suspeitos (cada um com 3 pistas que podem incrimina-los),
 inicia exploração e conduz julgamento.
 */
int main() {
    //inicializa o hash
    for (int i = 0; i < HASH_SIZE; ++i) tabelaHash[i] = NULL;

    /*
    Montagem da mansão (árvore binária de salas)
    Todas as pistas foram definidas aqui; algumas salas não tem pistas e são representadas por ""
    ou "pistas inconclusivas" para confundir o detetive.
    */
    Sala *hall        = criarSala("Hall de Entrada",          "Pegadas de lama seca"); //Jardineiro
    Sala *cozinha     = criarSala("Cozinha",                  "Pétala rosa"); //Sra. Almeida
    Sala *biblioteca  = criarSala("Biblioteca",               "Diário com inicial 'S'"); //Sr. Silva
    Sala *jardim      = criarSala("Jardim",                   "Tesoura de jardinagem ensanguentada"); //Jardineiro
    Sala *quarto      = criarSala("Quarto Principal",         "Carta aberta"); //Sra. Almeida
    Sala *escritorio  = criarSala("Escritório",               "Testamento alterado");//Sr. Silva
    Sala *porao       = criarSala("Porão",                    "Cofre arrombado, tem as mesmas pegadas de lama");//Jardineiro
    Sala *salaMusica  = criarSala("Sala de Música",           "Violino quebrado");//Sr. Silva
    Sala *closet      = criarSala("Closet",                   ""); // sala sem pistas
    Sala *varanda     = criarSala("Varanda",                  "cigarro pisado");//pista inconclusiva
    Sala *estufa      = criarSala("Estufa",                   "Jardim de espirradeira rosa");//Sra. Almeida
    Sala *corredor    = criarSala("Corredor",                 "Fio de cabelo preto preso no corrimão");//prova incomclusiva 

    // hall para:
    hall->esq = cozinha;
    hall->dir = corredor;

    //cozinha para:
    cozinha->esq = jardim;
    cozinha->dir = varanda;
    
    //corredor para:
    corredor->esq = biblioteca;
    corredor->dir = quarto;
    
    //biblioteca para:
    biblioteca->esq = salaMusica;
    bibliography: ; // nota: comentário para organizar mentalmente (sem efeito)
    
    //Jardim para:
    jardim->esq = estufa;
    
    //Quarto para:
    quarto->dir = closet;
    quarto->esq = escritorio;
    //varanda para:
    varanda->dir = porao;

    //Pistas que incriminam cada suspeito (se o jogador encontrar >= 2 pistas o culpado é condenado.)
    //Sr. Silva
    inserirNaHash("Diário com inicial 'S'", "Sr. Silva");
    inserirNaHash("Testamento alterado", "Sr. Silva");
    inserirNaHash("Violino quebrado", "Sr. Silva");

    //Sra. Almeida
    inserirNaHash("Pétala rosa", "Sra. Almeida");
    inserirNaHash("Carta aberta", "Sra. Almeida");
    inserirNaHash("Jardim de espirradeira rosa", "Sra. Almeida");

    //Jardineiro
    inserirNaHash("Pegadas de lama seca", "Jardineiro");
    inserirNaHash("Tesoura de jardinagem ensanguentada", "Jardineiro");
    inserirNaHash("Cofre arrombado, tem as mesmas pegadas de lama", "Jardineiro");

    //Pistas coletadas, ela inicia o jogo vazia
    PistaNode *raizPistas = NULL;

    printf("=== DETECTIVE QUEST: Acusar o Culpado ===\n");
    printf("Sr. Almeida foi encontrado morto em sua residência, atualmente existem 3 suspeitos: Sr. Silva, Sra. Almeida e o Jardineiro.\n");
    printf("Explore a mansão a partir do Hall de Entrada e colete pistas suficientes para levar o assassino a justiça!\n");

    // exploração interativa
    explorarSalas(hall, &raizPistas);

    // exibir pistas coletadas
    printf("\n===== PISTAS COLETADAS =====\n");
    if (!raizPistas) {
        printf("Nenhuma pista foi coletada.\n");
    } else {
        exibirPistas(raizPistas);
    }

    if (!raizPistas) {
        printf("\nSem pistas, não é possível acusar. Fim do jogo.\n");
        liberarHash();
        liberarSalas(hall);
        return 0;
    }

    // pede para que o jogador acuse um dos suspeitos.
    char acusado[MAX_NOME];
    printf("\nQuem é o culpado? Sr. Silva, Sra. Almeida ou o Jardineiro?:\n> ");
    if (!fgets(acusado, sizeof(acusado), stdin)) {
        acusado[0] = '\0';
    } else {
        acusado[strcspn(acusado, "\n")] = '\0';
    }

    // verificar número de pistas que apontam para o acusado
    int cont = verificarSuspeitoFinal(raizPistas, acusado);

    printf("\nResultado da acusação contra '%s':\n", acusado);
    printf("Número de pistas coletadas que apontam para %s: %d\n", acusado, cont); 
    

if (cont >= 2) {
    printf("\nVeredito: EVIDÊNCIAS SUFICIENTES! %s foi levado a justiça. Bom trabalho!\n", acusado);

    //Mostra o motivo do crime.
    const char *motivo = obterMotivo(acusado);
    printf("\nMotivo do crime: %s\n", motivo);

} else {
    printf("\nVeredito: EVIDÊNCIAS INSUFICIENTES! %s NÃO PODE SER CONDENADO.\n", acusado);
}
}