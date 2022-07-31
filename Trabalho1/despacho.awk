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
    # t = custo geração da termoelétrica
    # a = custo ambiental associado à hidrelétrica
    # var(i .. n) = variação do custo ambiental mensal da hidrelétrica
    # r(i .. n) = reservatório

    # função objetiva a ser minimizada
    printf "min: %g t + %g a;\n\n", CT, CA

    # variáveis utilizadas na função objetiva
    printf "t = "
    for (i = 1; i < n; ++i)
        printf "custoT%d + ", i
    printf "custoT%d;\n", n
    printf "a = "
    for (i = 1; i < n; ++i)
        printf "var%d + ", i
    printf "var%d;\n", n
    # volume do primeiro reservatório
    printf "r0 = %d;\n\n", V[0]

    # gera restrições
    for (i = 1; i <= n; ++i) {
        printf "custoT%d + %g total%d >= %d;\n", i, k, i, D[i - 1]
        printf "var%d >= %d - total%d;\n", i, Y[i - 1], i
        printf "var%d >= -%d + total%d;\n", i, Y[i - 1], i
        printf "r%d = r%d + %d - total%d;\n", i, i - 1, Y[i - 1], i
        printf "%d <= r%d <= %d;\n", V[1], i, V[2]
        printf "0 <= custoT%d <= %d;\n", i, tmax
        printf "var%d >= 0;\n\n", i
    }
}
