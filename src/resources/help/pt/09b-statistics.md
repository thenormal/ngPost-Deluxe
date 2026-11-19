# Estatísticas

As estatísticas transformam o seu [Histórico](09-nzb-files.md) em totais e gráficos: quanto publicou, a que velocidade, quando, onde e com que opções. Tudo é calculado no seu computador a partir do ficheiro do histórico; nada é enviado.

## Onde encontrar

Abra a página **Histórico** e prima **Estatísticas** (o seletor ao lado de **Lista**). Se o histórico estiver desativado ou vazio, a página indica-o: ative o histórico nas Definições e as estatísticas aparecem depois da sua primeira publicação.

## O período

A lista no topo escolhe o período: **Hoje**, **Últimos 7 dias**, **Últimos 30 dias**, **Últimos 12 meses** ou **Tudo**. A página inteira acompanha-o. Uma linha por baixo da lista indica desde quando existem os dados e quantas publicações abrangem.

## O que mostram

- **Totais**: oito quadros com o número de publicações, os dados enviados, o tempo gasto a publicar, a proporção de publicações sem erros, a velocidade média e a melhor, a maior publicação e a média por dia.
- **Dados enviados**: o tamanho publicado por hora (para **Hoje**), por dia (7 e 30 dias) ou por mês (12 meses e **Tudo**).
- **Velocidade**: velocidade média e melhor das publicações, na mesma escala.
- **Resultados**: publicações sem erros e com erros.
- **Quando publica (dia e hora)**: uma grelha da semana; quanto mais escuro o quadrado, mais publicações começaram a essa hora.
- **Grupos de notícias mais usados**, **Servidores**, **Como as publicações foram iniciadas** (Quick Post, Postagem Automática com itens escolhidos, análise de uma pasta ou pasta vigiada, linha de comandos) e **Opções** (compressão, ficheiros publicados tal como estão, par2, palavra-passe, arquivos mantidos).
- **Maiores publicações** e **Dias mais ativos**: as dez maiores publicações e os dias mais ativos, com o seu tamanho e velocidade.

Passe o rato sobre uma barra ou um quadrado para ler o valor exato.

## Exportar

O botão **Exportar** guarda o período mostrado:

- **Documento PDF**: um documento A4 para imprimir ou ler, com os totais, os gráficos e as classificações.
- **Página web (HTML)**: um único ficheiro com os gráficos lá dentro, que pode abrir em qualquer navegador ou enviar por email.
- **Folha de cálculo (CSV)**: os números exatos (bytes, bytes por segundo, número de publicações) para usar numa folha de cálculo; usa o mesmo separador do histórico.
- **Imagens dos gráficos (PNG)**: uma imagem por gráfico, na pasta que escolher.

As exportações usam sempre cores claras, por isso imprimem bem mesmo com o tema escuro. Contêm os nomes das suas maiores publicações mas nunca as palavras-passe.

## Bom saber

- Os servidores, as opções e a forma como uma publicação foi iniciada são registados desde a versão 1.4. As publicações mais antigas não são contadas nesses três gráficos, e uma linha por baixo de cada gráfico indica de quantas publicações a informação é conhecida.
- Os tamanhos das publicações feitas antes da versão 1.4 estão arredondados.
- Uma publicação feita com a [Postagem guiada](01b-guided-post.md) é contada como Quick Post ou Postagem Automática, porque são essas páginas que fazem o trabalho.
- Se apagar o ficheiro do histórico, as estatísticas recomeçam do zero.
