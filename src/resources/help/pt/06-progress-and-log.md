# Progresso e registo

## A barra de progresso

A barra na parte de baixo mostra a fase atual:

- **Compressão**: criação do arquivo.
- **par2**: criação dos ficheiros de recuperação.
- **Envio**: envio dos artigos. Aqui também vê a velocidade, o tamanho, o número de artigos e o **ETA** (tempo restante estimado).
- **Concluído**, **Em pausa** ou **Parado**.

A cor indica o estado: azul enquanto trabalha, cinzento em pausa, verde quando concluído, laranja se alguns artigos falharam, vermelho se a publicação parou com um erro. A percentagem também aparece no título da janela.

O ETA é uma estimativa baseada na velocidade recente: muda quando a velocidade muda.

## O registo

O registo colore as linhas: as fases, os sucessos, os avisos e os erros distinguem-se facilmente. No fim de uma publicação mostra um resumo com o tempo de cada fase (compressão, par2, envio) e o total.

## Latência

No fim de uma publicação o registo mostra também a **latência**: o tempo mediano de uma ida e volta com o servidor (o comando POST e a sua resposta, sem dados) e da ligação. É medida apenas a partir do que o ngPost já faz, não se envia nada para isso. Um valor alto (mais de meio segundo) significa que o atraso vem da rede, da distância ao servidor ou do próprio servidor, por exemplo uma VPN, e não do seu computador.

## Relatório de diagnóstico

Se algo não funcionar, o botão **Relatório de diagnóstico**, por baixo do registo na Postagem Rápida, guarda um relatório do que o programa fez, com uma secção **Latência** para cada servidor (ligação, SSL, início de sessão, ida e volta: mínimo, mediana, percentil 95 e máximo) e uma secção **Notificações** (o que é notificado, se a notificação do sistema está ativada e disponível, que canais estão definidos e a última falha; os endereços e os tokens nunca são escritos). É um ficheiro de texto que pode anexar quando pede ajuda. Verifique-o antes de o partilhar: não deve conter a sua palavra-passe.

## Notificações

O ngPost pode avisá-lo quando as suas publicações terminam, para não ter de vigiar a janela. Define-se em **Definições**, categoria **Notificações**, em duas partes: **o que notificar** (uma publicação terminada, uma publicação com erros ou falhada, uma mensagem no fim ou uma por publicação) e **para onde enviar as mensagens**. No programa com janela, a notificação do sistema está ativada desde o início; os outros canais precisam de um endereço.

- **Notificação do sistema**: uma mensagem a partir do ícone da área de notificação (só na versão de ambiente de trabalho; ativada por predefinição, pode desativá-la). No Windows aparece como qualquer outra notificação e fica no centro de notificações; se não a vir, verifique se as notificações e o assistente de concentração («Não incomodar») do Windows não a escondem.
- **Discord**: cole o endereço de um webhook do Discord.
- **Telegram**: o token do seu bot e o id da conversa que recebe as mensagens.
- **Outro webhook**: qualquer endereço que aceite um POST JSON com um `title` e um `text` (ferramentas tipo Slack, ntfy, Home Assistant).

Escolha o que é notificado (terminadas, erros ou falhas) e como: **uma mensagem quando todas as publicações terminam** (predefinido, o melhor para a Postagem Automática) ou **uma mensagem depois de cada publicação**. O botão **Enviar uma mensagem de teste** mostra logo se funciona.

A mensagem tem o nome da publicação, o resultado, o tamanho, o tempo e a velocidade. Nunca contém a palavra-passe nem os nomes dos ficheiros. Os endereços e o token são guardados em texto simples no ficheiro de configuração, tal como as outras palavras-passe: mantenha-o privado. Se uma mensagem não puder ser enviada, uma linha no registo di-lo e a publicação não é afetada.

Na linha de comandos, ou no Docker, use as opções `--notify_discord`, `--notify_telegram_token`, `--notify_telegram_chat`, `--notify_webhook` e `--notify_when each|all_done`, ou as chaves `notify_*` do ficheiro de configuração.
