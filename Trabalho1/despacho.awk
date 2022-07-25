#!/usr/bin/awk -f

BEGIN { 
    FS = "[ \t\n]+" # separador de campo (espaço, \t ou \n consecutivos)
    RS = ""         # desabilita separador de linha default (\n)
}

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

FNR == 1 {
    field = 0 # índice do campo a ser lido (incremental)

    # extrai dados do arquivo
    n = $++field
    for (i = 0; i < n; ++i) D[i] = $++field
    for (i = 0; i < n; ++i) Y[i] = $++field
    for (i = 0; i < 3; ++i) V[i] = $++field
    k = $++field
    tmax = $++field
    CT = $++field
    CA = $++field
}

ENDFILE {
    printf "n: %d\n", n

    printf "D:\n"
    for (x in D) print D[x]

    printf "Y:\n"
    for (x in Y) print Y[x]

    printf "V:\n"
    for (x in V) print V[x]
    printf "k: %f\n", k

    printf "tmax: %d | CT: %f\n", tmax, CT
    printf "CA: %f\n", CA
}
