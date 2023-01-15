#!/usr/bin/awk -f

BEGIN { 
    FS = "[ \t\n]+" # separador de campo (espaço, \t ou \n e consecutivos)
    RS = ""         # desabilita separador de linha padrão (\n)
}

# variáveis de extração de dados
BEGINFILE {
    # quantidade de fábricas da empresa
    m = 0
    # capacidade de produção em toneladas por fábrica da empresa
    C[m] = 0
    # quantidade de cidades da região
    n = 0
    # demanda de toneladas por cidade da região
    D[n] = 0
    # custos de transporte (reais/tonelada)
    T[m][n] = 0
}

# extrai dados do arquivo
FNR == 1 {
    idx = 1 # índice do campo a ser lido (incremental)
    m = $idx
    n = $++idx
    for (i = 0; i < m; ++i) C[i] = $++idx
    for (i = 0; i < n; ++i) D[i] = $++idx
    for (i = 0; i < m; ++i) for (j = 0; j < n; ++j) T[i][j] = $++idx
}

# converte saída para o formato lp_solve
ENDFILE {
    # função objetiva a ser minimizada
    printf "min: "
    for (i = 0; i < m; ++i) {
        for (j = 0; j < n - 1; ++j) printf "%d f%d%d + ", T[i][j], i, j

        if (i < m - 1) printf "%d f%d%d + ", T[i][n - 1], i, n - 1
        else printf "%d f%d%d;\n\n", T[m - 1][n - 1], m - 1, n - 1
    }
    # gera restrições
    for (i = 0; i < m; ++i) {
        for (j = 0; j < n - 1; ++j) printf "f%d%d + ", i, j
        printf "f%d%d <= %d;\n", i, n - 1, C[i]
    }
    for (i = 0; i < n; ++i) {
        for (j = 0; j < m - 1; ++j) printf "f%d%d + ", j, i
        printf "f%d%d >= %d;\n", m - 1, i, D[i]
    }
}
