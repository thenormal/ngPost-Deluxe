# Servidores e ligações

Um servidor é o seu fornecedor de Usenet. Pode introduzir vários: as publicações são enviadas para todos os servidores ativados.

## Campos

- **Host** e **Porta**: dados pelo seu fornecedor. As portas habituais são 563 (ou 443) com SSL e 119 sem.
- **SSL**: use-o sempre que o seu fornecedor o ofereça. Cifra a ligação.
- **Utilizador** e **Palavra-passe**: a sua conta.
- **Ligações**: quantas ligações são abertas em paralelo. Mais ligações podem ser mais rápidas, mas o seu fornecedor tem um limite (muitas vezes de 20 a 50). Passar o limite provoca erros.

## Testar ligação

Carregue em **Testar ligação** antes de publicar. O teste liga-se e inicia sessão, e diz-lhe se o host, a porta, a definição SSL ou as credenciais estão errados.

## Erros comuns

- *Não é possível ligar*: host ou porta errados, sem internet, ou uma firewall ou uma VPN bloqueia a ligação.
- *Falha na autenticação*: nome de utilizador ou palavra-passe errados. Tente apenas algumas vezes: os fornecedores podem bloquear temporariamente uma conta depois de muitos inícios de sessão falhados.
- *Demasiadas ligações*: diminua o número de ligações.

Veja também [Problemas comuns](12-problems.md).
