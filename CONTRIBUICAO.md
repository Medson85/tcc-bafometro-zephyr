# Regras de contribuição

## Princípios

- A linha principal deve permanecer íntegra.
- Cada alteração deve possuir um objetivo claro.
- Arquivos gerados não devem ser registrados.
- Mudanças funcionais devem incluir testes quando aplicável.
- Mudanças arquiteturais devem ser documentadas.
- Alterações no ambiente devem ocorrer separadamente das funcionalidades.
- A documentação deve acompanhar a evolução do projeto.

## Ramos de desenvolvimento

Os ramos deverão ser curtos e representar uma única finalidade.

- funcionalidade/<descricao>;
- correcao/<descricao>;
- teste/<descricao>;
- documentacao/<descricao>;
- refatoracao/<descricao>;
- manutencao/<descricao>.

## Mensagens dos registros

As mensagens deverão utilizar um identificador curto seguido de uma descrição em português.

Exemplos:

- funcionalidade: adiciona estado de pré-aquecimento
- correção: mantém o inibidor ativo em falha
- teste: valida a duração mínima do sopro
- documentação: registra o ambiente de desenvolvimento
- refatoração: separa a leitura do sensor
- manutenção: atualiza os arquivos de configuração

## Revisão antes da integração

- verificar todos os arquivos modificados;
- executar os testes aplicáveis;
- compilar para as duas plataformas;
- atualizar a documentação;
- verificar o comportamento em condições de falha;
- confirmar que não foram incluídos arquivos gerados.

## Linha principal

O ramo principal será denominado main.

Alterações de desenvolvimento deverão ser integradas por meio de uma solicitação de integração no GitHub.