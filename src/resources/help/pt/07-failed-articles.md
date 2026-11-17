# Artigos falhados e repostagem

Durante um envio, alguns artigos podem falhar (uma ligação cai, o servidor recusa-os). O ngPost tenta cada artigo várias vezes. Se alguns continuarem a falhar, a publicação termina a laranja e o registo diz quantos artigos falharam.

## Repostar os artigos em falta

Quando alguns artigos falharam, o separador mostra um botão **Repostar os artigos em falta**. Publica de novo **apenas esses artigos** e corrige o ficheiro NZB. Não é preciso enviar outra vez toda a publicação.

Depois de uma repostagem bem-sucedida, a lista de ficheiros é atualizada e a publicação fica verde.

## Dicas

- Se falharem muitos artigos, a causa costuma ser a ligação. Diminua o número de ligações, ou verifique os limites do fornecedor.
- Se os mesmos artigos falharem vezes sem conta, verifique a sua conta e os newsgroups.
- O número de tentativas pode ser definido com a opção `--retry` (5 por predefinição) ou no ficheiro de configuração.

Veja [Servidores e ligações](02-servers.md).
