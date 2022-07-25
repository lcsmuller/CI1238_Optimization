#!/usr/bin/awk -f

BEGIN { 
    FS = "[ \t\n]+" # separador de campo (espaço, \t ou \n e consecutivos)
    RS = ""         # desabilita separador de linha padrão (\n)
}

# variáveis de extração de dados
BEGINFILE {
    # quantidade de meses do planejamento
    n = 0
    # demandas mensais
    D[n] = 0
    # afluências mensais estimadas
    Y[n] = 0
    # capacidades da hidrelétrica
    #   (volume inicial, volume mínimo, volume máximo)
    V[2] = 0
    # coeficiente de geração
    k = 0.0
    # capacidade máxima de geração da termoelétrica
    tmax = 0
    # custo de geração da termoelétrica
    CT = 0.0
    # custo ambiental de variação do volume do reservatório da
    #    hidrelétrica em meses consecutivos
    CA = 0.0
}

# extrai dados do arquivo
FNR == 1 {
    idx = 1 # índice do campo a ser lido (incremental)
    n = $idx
    for (i = 0; i < n; ++i) D[i] = $++idx
    for (i = 0; i < n; ++i) Y[i] = $++idx
    for (i = 0; i < 3; ++i) V[i] = $++idx
    k = $++idx
    tmax = $++idx
    CT = $++idx
    CA = $++idx
}

# converte saída para o formato lp_solve
ENDFILE {
    # a = afluências mensais estimadas
    # d = demandas mensais
    # v = variação
    # r = reservatório

    # função objetiva a ser minimizada
    printf "min: %g a + %g d;\n\n", CT, CA

    # variáveis utilizadas na função objetiva
    printf "a = "
    for (i = 1; i < n; ++i)
        printf "a%d + ", i
    printf("a%d;\n", n)
    printf "d = "
    for (i = 1; i < n; ++i)
        printf "d%d + ", i
    printf("d%d;\n", n)
    # volume inicial do reservatório
    printf "r0 = %d;\n\n", V[0]

    # gera restrições
    for (i = 1; i <= n; ++i) {
        printf "a%d + %g total%d >= %d;\n", i, k, i, D[i - 1]
        printf "d%d = v%d;\n", i, i
        printf "v%d >= %d - total%d;\n", i, Y[i - 1], i
        printf "v%d >= -%d + total%d;\n", i, Y[i - 1], i
        printf "r%d = r%d + %d - total%d;\n", i, i - 1,
               Y[i - 1], i
        printf "%d <= r%d <= %d;\n", V[1], i, V[2]
        printf "0 <= a%d <= %d;\n", i, V[2], tmax
        printf "v%d >= 0;\n\n", i
    }
}
